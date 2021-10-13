#include "../lib/vector.h"

#define V_EXPR_ACCEPT_METHODS                                \
  virtual Value* accept(Visitor<Value*>& visitor) const = 0; \
  virtual void accept(Visitor<void>& visitor) const = 0;

#define EXPR_ACCEPT_METHODS                                \
  Value* accept(Visitor<Value*>& visitor) const override { \
    return visitor.visit(this);                            \
  }                                                        \
  void accept(Visitor<void>& visitor) const override {     \
    return visitor.visit(this);                            \
  }

namespace lox {

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

  struct Expr {
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
    Call(Expr* callee, Token* paren, Vector<Expr*> arguments)
      : callee(callee)
      , paren(paren)
      , arguments(arguments) {}

    Expr* callee;
    Token* paren;
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

}; // namespace lox