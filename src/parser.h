#pragma once

#include "ast/expr.h"
#include "lexer.h"
#include "lib/queue.h"

namespace lox {

  class Parser {
  public:
    Parser(Lexer& lexer);
    ~Parser();

  private:
    Expr* primary();

    bool match(TokenType type);
    template <typename... Args>
    bool match(Args&&... types);
    bool lookAhead(TokenType type);
    void fillLookAhead(int count);
    Token* consume();

  private:
    Lexer& lexer_;

    Queue<Token*, 2> read_;
  };

}; // namespace lox
