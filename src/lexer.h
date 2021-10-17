#pragma once

#include <functional>

#include "./lib/vector.h"

namespace lox {

  enum TokenType {
    // Single-character tokens.
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,
    // One or two character tokens.
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    // Literals.
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    // Keywords.
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUN,
    TOKEN_IF,
    TOKEN_NIL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,

    TOKEN_ERROR,
    TOKEN_EOF
  };

  struct Token {
    Token(TokenType type, const char* start, int length, int line)
      : type(type)
      , start(start)
      , length(length)
      , line(line) {}

    TokenType type;
    const char* start;
    int length;
    int line;
  };

  class Lexer {
  public:
    Lexer(const char* source);
    ~Lexer();

    Token* readToken();
    Token* syntheticToken(TokenType type, const char* text);

  private:
    void skipWhitespace();
    char peek(int ahead = 0) const;
    bool isDone() const;
    char advance();
    void advanceWhile(std::function<bool()> cond);
    bool match(char c);
    int currentLength() const;

    Token* number();
    Token* identifier();
    TokenType identifierType();
    TokenType checkKeyword(int start, int length, const char* rest, TokenType type) const;
    Token* string();

    Token* makeToken(TokenType type);
    Token* errorToken(const char* message);
    Token* newToken();
    void freeToken(Token* token);

  private:
    const char* source_;
    Vector<Token*> tokens_;

    const char* start_;
    const char* current_;
    int line_;
  };

}; // namespace lox
