#pragma once

#include "lib/vector.h"
#include "memory.h"

#define V_EXPR_ACCEPT_METHODS                                \
  virtual Value* accept(Visitor<Value*>* visitor) const = 0; \
  virtual void accept(Visitor<void>* visitor) const = 0;

#define EXPR_ACCEPT_METHODS                                \
  Value* accept(Visitor<Value*>* visitor) const override { \
    return visitor->visit(this);                           \
  }                                                        \
  void accept(Visitor<void>* visitor) const override {     \
    return visitor->visit(this);                           \
  }

#define V_STMT_ACCEPT_METHODS virtual void accept(Visitor<void>* visitor) const = 0;

#define STMT_ACCEPT_METHODS                            \
  void accept(Visitor<void>* visitor) const override { \
    return visitor->visit(this);                       \
  }

#define GET_TOKENS_METHODS(start, stop) \
  virtual Token* getStart() const {     \
    return start;                       \
  }                                     \
  virtual Token* getStop() const {      \
    return stop;                        \
  }

namespace lox {

  class Token;
  class Value;

  struct Ast {
    virtual ~Ast() {}

    virtual Token* getStart() const = 0;
    virtual Token* getStop() const = 0;
  };

  class Assign;
  class Binary;
  class Call;
  class Get;
  class Grouping;
  class Literal;
  class Logical;
  class Set;
  class Super;
  class This;
  class Unary;
  class Variable;

  struct Expr : public Ast {
    template <class R>
    class Visitor {
     public:
      virtual R visit(const Assign* expr) = 0;
      virtual R visit(const Binary* expr) = 0;
      virtual R visit(const Call* expr) = 0;
      virtual R visit(const Get* expr) = 0;
      virtual R visit(const Grouping* expr) = 0;
      virtual R visit(const Literal* expr) = 0;
      virtual R visit(const Logical* expr) = 0;
      virtual R visit(const Set* expr) = 0;
      virtual R visit(const Super* expr) = 0;
      virtual R visit(const This* expr) = 0;
      virtual R visit(const Unary* expr) = 0;
      virtual R visit(const Variable* expr) = 0;
    };

    virtual ~Expr() {}

    V_EXPR_ACCEPT_METHODS
  };

  struct Assign : public Expr {
    Assign(Token* name, Expr* value)
      : name(name)
      , value(value) {}

    Token* name;
    Expr* value;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(name, value->getStop())
  };

  struct Binary : public Expr {
    Binary(Expr* left, Token* op, Expr* right)
      : left(left)
      , op(op)
      , right(right) {}

    Expr* left;
    Token* op;
    Expr* right;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(left->getStart(), right->getStop())
  };

  struct Call : public Expr {
    Call(Expr* callee, Vector<Expr*> arguments, Token* stop)
      : callee(callee)
      , arguments(arguments)
      , stop(stop) {}

    Expr* callee;
    Vector<Expr*> arguments;
    Token* stop;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(callee->getStart(), stop)
  };

  struct Get : public Expr {
    Get(Expr* object, Token* name)
      : object(object)
      , name(name) {}

    Expr* object;
    Token* name;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(object->getStart(), name)
  };

  struct Grouping : public Expr {
    Grouping(Token* lParen, Expr* expression, Token* rParen)
      : start(lParen)
      , expression(expression)
      , stop(rParen) {}

    Token* start;
    Expr* expression;
    Token* stop;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(start, stop)
  };

  struct Literal : public Expr {
    Literal(Token* value)
      : value(value) {}

    Token* value;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(value, value)
  };

  struct Logical : public Expr {
    Logical(Expr* left, Token* op, Expr* right)
      : left(left)
      , op(op)
      , right(right) {}

    Expr* left;
    Token* op;
    Expr* right;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(left->getStart(), right->getStop())
  };

  struct Set : public Expr {
    Set(Expr* object, Token* name, Expr* value)
      : object(object)
      , name(name)
      , value(value) {}

    Expr* object;
    Token* name;
    Expr* value;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(object->getStart(), value->getStop())
  };

  struct Super : public Expr {
    Super(Token* keyword, Token* method)
      : start(keyword)
      , method(method) {}

    Token* start;
    Token* method;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(start, method)
  };

  struct This : public Expr {
    This(Token* keyword)
      : keyword(keyword) {}

