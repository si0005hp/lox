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

namespace lox {

  struct Ast {
    virtual ~Ast() {}
  };

  /* Expr */
  class Token;
  class Value;

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
  };

  struct Call : public Expr {
    Call(Expr* callee, Vector<Expr*> arguments)
      : callee(callee)
      , arguments(arguments) {}

    Expr* callee;
    Vector<Expr*> arguments;

    EXPR_ACCEPT_METHODS
  };

  struct Get : public Expr {
    Get(Expr* object, Token* name)
      : object(object)
      , name(name) {}

    Expr* object;
    Token* name;

    EXPR_ACCEPT_METHODS
  };

  struct Grouping : public Expr {
    Grouping(Expr* expression)
      : expression(expression) {}

    Expr* expression;

    EXPR_ACCEPT_METHODS
  };

  struct Literal : public Expr {
    Literal(Token* value)
      : value(value) {}

    Token* value;

    EXPR_ACCEPT_METHODS
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
  };

  struct Super : public Expr {
    Super(Token* keyword, Token* method)
      : keyword(keyword)
      , method(method) {}

    Token* keyword;
    Token* method;

    EXPR_ACCEPT_METHODS
  };

  struct This : public Expr {
    This(Token* keyword)
      : keyword(keyword) {}

    Token* keyword;

    EXPR_ACCEPT_METHODS
  };

  struct Unary : public Expr {
    Unary(Token* op, Expr* right)
      : op(op)
      , right(right) {}

    Token* op;
    Expr* right;

    EXPR_ACCEPT_METHODS
  };

  struct Variable : public Expr {
    Variable(Token* name)
      : name(name) {}

    Token* name;

    EXPR_ACCEPT_METHODS
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

    virtual ~Stmt() {}

    V_STMT_ACCEPT_METHODS
  };

  struct Block : public Stmt {
    Block(Vector<Stmt*> statements)
      : statements(statements) {}

    Vector<Stmt*> statements;

    STMT_ACCEPT_METHODS
  };

  struct Class : public Stmt {
    Class(Token* name, Variable* superclass, Vector<Function*> methods)
      : name(name)
      , superclass(superclass)
      , methods(methods) {}

    Token* name;
    Variable* superclass;
    Vector<Function*> methods;

    STMT_ACCEPT_METHODS
  };

  struct Expression : public Stmt {
    Expression(Expr* expression, Token* stop)
      : expression(expression)
      , stop(stop) {}

    Expr* expression;
    Token* stop; // TODO

    STMT_ACCEPT_METHODS
  };

  struct Function : public Stmt {
    Function(Token* name, Vector<Token*> params, Vector<Stmt*> body)
      : name(name)
      , params(params)
      , body(body) {}

    Token* name;
    Vector<Token*> params;
    Vector<Stmt*> body;

    STMT_ACCEPT_METHODS
  };

  struct If : public Stmt {
    If(Expr* condition, Stmt* thenBranch, Stmt* elseBranch)
      : condition(condition)
      , thenBranch(thenBranch)
      , elseBranch(elseBranch) {}

    Expr* condition;
    Stmt* thenBranch;
    Stmt* elseBranch;

    STMT_ACCEPT_METHODS
  };

  struct Print : public Stmt {
    Print(Token* print, Expr* expression)
      : print(print)
      , expression(expression) {}

    Token* print; // TODO
    Expr* expression;

    STMT_ACCEPT_METHODS
  };

  struct Return : public Stmt {
    Return(Expr* value)
      : value(value) {}

    Expr* value;

    STMT_ACCEPT_METHODS
  };

  struct Var : public Stmt {
    Var(Token* name, Expr* initializer)
      : name(name)
      , initializer(initializer) {}

    Token* name;
    Expr* initializer;

    STMT_ACCEPT_METHODS
  };

  struct While : public Stmt {
    While(Expr* condition, Stmt* body)
      : condition(condition)
      , body(body) {}

    Expr* condition;
    Stmt* body;

    STMT_ACCEPT_METHODS
  };

}; // namespace lox
