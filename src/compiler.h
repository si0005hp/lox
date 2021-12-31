#pragma once

#include "ast.h"
#include "lexer.h"
#include "value/object.h"

namespace lox {

#define SRC Token* token

  struct Local {
    Local() {} // TODO: Essentially not necessary
    Local(Token* name)
      : name(name) {}

    bool isInitialized() const {
      return depth != -1;
    }

    Token* name = nullptr;
    int depth = -1;
    bool isCapturedAsUpvalue = false;
  };

  struct CompilerUpvalue {
    CompilerUpvalue() {} // TODO: Essentially not necessary
    CompilerUpvalue(int index, bool isLocal)
      : index(index)
      , isLocal(isLocal) {}

    int index = -1;
    bool isLocal = false;
  };

  struct ClassInfo {
    ClassInfo(ClassInfo* enclosing, Token* name, bool hasSuperclass)
      : enclosing(enclosing)
      , name(name)
      , hasSuperclass(hasSuperclass) {}

    ClassInfo* enclosing;
    Token* name;
    bool hasSuperclass;
  };

  class Compiler
    : public Expr::Visitor<void>
    , public Stmt::Visitor<void> {
    friend class VM;

   public:
    Compiler(VM& vm, Compiler* parent, const char* source);
    Compiler(VM& vm, Compiler* parent, const Function* fn, FunctionType type);

    ObjFunction* compile();

    void gcBlacken(VM& vm) const;

   private:
    Compiler(VM& vm, Compiler* parent, const char* source, const Function* fn, FunctionType type);

    void setFirstLocal();

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
    void emitBytes(SRC, instruction inst1, instruction inst2, instruction inst3);
    void emitReturn(SRC);
    void emitConstant(SRC, Value value);

    instruction makeConstant(SRC, Value value);
    instruction identifierConstant(SRC);
    instruction addConstant(Value value);
    void error(SRC, const char* message);

    instruction parseVariable(Token* var);
    void declareVariableLocal(Token* var);
    void addLocal(Token* var);
    void defineVariable(Token* var, instruction global = -1);
    void namedVariable(Token* name, bool isSetOp = false);
    void markInitialized();

    void namedProperty(Expr* receiver, Token* name, bool isSetOp = false);

    int resolveLocal(Token* name);
    int resolveUpvalue(Token* name);
    int addUpvalue(SRC, int index, bool isLocal);

    bool isLocalScope() const {
      return scopeDepth_ > 0;
    }

    void beginScope() {
      scopeDepth_++;
    }

    void endScope(SRC);

    void compileBlock(const Vector<Stmt*>& stmts);

    int emitJump(SRC, instruction opCode);
    void patchJump(SRC, int offset);
    void emitLoop(SRC, int loopStart);

    static constexpr int MAX_FUNC_PARAMS = 255;
    void compileFunction(const Function* fn, FunctionType type);
    void doCompileFunction(const Function* fn);
    void emitClosure(SRC, ObjFunction* fn, const Vector<CompilerUpvalue>& upvalues);
    void compileArguments(const Vector<Expr*>& arguments);

    void compileMethod(const Function* method);

    void invoke(const Get* get, const Vector<Expr*>& arguments);
    void superInvoke(const Super* super, const Vector<Expr*>& arguments);
    void preprocessSuper(const Super* super);

   private:
    Lexer lexer_;
    VM& vm_;
    Compiler* enclosing_ = nullptr;

    ObjFunction* function_ = nullptr;

    bool hadError_ = false;

    static constexpr int LOCALS_MAX = 256;
    Vector<Local> locals_; // TODO: Fixed size container
    int scopeDepth_ = 0;

    static constexpr int UPVALUES_MAX = 256;
    Vector<CompilerUpvalue> upvalues_; // TODO: Fixed size container

    // Pointer to the Class info that is currently being compiled.
    ClassInfo* currentClass_ = nullptr;
  };

}; // namespace lox