    Token* keyword;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(keyword, keyword)
  };

  struct Unary : public Expr {
    Unary(Token* op, Expr* right)
      : op(op)
      , right(right) {}

    Token* op;
    Expr* right;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(op, right->getStop())
  };

  struct Variable : public Expr {
    Variable(Token* name)
      : name(name) {}

    Token* name;

    EXPR_ACCEPT_METHODS
    GET_TOKENS_METHODS(name, name)
  };

  /* Stmt */
  class Block;
  class Class;
  class Expression;
  class Function;
  class If;
  class Print;
  class Return;
  class Var;
  class While;

  struct Stmt : public Ast {
    template <class R>
    class Visitor {
     public:
      virtual R visit(const Block* stmt) = 0;
      virtual R visit(const Class* stmt) = 0;
      virtual R visit(const Expression* stmt) = 0;
      virtual R visit(const Function* stmt) = 0;
      virtual R visit(const If* stmt) = 0;
      virtual R visit(const Print* stmt) = 0;
      virtual R visit(const Return* stmt) = 0;
      virtual R visit(const Var* stmt) = 0;
      virtual R visit(const While* stmt) = 0;
    };

    Stmt(Token* start, Token* stop)
      : start(start)
      , stop(stop) {}

    virtual ~Stmt() {}

    V_STMT_ACCEPT_METHODS
    GET_TOKENS_METHODS(start, stop)

    Token* start;
    Token* stop;
  };

  struct Block : public Stmt {
    Block(Token* lBrace, Vector<Stmt*> statements, Token* rBrace)
      : Stmt(lBrace, rBrace)
      , statements(statements) {}

    Vector<Stmt*> statements;

    STMT_ACCEPT_METHODS
  };

  struct Class : public Stmt {
    Class(Token* keyword, Token* name, Variable* superclass, Vector<Function*> methods,
          Token* rBrace)
      : Stmt(keyword, rBrace)
      , name(name)
      , superclass(superclass)
      , methods(methods) {}

    Token* name;
    Variable* superclass;
    Vector<Function*> methods;

    STMT_ACCEPT_METHODS
  };

  struct Expression : public Stmt {
    Expression(Expr* expression, Token* semicolon)
      : Stmt(expression->getStart(), semicolon)
      , expression(expression) {}

    Expr* expression;

    STMT_ACCEPT_METHODS
  };

  struct Function : public Stmt {
    Function(Token* start, Token* name, Vector<Token*> params, Vector<Stmt*> body, Token* stop)
      : Stmt(start, stop)
      , name(name)
      , params(params)
      , body(body) {}

    Token* name;
    Vector<Token*> params;
    Vector<Stmt*> body;

    STMT_ACCEPT_METHODS
  };

  struct If : public Stmt {
    If(Token* start, Expr* condition, Stmt* thenBranch, Stmt* elseBranch)
      : Stmt(start, (elseBranch ? elseBranch : thenBranch)->getStop())
      , condition(condition)
      , thenBranch(thenBranch)
      , elseBranch(elseBranch) {}

    Expr* condition;
    Stmt* thenBranch;
    Stmt* elseBranch;

    STMT_ACCEPT_METHODS
  };

  struct Print : public Stmt {
    Print(Token* keyword, Expr* expression, Token* stop)
      : Stmt(keyword, stop)
      , expression(expression) {}

    Expr* expression;

    STMT_ACCEPT_METHODS
  };

  struct Return : public Stmt {
    Return(Token* keyword, Expr* value, Token* stop)
      : Stmt(keyword, stop)
      , value(value) {}

    Expr* value;

    STMT_ACCEPT_METHODS
  };

  struct Var : public Stmt {
    Var(Token* keyword, Token* name, Expr* initializer, Token* stop)
      : Stmt(keyword, stop)
      , name(name)
      , initializer(initializer) {}

    Token* name;
    Expr* initializer;

    STMT_ACCEPT_METHODS
  };

  struct While : public Stmt {
    While(Token* keyword, Expr* condition, Stmt* body, Token* stop)
      : Stmt(keyword, stop)
      , condition(condition)
      , body(body) {}

    Expr* condition;
    Stmt* body;

    STMT_ACCEPT_METHODS
  };

}; // namespace lox
