#include "../lib/vector.h"

#define V_STMT_ACCEPT_METHODS virtual void accept(Visitor<void>& visitor) const = 0;

#define STMT_ACCEPT_METHODS                            \
  void accept(Visitor<void>& visitor) const override { \
    return visitor.visit(this);                        \
  }

namespace lox {

  class Token;
  class Value;
  class Variable;
  class Expr;

  class Block;
  class Class;
  class Expression;
  class Function;
  class If;
  class Print;
  class Return;
  class Var;
  class While;

  struct Stmt {
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
    Expression(Expr* expression)
      : expression(expression) {}

    Expr* expression;

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
    Print(Expr* expression)
      : expression(expression) {}

    Expr* expression;

    STMT_ACCEPT_METHODS
  };

  struct Return : public Stmt {
    Return(Token* keyword, Expr* value)
      : keyword(keyword)
      , value(value) {}

    Token* keyword;
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