#pragma once

#include "ast.h"
#include "lexer.h"
#include "lib/queue.h"
#include "lib/vector.h"

namespace lox {

  struct ParseResult {
    Vector<Stmt*> stmts;
    Token* eof;
  };

  class Parser {
   public:
    Parser(Lexer& lexer);
    ~Parser();

    bool parse();

    Expr* expression(); // TODO: Exposed for tests only

    const ParseResult& result() const {
      return result_;
    }

   private:
    Stmt* declaration();
    Stmt* classDeclaration();
    Stmt* funcDeclaration();
    Stmt* varDeclaration();
    Stmt* statement();
    Stmt* forStatement();
    Stmt* ifStatement();
    Stmt* printStatement();
    Stmt* returnStatement();
    Stmt* expressionStatement();
    Stmt* whileStatement();
    Stmt* block();

    Expr* assignment();
    Expr* or_();
    Expr* and_();
    Expr* equality();
    Expr* comparison();
    Expr* term();
    Expr* factor();
    Expr* unary();
    Expr* call();
    Expr* primary();

    Expr* finishCall(Expr* expr);
    Vector<Stmt*> blockBody();
    Function* function(const char* kind);

    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    bool lookAhead(TokenType type);
    void fillLookAhead(int count);
    Token* consume();
    Token* consume(TokenType type, const char* format, ...);
    const Token& current();
    bool isDone();

    void error(const char* format, ...);
    void errorAt(const Token& token, const char* format, ...);
    void synchronize();

    // https://stackoverflow.com/a/1111470
    // Templated code implementation should never be in a .cpp file
    template <typename T, typename... Args>
    T* newAstNode(Args&&... args) {
      T* node = Memory::allocate<T>();
      new (node) T(std::forward<Args>(args)...);

      astNodes_.push(node); // Save ast node to keep ownership
      astBytesAllocated_ += sizeof(T);

      return node;
    }

    void freeAstNode(Ast* node);

   private:
    ParseResult result_;

    Lexer& lexer_;
    Vector<Ast*> astNodes_;
    size_t astBytesAllocated_;

    Queue<Token*, 2> read_;
    // The most recently consumed token.
    Token* last_;

    bool hadError_;
    bool panicMode_;
  };

}; // namespace lox
