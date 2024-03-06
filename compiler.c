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

Parser parser;

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
 * Handling syntax errors from the parser
 */
static void errorAtCurrent(const char *message) {
  errorAt(&parser.current, message);
}

/*
 * We save the current consumed token as previous then get the next token
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

static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

static void emitReturn() { emitByte(OP_RETURN); }

static void endCompiler() { emitReturn(); }

static void parsePrecedence(Precedence Precedence) {}

void expression() { parsePrecedence(PREC_ASSIGNMENT); }
/****************** The heart of compilation ******************* */

/**
 * We assume the beginning `(` has been comsumed already
 */
static void grouping() {
  /*Compute the content of the bracket */
  expression();
  consume(TOKEN_RIGHT_PAREN, "expected ')' after expression.");
}

/**
 * Assumes the the begining unary operator has already been consumed
 * in this case `!` or `-`
 * The negate is emitted last after the expression because it lives on top of
 * the stack
 */
static void unary() {
  TokenType operatorType = parser.previous.type;

  // We compile the second operand
  parsePrecedence(PREC_UNARY);
  expression();

  switch (operatorType) {
  case TOKEN_MINUS:
    emitByte(OP_NEGATE);
  default:
    return;
  }
}
static void binary() {
  TokenType operatorType = parser.previous.type;

  ParseRule *rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

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
  emitByte(OP_CONSTANT, makeConstant(value));
}

/*
 * Takes a consumed number token  and convert it into a double, then emits it
 */
static void number() {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(value);
}

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
