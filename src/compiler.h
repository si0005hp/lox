#pragma once

#include "ast.h"
#include "value/object.h"
#include "vm.h"

namespace lox {

  enum FunctionType {
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT
  };

  class Compiler
    : public Expr::Visitor<void>
    , public Stmt::Visitor<void> {

   public:
    Compiler(VM& vm, FunctionType type);
    ~Compiler();

    ObjFunction* compile(const char* source);

    virtual void visit(const Assign* expr);
    virtual void visit(const Binary* expr);
    virtual void visit(const Call* expr);
    virtual void visit(const Get* expr);
    virtual void visit(const Grouping* expr);
    virtual void visit(const Literal* expr);
    virtual void visit(const Logical* expr);
    virtual void visit(const Set* expr);
    virtual void visit(const Super* expr);
    virtual void visit(const This* expr);
    virtual void visit(const Unary* expr);
    virtual void visit(const Variable* expr);

    virtual void visit(const Block* stmt);
    virtual void visit(const Class* stmt);
    virtual void visit(const Expression* stmt);
    virtual void visit(const Function* stmt);
    virtual void visit(const If* stmt);
    virtual void visit(const Print* stmt);
    virtual void visit(const Return* stmt);
    virtual void visit(const Var* stmt);
    virtual void visit(const While* stmt);

   private:
    VM& vm_;

    ObjFunction* function_;
    FunctionType type_;
  };

}; // namespace lox
