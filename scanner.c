#include "scanner.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

/*
 * @start Points to the first character of a word in the source code
 * @current Points to the current character to be consumed
 * Notice that all arrays of characters or strings end with the special end of
 * line character, or `\0`
 */
typedef struct {
  const char *start;
  const char *current;
  int line;
} Scanner;

/* We need only one "instance" of the scanner, thus global varaible */
Scanner scanner;

void initScanner(const char *source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

/**
 * A simple constructor for tokens of a particular type
 */
static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);

  token.line = scanner.line;
  return token;
}

/*
 * Constructs an error token with its length being that of the message
 * Can be used to construct any type or kind of error we wish
 */
static Token errorToken(const char *message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;

  return token;
}

/*
 * All strings in C end with the terminator `\0` and so if we currently point to
 * it, then we have reached the end of the array of characters.
 */
static bool isAtEnd() { return *scanner.current == '\0'; }

/*
 * We point to next character in the array of characters constituting the source
 * code, then we return the value;
 */
char advance() {
  scanner.current++;
  return scanner.current[-1];
}

/*
 * Is the current character same as the expected character?
 */
static bool match(char expected) {
  if (isAtEnd())
    return false;
  if (*scanner.current != expected)
    return false;

  // In that case, we advance
  scanner.current++;
  return true;
}

/* I just want to see what the nex unconsumed character is.
 */
char peek() { return *scanner.current; }

/*I just want to see at the next next unconsumed character is.
 */
char peekNext() {
  if (isAtEnd())
    return '\0';
  return *(scanner.current + 1);
}

/*Remove trailing whitespaces at the beginning of the source code
 * Or, increement line number in case of a newline `\n`
 */
void skipWhitespace() {
  for (;;) {
    char c = peek();

    switch (c) {
    case ' ':
    case '\t':
    case '\r':
      advance();
      break;

    case '\n':
      scanner.line++;
      advance();
      break;

    default:
      return;
    }
  }
}

/*
 * I want consider all things that are between " ... " as part of a string
 * token. My strings can be multilined as well and we will consider that the
 * token line is the last line
 */
static Token string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      scanner.line++;
    advance();
  }
  if (isAtEnd())
    errorToken("Unterminated string.");

  advance(); // Consume the closing quotes
  return makeToken(TOKEN_STRING);
}

/* Want to know the character `c` is a digit 0 .. 9 */
static bool isDigit(char c) { return c >= '0' && c <= '9'; }

/* Want to know if character `c` is a letter a..z A..Z or _
 */
static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/* Scans for a number and returns the number. fractional number are also
 * possible. Notice that we donot convert the lexeme into double yet. This will
 * be done at compile time (Memory needs not to be occupied for nothing).
 */
Token number() {
  while (isDigit(peek()))
    advance();

  // The fractional part
  if (peek() == '.' && isDigit(peekNext())) {
    advance(); // We consume the '.' in the fraction
  }

  while (isDigit(peek()))
    advance();

  return makeToken(TOKEN_NUMBER);
}

/*
 * Checks to see if the substring from start and of given length of the lexeme
 * currently being scanned is of same length and is equals to the `rest` string.
 */
static TokenType checkKeyword(int start, int length, const char *rest,
                              TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length)) {
    return type;
  }
  return TOKEN_IDENTIFIER;
}

/*
 * To manage the case where the identifier we just scanned is a keyword or not
 * We implement the DFA to know if we are in presence of a keyword or user
 * define identifier
 */
static TokenType identifierType() {
  switch (scanner.start[0]) {
  case 'a':
    return checkKeyword(1, 2, "nd", TOKEN_AND);
  case 'c':
    return checkKeyword(1, 4, "lass", TOKEN_CLASS);
  case 'e':
    return checkKeyword(1, 3, "lse", TOKEN_ELSE);
  case 'f':
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'a':
        return checkKeyword(2, 3, "lse", TOKEN_FALSE);
      case 'o':
        return checkKeyword(2, 1, "r", TOKEN_FOR);
      case 'u':
        return checkKeyword(2, 1, "n", TOKEN_FUN);
      }
    }
    break;

  case 'i':
    return checkKeyword(1, 1, "f", TOKEN_IF);
  case 'n':
    return checkKeyword(1, 2, "il", TOKEN_NIL);
  case 'o':
    return checkKeyword(1, 1, "r", TOKEN_OR);
  case 'p':
    return checkKeyword(1, 4, "rint", TOKEN_PRINT);
  case 'r':
    return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
  case 's':
    return checkKeyword(1, 4, "uper", TOKEN_SUPER);
  case 't':
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'h':
        return checkKeyword(2, 2, "is", TOKEN_THIS);
      case 'r':
        return checkKeyword(2, 2, "ue", TOKEN_TRUE);
      }
    }
    break;
  case 'v':
    return checkKeyword(1, 2, "ar", TOKEN_VAR);
  case 'w':
    return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }
  return TOKEN_IDENTIFIER;
}

/*
 * Need identifiers of the form (alpha)?(alpha)*(number)* e.g _joe213, sum.
 * Etc
 */
Token identifier() {
  /* Notice that when we enter this function, isAlpha must be true*/
  while (isAlpha(peek()) || isDigit(peek())) {
    advance();
  }

  return makeToken(identifierType());
}

/* Heart of the scanner */
Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken(TOKEN_EOF);

  char c = advance();

  if (isAlpha(c))
    return identifier();
  if (isDigit(c))
    return number();

  /* Lexical grammar for Lox  */
  switch (c) {
  case '(':
    return makeToken(TOKEN_LEFT_PAREN);
  case ')':
    return makeToken(TOKEN_RIGHT_PAREN);
  case '{':
    return makeToken(TOKEN_LEFT_BRACE);
  case '}':
    return makeToken(TOKEN_RIGHT_BRACE);
  case ';':
    return makeToken(TOKEN_SEMICOLON);
  case ',':
    return makeToken(TOKEN_COMMA);
  case '.':
    return makeToken(TOKEN_DOT);
  case '-':
    return makeToken(TOKEN_MINUS);
  case '+':
    return makeToken(TOKEN_PLUS);
  case '/':
    // We support single line comments
    if (peekNext() == '/') {
      while (peek() != '\n' && !isAtEnd())
        advance();
    } else if (peekNext() == '*') {
      // We support multi line comments
      while (peek() != '*' && peekNext() != '/' && !isAtEnd()) {
        if (peek() == '\n')
          scanner.line++;
        advance();
      }
      if (isAtEnd())
        return errorToken("Unterminated block comment.");
    } else
      return makeToken(TOKEN_SLASH);

    break;

  case '*':
    return makeToken(TOKEN_STAR);
  case '!':
    return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
  case '=':
    return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
  case '<':
    return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
  case '>':
    return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
  case '"':
    return string();
  }
  return errorToken("Unexpected character.");
}
