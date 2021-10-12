#include "lexer.h"

#include <cstring>
#include <new>

#include "memory.h"

namespace lox {

  static inline bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }

  static inline bool isDigit(char c) {
    return c >= '0' && c <= '9';
  }

  static inline bool strEq(const char* a, const char* b, int len) {
    return std::memcmp(a, b, len) == 0;
  }

  Lexer::Lexer(const char* source)
    : source_(source)
    , start_(source)
    , current_(source)
    , line_(1) {}

  Lexer::~Lexer() {
    for (int i = 0; i < tokens_.size(); i++)
      freeToken(tokens_[i]);
  }

  Token* Lexer::readToken() {
    skipWhitespace();

    start_ = current_;

    if (isDone()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
      case '(': return makeToken(TOKEN_LEFT_PAREN);
      case ')': return makeToken(TOKEN_RIGHT_PAREN);
      case '{': return makeToken(TOKEN_LEFT_BRACE);
      case '}': return makeToken(TOKEN_RIGHT_BRACE);
      case ';': return makeToken(TOKEN_SEMICOLON);
      case ',': return makeToken(TOKEN_COMMA);
      case '.': return makeToken(TOKEN_DOT);
      case '-': return makeToken(TOKEN_MINUS);
      case '+': return makeToken(TOKEN_PLUS);
      case '/': return makeToken(TOKEN_SLASH);
      case '*': return makeToken(TOKEN_STAR);
      case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
      case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
      case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
      case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
      case '"': return string();
    }

    return errorToken("Unexpected character.");
  }

  Token* Lexer::number() {
    advanceWhile([this]() { return isDigit(peek()); });
    if (peek() == '.' && isDigit(peek(1))) {
      advance(); // Consume the ".".
      advanceWhile([this]() { return isDigit(peek()); });
    }
    return makeToken(TOKEN_NUMBER);
  }

  Token* Lexer::identifier() {
    advanceWhile([this]() { return isAlpha(peek()) || isDigit(peek()); });
    return makeToken(identifierType());
  }

  TokenType Lexer::identifierType() {
    switch (start_[0]) {
      case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
      case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
      case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
      case 'f':
        if (currentLength() > 1) {
          switch (start_[1]) {
            case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
            case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
            case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
          }
        }
        break;
      case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
      case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
      case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
      case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
      case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
      case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
      case 't':
        if (currentLength() > 1) {
          switch (start_[1]) {
            case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
            case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
          }
        }
        break;
      case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
      case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
  }

  TokenType Lexer::checkKeyword(int start, int length, const char* rest, TokenType type) const {
    if (currentLength() == start + length && strEq(start_ + start, rest, length)) {
      return type;
    }
    return TOKEN_IDENTIFIER;
  }

  Token* Lexer::string() {
    while (peek() != '"' && !isDone()) {
      if (peek() == '\n') line_++;
      advance();
    }

    if (isDone()) return errorToken("Unterminated string.");

    advance(); // Closing quote.
    return makeToken(TOKEN_STRING);
  }

  void Lexer::skipWhitespace() {
    while (true) {
      char c = peek();
      switch (c) {
        case ' ':
        case '\r':
        case '\t': {
          advance();
          break;
        }
        case '\n': {
          line_++;
          advance();
          break;
        }
        case '/':
          if (peek(1) == '/') {
            advanceWhile([this]() { return peek() != '\n' && !isDone(); });
            break;
          } else {
            return;
          }
        default: return;
      }
    }
  }

  char Lexer::peek(int ahead) const {
    if (ahead < 1 || isDone()) {
      return *current_;
    }
    return current_[ahead];
  }

  bool Lexer::isDone() const {
    return *current_ == '\0';
  }

  char Lexer::advance() {
    current_++;
    return current_[-1];
  }

  void Lexer::advanceWhile(std::function<bool()> cond) {
    while (cond())
      advance();
  }

  bool Lexer::match(char c) {
    if (isDone() || *current_ != c) return false;

    current_++;
    return true;
  }

  int Lexer::currentLength() const {
    return (int)(current_ - start_);
  }

  Token* Lexer::makeToken(TokenType type) {
    Token* token = newToken();
    return new (token) Token(type, start_, currentLength(), line_);
  }

  Token* Lexer::errorToken(const char* message) {
    Token* token = newToken();
    return new (token) Token(TOKEN_ERROR, message, std::strlen(message), line_);
  }

  Token* Lexer::newToken() {
    Token* token = Memory::allocate<Token>();
    tokens_.push(token); // Save token to keep ownership
    return token;
  }

  void Lexer::freeToken(Token* token) {
    Memory::reallocate(token, sizeof(Token), 0);
  }

}; // namespace lox
