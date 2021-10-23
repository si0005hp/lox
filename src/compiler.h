#pragma once

#include "ast.h"

namespace lox {

  class Compiler
    : public Expr::Visitor<void>
    , public Stmt::Visitor<void> {

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
  };

}; // namespace lox
