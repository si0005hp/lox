#include "compiler.h"

#include "common.h"
#include "debug.h"
#include "lexer.h"
#include "op_code.h"
#include "parser.h"
#include "value/value.h"

namespace lox {

  Compiler::Compiler(const char* source, VM& vm, FunctionType type)
    : lexer_(source)
    , vm_(vm)
    , locals_(Vector<Local>(LOCALS_MAX)) {
    function_ = vm.allocateObj<ObjFunction>(type, 0, nullptr);
    setFirstLocal();
  }

  void Compiler::setFirstLocal() {
    Local local(
      lexer_.syntheticToken(TOKEN_IDENTIFIER, function_->type() == TYPE_FUNCTION ? "" : "this"));
    local.depth = 0;
    locals_.push(local);
  }

  ObjFunction* Compiler::compile() {
    Parser parser(lexer_);

    if (!parser.parse()) return nullptr;

    Vector<Stmt*> stmts = parser.result().stmts;
    for (int i = 0; i < stmts.size(); i++) {
      stmts[i]->accept(this);
    }

    endCompiler(parser.result().eof);
    return hadError_ ? nullptr : function_;
  }

  void Compiler::endCompiler(SRC) {
    emitReturn(token);

#ifdef DEBUG_PRINT_CODE
    if (!hadError_)
      Disassembler::disassembleChunk(currentChunk(),
                                     function_->name() ? function_->name()->value() : "<script>");
#endif
  }

  void Compiler::emitByte(SRC, instruction inst) {
    currentChunk().write(inst, token->line);
  }

  void Compiler::emitBytes(SRC, instruction inst1, instruction inst2) {
    emitByte(token, inst1);
    emitByte(token, inst2);
  }

  void Compiler::emitReturn(SRC) {
    emitByte(token, OP_RETURN);
  }

  void Compiler::emitConstant(SRC, Value value) {
    emitBytes(token, OP_CONSTANT, makeConstant(token, value));
  }

  instruction Compiler::makeConstant(SRC, Value value) {
    instruction constant = currentChunk().addConstant(value);
    if (constant > UINT8_MAX) {
      error(token, "Too many constants in one chunk.");
      return 0;
    }
    return constant;
  }

  instruction Compiler::identifierConstant(Token* idt) {
    return makeConstant(idt, vm_.allocateObj<ObjString>(idt->start, idt->length)->asValue());
  }

  void Compiler::error(SRC, const char* message) {
    // TODO: implement
  }

  instruction Compiler::parseVariable(Token* var) {
    if (isLocalScope()) {
      declareVariableLocal(var);
      return -1; // dummy
    }
    return identifierConstant(var);
  }

  void Compiler::declareVariableLocal(Token* var) {
    if (!isLocalScope()) return;

    // Check duplication
    for (int i = locals_.size() - 1; i >= 0; i--) {
      if (locals_[i].isInitialized() && locals_[i].depth < scopeDepth_) break;

      if (*locals_[i].name == *var) error(var, "Already variable with this name in this scope.");
    }
    addLocal(var);
  }

  void Compiler::addLocal(Token* var) {
    if (locals_.size() > LOCALS_MAX) {
      error(var, "Too many local variables.");
      return;
    }
    locals_.emplace(var);
  }

  void Compiler::defineVariable(Token* var, instruction global) {
    if (isLocalScope()) {
      // markInitialized();
      return;
    }
    ASSERT(global != -1, "Global slot must be given.");
    emitBytes(var, OP_DEFINE_GLOBAL, global);
  }

  void Compiler::namedVariable(Token* name, bool isSetOp) {
    int index = resolveLocal(name);
    if (index != -1) {
      emitBytes(name, isSetOp ? OP_SET_LOCAL : OP_GET_LOCAL, index);
    } else {
      instruction slot = identifierConstant(name);
      emitBytes(name, isSetOp ? OP_SET_GLOBAL : OP_GET_GLOBAL, slot);
    }
  }

  int Compiler::resolveLocal(Token* name) {
    for (int i = locals_.size() - 1; i >= 0; i--) {
      if (*locals_[i].name == *name) {
        if (!locals_[i].isInitialized())
          error(name, "Can't read local variable in its own initializer.");
        return i;
      }
    }
    return -1; // Not found
  }

  void Compiler::visit(const Assign* expr) {
    expr->value->accept(this);
    namedVariable(expr->name, true);
  }

  void Compiler::visit(const Binary* expr) {
    expr->left->accept(this);
    expr->right->accept(this);

    switch (expr->op->type) {
      case TOKEN_BANG_EQUAL: emitBytes(expr->op, OP_EQUAL, OP_NOT); break;
      case TOKEN_EQUAL_EQUAL: emitByte(expr->op, OP_EQUAL); break;
      case TOKEN_GREATER: emitByte(expr->op, OP_GREATER); break;
      case TOKEN_GREATER_EQUAL: emitBytes(expr->op, OP_LESS, OP_NOT); break;
      case TOKEN_LESS: emitByte(expr->op, OP_LESS); break;
      case TOKEN_LESS_EQUAL: emitBytes(expr->op, OP_GREATER, OP_NOT); break;
      case TOKEN_PLUS: emitByte(expr->op, OP_ADD); break;
      case TOKEN_MINUS: emitByte(expr->op, OP_SUBTRACT); break;
      case TOKEN_STAR: emitByte(expr->op, OP_MULTIPLY); break;
      case TOKEN_SLASH: emitByte(expr->op, OP_DIVIDE); break;
      default: UNREACHABLE();
    }
  }

