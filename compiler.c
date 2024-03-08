#include "compiler.h"
#include "chunk.h"
#include "scanner.h"
#include "value.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Used to hold the current and previous token in memory for compilation
 * @hadError This is returned by the compiler for the VM to know if any error
 * occured.
 * @panicMode We get into panic mode when the user has made an error up and in
 * this mode for convinience sake, we continue compiling but we know we will not
 * execute the faulty code
 */
typedef struct {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
} Parser;

/*
 * Helps us to know and to manage the precedence of operations
 * The order counts and relies on the fact that C gives higher numbers to lower
 * members in an enum
 * PREC_NONE < PREC_ASSIGNMENT is true*/

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
} Precedence;

/*
 * This is function type. We just named it ParseFn and parse function
 */
typedef void (*ParseFn)();

/*
 * A general of a parser rule. This rule is describe precedence of operators,
 * function to call to parse them, etc
 * @prefix and @infix are the function to call when the operator appears as a
 * prefix or infix respectively e.g minus can be prefix (-3) or infix (2 - 3)
 * @precendence is the precendence of the infix expression which uses this
 * operator.
 */
typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

// static ParseRule *getRule(TokenType type);

Parser parser;

/*Brief declaration of functions
 */
static void expression();
static ParseRule *getRule(TokenType type);
/**
 * This is nice way to only parse expressions whose precdence is higher or
 * equals to the provided @minimum precedence
 */
static void parsePrecedence(Precedence minimum);
/*End of declaration*/

// A chunk can represent a scope or context of executation
Chunk *compilingChunk;

static Chunk *currentChunck() { return compilingChunk; }

static void errorAt(Token *token, const char *message) {
  parser.panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

/*
 * Reports an error that has happened on the current token
 */
static void errorAtCurrent(const char *message) {
  errorAt(&parser.current, message);
}

/*
 * Report an error that happened on the previous token
 */
static void error(const char *message) { errorAt(&parser.previous, message); }
/* We save the current consumed token as previous then get the next token
 * Report any error emitted by the scanner
 */
static void advance() {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR)
      break;

    errorAtCurrent(parser.current.start);
  }
}

/* Checks whether the current token is aof expected @type in which case we
 * advance, else we throw the @message as error */
static void consume(TokenType type, const char *message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

/*
 * Emitting bytecode into the curent chunk.
 * Emit a single @byte (OP_CODE or OPERAND)
 *
 */
static void emitByte(uint8_t byte) {
  writeChunk(currentChunck(), byte, parser.previous.line);
}

/*
 * Emit 2 bytes
 */
static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

static void emitReturn() { emitByte(OP_RETURN); }
/*
 * Add the value as a constant to the compilingChunk and verifies if the limit
 * of 256 constants per chunk has not been exceeded
 * returs: The constant's index in the chunk's table
 */
static uint8_t makeConstant(Value value) {
  int constIndex = addConstant(currentChunck(), value);
  /*Maximum of 256 constants per chunk */
  if (constIndex > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return (uint8_t)constIndex;
}

static void emitConstant(Value value) {
  emitBytes(OP_CONSTANT, makeConstant(value));
}

static void parsePrecedence(Precedence precedence) {
  advance();
  // We get the prefix rule which is a function pointer or NULL
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expect expression.");
    return;
  }
  prefixRule();

  /*
   * We continuously parse till we reach a lower precendence
   */
  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    infixRule();
  }
}

static void endCompiler() { emitReturn(); }

/*
 * We want to parse all things that have greater precedence than assignments
 */
void expression() { parsePrecedence(PREC_ASSIGNMENT); }

/**
 * We assume the left operand has already been consumed already and we have now
 * reached the operator.
 * We push the left operand, the right then the operator
 * a + b => |+|b|a| in the stack
 */
static void binary() {
  // Remember the operator
  TokenType operatorType = parser.previous.type;

  // Compile the right operand
  ParseRule *rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  // Emit the operator instruction.
  switch (operatorType) {
  case TOKEN_PLUS:
    emitByte(OP_ADD);
    break;
  case TOKEN_MINUS:
    emitByte(OP_SUBTRACT);
    break;
  case TOKEN_STAR:
    emitByte(OP_MULTIPLY);
    break;
  case TOKEN_SLASH:
    emitByte(OP_DIVIDE);
    break;
  default:
    return;
  }
}

/**
 * We assume the beginning `(` has been comsumed already
 */
static void grouping() {
  /*Compute the content of the bracket */
  expression();
  consume(TOKEN_RIGHT_PAREN, "expected ')' after expression.");
}

/*
 * Takes a consumed number token  and convert it into a double, then emits it
 */
static void number() {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(value);
}

/**
 * Assumes the the begining unary operator has already been consumed
 * in this case `!` or `-`
 * The negate is emitted last after the expression because it lives on top of
 * the stack
 */
static void unary() {
  TokenType operatorType = parser.previous.type;

  /*Note that we only want to parse expressions whose precedence is greater or
   * same as the unary expressions i.e. calls,
   * This is to avoid cases like -a.b + x where the unary should only apply to
   * a.b and not to the rest of the operation as binary + has lesser precendence
   * than unary*/
  parsePrecedence(PREC_UNARY);
  expression();

  switch (operatorType) {
  case TOKEN_MINUS:
    emitByte(OP_NEGATE);
  default:
    return;
  }
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG] = {NULL, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_GREATER] = {NULL, NULL, PREC_NONE},
    [TOKEN_GREATER_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_LESS] = {NULL, NULL, PREC_NONE},
    [TOKEN_LESS_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_IDENTIFIER] = {NULL, NULL, PREC_NONE},
    [TOKEN_STRING] = {NULL, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {NULL, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {NULL, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static ParseRule *getRule(TokenType type) { return &rules[type]; }

/****************** The heart of compilation ******************* */

bool compile(const char *source, Chunk *chunk) {
  initScanner(source);
  compilingChunk = chunk;

  parser.panicMode = false;
  parser.hadError = false;

  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");
  endCompiler();
  return !parser.hadError;
}
