#include "parser.h"

namespace lox {

  Parser::Parser(Lexer& lexer)
    : lexer_(lexer) {}

  Parser::~Parser() {}

  Expr* Parser::primary() {
    if (match(TOKEN_FALSE, TOKEN_TRUE, TOKEN_NIL, TOKEN_NUMBER, TOKEN_STRING, TOKEN_THIS,
              TOKEN_IDENTIFIER)) {
      return NULL;
    }
    return NULL;
  }

  bool Parser::match(TokenType type) {
    if (lookAhead(type)) {
      consume();
      return true;
    }
    return false;
  }

  template <typename... Args>
  bool Parser::match(Args&&... types) {
    for (auto type : std::initializer_list<TokenType>{types...}) {
      if (match(type)) return true;
    }
    return false;
  }

  bool Parser::lookAhead(TokenType type) {
    fillLookAhead(1);
    return read_[0]->type == type;
  }

  void Parser::fillLookAhead(int count) {
    while (read_.count() < count) {
      // TODO: Handle TOKEN_ERROR case
      Token* token = lexer_.readToken();
      read_.enqueue(token);
    }
  }

  Token* Parser::consume() {
    fillLookAhead(1);
    return read_.dequeue();
  }

}; // namespace lox
