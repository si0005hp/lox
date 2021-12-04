#pragma once

#include "ast.h"
#include "value/object.h"
#include "vm.h"

namespace lox {

#define SRC Token* token

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

    ObjFunction* compile(const char* source);

   private:
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

    Chunk& currentChunk() const {
      return function_->chunk();
    };

    void endCompiler(SRC);

    void emitByte(SRC, instruction inst);
    void emitBytes(SRC, instruction inst1, instruction inst2);
    void emitReturn(SRC);
    void emitConstant(SRC, Value value);

    int makeConstant(SRC, Value value);
    int identifierConstant(SRC);
    void error(SRC, const char* message);

   private:
    VM& vm_;

    ObjFunction* function_;
    FunctionType type_;

    bool hadError_ = false;
  };

}; // namespace lox