  void Compiler::visit(const Call* expr) {}

  void Compiler::visit(const Get* expr) {}

  void Compiler::visit(const Grouping* expr) {
    expr->expression->accept(this);
  }

  void Compiler::visit(const Literal* expr) {
    Token* value = expr->value;
    switch (value->type) {
      case TOKEN_NUMBER: {
        double n = std::strtod(value->start, 0);
        emitConstant(value, Number(n).asValue());
        break;
      }
      case TOKEN_FALSE: emitByte(value, OP_FALSE); break;
      case TOKEN_NIL: emitByte(value, OP_NIL); break;
      case TOKEN_TRUE: emitByte(value, OP_TRUE); break;
      case TOKEN_STRING: {
        // Trim double quotes.
        ObjString* s = vm_.allocateObj<ObjString>(value->start + 1, value->length - 2);
        emitConstant(value, s->asValue());
        break;
      }
      default: UNREACHABLE();
    }
  }

  void Compiler::visit(const Logical* expr) {
    expr->left->accept(this);

    int jumpOffset = emitJump(expr->op, expr->op->type == TOKEN_AND ? OP_AND : OP_OR);
    expr->right->accept(this);
    patchJump(expr->op, jumpOffset);
  }

  void Compiler::visit(const Set* expr) {}

  void Compiler::visit(const Super* expr) {}

  void Compiler::visit(const This* expr) {}

  void Compiler::visit(const Unary* expr) {
    expr->right->accept(this);

    switch (expr->op->type) {
      case TOKEN_BANG: emitByte(expr->op, OP_NOT); break;
      case TOKEN_MINUS: emitByte(expr->op, OP_NEGATE); break;
      default: UNREACHABLE();
    }
  }

  void Compiler::visit(const Variable* expr) {
    namedVariable(expr->name);
  }

  void Compiler::visit(const Block* stmt) {
    beginScope();
    compileBlock(stmt->statements);
    endScope(stmt->getStop());
  }

  void Compiler::endScope(SRC) {
    scopeDepth_--;
    while (!locals_.isEmpty() && locals_[-1].depth > scopeDepth_) {
      Local local = locals_.removeAt(-1);
      emitByte(token, local.isCaptured ? OP_CLOSE_UPVALUE : OP_POP);
    }
  }

  void Compiler::compileBlock(Vector<Stmt*> stmts) {
    for (int i = 0; i < stmts.size(); i++) stmts[i]->accept(this);
  }

  void Compiler::visit(const Class* stmt) {}

  void Compiler::visit(const Expression* stmt) {
    stmt->expression->accept(this);
    emitByte(stmt->stop, OP_POP);
  }

  void Compiler::visit(const Function* stmt) {}

  void Compiler::visit(const If* stmt) {
    stmt->condition->accept(this);

    int thenJumpOffset = emitJump(stmt->getStart(), OP_JUMP_IF_FALSE);
    emitByte(stmt->getStart(), OP_POP);

    stmt->thenBranch->accept(this);
    int elseJumpOffset = emitJump(stmt->getStart(), OP_JUMP);

    patchJump(stmt->getStart(), thenJumpOffset);
    emitByte(stmt->getStart(), OP_POP);

    if (stmt->elseBranch) stmt->elseBranch->accept(this);

    patchJump(stmt->getStart(), elseJumpOffset);
  }

  int Compiler::emitJump(SRC, instruction opCode) {
    emitByte(token, opCode);
    emitByte(token, 0xff);
    emitByte(token, 0xff);
    return currentChunk().count() - 2;
  }

  void Compiler::patchJump(SRC, int offset) {
    int jump = currentChunk().count() - offset - 2;

    if (jump > UINT16_MAX) {
      error(token, "Too much code to jump over.");
    }

    currentChunk().rewrite(offset, (jump >> 8) & 0xff);
    currentChunk().rewrite(offset + 1, jump & 0xff);
  }

  void Compiler::visit(const Print* stmt) {
    stmt->expression->accept(this);
    emitByte(stmt->start, OP_PRINT);
  }

  void Compiler::visit(const Return* stmt) {}

  void Compiler::visit(const Var* stmt) {
    instruction slot = parseVariable(stmt->name);

    if (stmt->initializer)
      stmt->initializer->accept(this);
    else
      emitByte(stmt->name, OP_NIL);

    defineVariable(stmt->name, slot);
  }

  void Compiler::visit(const While* stmt) {
    int loopStart = currentChunk().count();

    stmt->condition->accept(this);
    int exitJumpOffset = emitJump(stmt->getStart(), OP_JUMP_IF_FALSE);
    emitByte(stmt->getStart(), OP_POP);

    stmt->body->accept(this);
    emitLoop(stmt->getStart(), loopStart);

    patchJump(stmt->getStart(), exitJumpOffset);
    emitByte(stmt->getStart(), OP_POP);
  }

  void Compiler::emitLoop(SRC, int loopStart) {
    int backJumpDistance = currentChunk().count() - loopStart + 3;
    if (backJumpDistance > UINT16_MAX) error(token, "Loop body too large.");

    emitByte(token, OP_LOOP);
    emitByte(token, (backJumpDistance >> 8) & 0xff);
    emitByte(token, backJumpDistance & 0xff);
  }

}; // namespace lox
