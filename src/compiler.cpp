#include "compiler.h"

#include "common.h"
#include "debug.h"
#include "lexer.h"
#include "op_code.h"
#include "parser.h"
#include "value/value.h"
#include "vm.h"

namespace lox {

  Compiler::Compiler(VM& vm, Compiler* parent, const char* source)
    : Compiler(vm, parent, source, nullptr, TYPE_SCRIPT) {}

  Compiler::Compiler(VM& vm, Compiler* parent, const Function* fn, FunctionType type)
    : Compiler(vm, parent, nullptr, fn, type) {}

  Compiler::Compiler(VM& vm, Compiler* parent, const char* source, const Function* fn,
                     FunctionType type)
    : lexer_(source)
    , vm_(vm)
    , enclosing_(parent)
    , locals_(Vector<Local>(LOCALS_MAX))
    , upvalues_(Vector<CompilerUpvalue>(UPVALUES_MAX)) {
    // TOOD: fix initialization logic
    vm_.setCompiler(this);

    if (!fn) {
      function_ = vm_.allocateObj<ObjFunction>(TYPE_SCRIPT, 0, nullptr);
    } else {
      ObjString* name = vm_.allocateObj<ObjString>(fn->name->start, fn->name->length);
      vm_.pushRoot(name);
      function_ = vm_.allocateObj<ObjFunction>(type, fn->params.size(), name);
      vm_.popRoot();
    }

    setFirstLocal();
  }

  void Compiler::setFirstLocal() {
    Local local(
      lexer_.syntheticToken(TOKEN_IDENTIFIER, function_->type() == TYPE_FUNCTION ? "" : "this"));
    local.depth = 0;
    locals_.push(local);
  }

  void Compiler::gcBlacken(VM& vm) const {
    vm.gcMarkObject(function_);
    // TODO: ast size
  }

  ObjFunction* Compiler::compile() {
    Parser parser(lexer_);

    if (!parser.parse()) return nullptr;

    const ParseResult& result = parser.result();
    for (int i = 0; i < result.stmts.size(); i++) {
      result.stmts[i]->accept(this);
    }

    endCompiler(result.eof);
    return hadError_ ? nullptr : function_;
  }

  void Compiler::endCompiler(SRC) {
    emitReturn(token);

#ifdef DEBUG_PRINT_CODE
    if (!hadError_)
      Disassembler::disassembleChunk(currentChunk(),
                                     function_->name() ? function_->name()->value() : "<script>");
#endif

    vm_.setCompiler(enclosing_); // TODO: accurate?
  }

  void Compiler::emitByte(SRC, instruction inst) {
    currentChunk().write(inst, token->line);
  }

  void Compiler::emitBytes(SRC, instruction inst1, instruction inst2) {
    emitByte(token, inst1);
    emitByte(token, inst2);
  }

  void Compiler::emitBytes(SRC, instruction inst1, instruction inst2, instruction inst3) {
    emitByte(token, inst1);
    emitByte(token, inst2);
    emitByte(token, inst3);
  }

  void Compiler::emitReturn(SRC) {
    if (function_->type() == TYPE_INITIALIZER)
      emitBytes(token, OP_GET_LOCAL, 0);
    else
      emitByte(token, OP_NIL);

    emitByte(token, OP_RETURN);
  }

  void Compiler::emitConstant(SRC, Value value) {
    emitBytes(token, OP_CONSTANT, makeConstant(token, value));
  }

  instruction Compiler::makeConstant(SRC, Value value) {
    instruction constant = addConstant(value);
    if (constant > UINT8_MAX) {
      error(token, "Too many constants in one chunk.");
      return 0;
    }
    return constant;
  }

  instruction Compiler::identifierConstant(Token* idt) {
    return makeConstant(idt, vm_.allocateObj<ObjString>(idt->start, idt->length)->asValue());
  }

  instruction Compiler::addConstant(Value value) {
    vm_.pushRoot(value);
    instruction constant = currentChunk().addConstant(value);
    vm_.popRoot();

    return constant;
  }

  // TODO: fix
  void Compiler::error(SRC, const char* message) {
    hadError_ = true;

    // TODO: duplicate logic: Parser::errorAt
    std::cerr << "[line " << token->line << "] Error";
    if (token->type == TOKEN_EOF) {
      std::cerr << " at end";
    } else if (token->type == TOKEN_ERROR) {
      // Nothing.
    } else {
      std::cerr << " at '" << std::string_view(token->start, token->length) << "'";
    }
    std::cerr << ": " << message << std::endl;
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
    if (locals_.size() >= LOCALS_MAX) {
      error(var, "Too many local variables in function.");
      return;
    }
    locals_.emplace(var);
  }

  void Compiler::defineVariable(Token* var, instruction global) {
    if (isLocalScope()) {
      markInitialized();
      return;
    }
    ASSERT(global != -1, "Global slot must be given.");
    emitBytes(var, OP_DEFINE_GLOBAL, global);
  }

  void Compiler::namedVariable(Token* name, bool isSetOp) {
    int index = resolveLocal(name);
    if (index != -1) {
      emitBytes(name, isSetOp ? OP_SET_LOCAL : OP_GET_LOCAL, index);
    } else if ((index = resolveUpvalue(name)) != -1) {
      emitBytes(name, isSetOp ? OP_SET_UPVALUE : OP_GET_UPVALUE, index);
    } else {
      instruction slot = identifierConstant(name);
      emitBytes(name, isSetOp ? OP_SET_GLOBAL : OP_GET_GLOBAL, slot);
    }
  }

  void Compiler::markInitialized() {
    ASSERT(isLocalScope(), "Must be in global scope.");

    locals_[-1].depth = scopeDepth_;
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

  int Compiler::resolveUpvalue(Token* name) {
    if (!enclosing_) return -1;

    int local = enclosing_->resolveLocal(name);
    if (local != -1) {
      enclosing_->locals_[local].isCapturedAsUpvalue = true;
      return addUpvalue(name, local, true);
    }

    int upvalue = enclosing_->resolveUpvalue(name);
    if (upvalue != -1) {
      return addUpvalue(name, upvalue, false);
    }

    return -1;
  }

  int Compiler::addUpvalue(SRC, int index, bool isLocal) {
    // Look for an existing one.
    for (int i = 0; i < function_->upvalueCount(); i++) {
      CompilerUpvalue& upvalue = upvalues_[i];
      if (upvalue.index == index && upvalue.isLocal == isLocal) return i;
    }

    if (function_->upvalueCount() == UPVALUES_MAX) {
      error(token, "Too many closure variables in function.");
      return 0;
    }
    // Add new upvalue.
    upvalues_.emplace(index, isLocal);
    return function_->getAndIncrementUpvalue();
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

  void Compiler::visit(const Call* expr) {
    if (typeid(*expr->callee) == typeid(Get)) {
      invoke(static_cast<Get*>(expr->callee), expr->arguments);
    } else if (typeid(*expr->callee) == typeid(Super)) {
      superInvoke(static_cast<Super*>(expr->callee), expr->arguments);
    } else {
      // Normal function call
      expr->callee->accept(this);
      compileArguments(expr->arguments);
      emitBytes(expr->callee->getStart(), OP_CALL, expr->arguments.size());
    }
  }

  void Compiler::invoke(const Get* get, const Vector<Expr*>& arguments) {
    get->object->accept(this);
    compileArguments(arguments);
    emitBytes(get->object->getStart(), OP_INVOKE, identifierConstant(get->name), arguments.size());
  }

  void Compiler::superInvoke(const Super* super, const Vector<Expr*>& arguments) {
    preprocessSuper(super);
    compileArguments(arguments);
    namedVariable(super->getStart()); // 'super'
    emitBytes(super->getStart(), OP_SUPER_INVOKE, identifierConstant(super->method),
              arguments.size());
  }

  void Compiler::compileArguments(const Vector<Expr*>& arguments) {
    ASSERT(arguments.size() <= MAX_FUNC_PARAMS, "Number of function args must be less than 255.");
    for (int i = 0; i < arguments.size(); i++) arguments[i]->accept(this);
  }

  void Compiler::visit(const Get* expr) {
    namedProperty(expr->object, expr->name);
  }

  void Compiler::namedProperty(Expr* receiver, Token* name, bool isSetOp) {
    receiver->accept(this);

    instruction slot = identifierConstant(name);
    emitBytes(name, isSetOp ? OP_SET_PROPERTY : OP_GET_PROPERTY, slot);
  }

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

  void Compiler::visit(const Set* expr) {
    expr->value->accept(this);

    namedProperty(expr->object, expr->name, true);
  }

  void Compiler::visit(const Super* expr) {
    preprocessSuper(expr);
    namedVariable(expr->getStart()); // 'super'
    emitBytes(expr->getStart(), OP_GET_SUPER, identifierConstant(expr->method));
  }

  void Compiler::preprocessSuper(const Super* super) {
    if (currentClass_ == nullptr) {
      error(super->getStart(), "Can't use 'super' outside of a class.");
    } else if (!currentClass_->hasSuperclass) {
      error(super->getStart(), "Can't use 'super' in a class with no superclass.");
    }

    namedVariable(lexer_.syntheticToken(TOKEN_THIS, "this"));
  }

  void Compiler::visit(const This* expr) {
    if (!currentClass_) {
      error(expr->keyword, "Can't use 'this' outside of a class.");
    }
    namedVariable(expr->keyword, false);
  }

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
      emitByte(token, local.isCapturedAsUpvalue ? OP_CLOSE_UPVALUE : OP_POP);
    }
  }

  void Compiler::compileBlock(const Vector<Stmt*>& stmts) {
    for (int i = 0; i < stmts.size(); i++) stmts[i]->accept(this);
  }

  void Compiler::visit(const Class* stmt) {
    instruction slot = identifierConstant(stmt->name);

    if (isLocalScope()) declareVariableLocal(stmt->name);
    emitBytes(stmt->name, OP_CLASS, slot);
    defineVariable(stmt->name, slot);

    ClassInfo classInfo(currentClass_, stmt->name, !!stmt->superclass);
    currentClass_ = &classInfo;

    if (stmt->superclass) {
      if (stmt->name == stmt->superclass->name)
        error(stmt->name, "A class can't inherit from itself.");

      namedVariable(stmt->superclass->name, false); // Push superclass

      beginScope();
      addLocal(lexer_.syntheticToken(TOKEN_SUPER, "super"));
      markInitialized();

      namedVariable(stmt->name, false); // Push subclass
      emitByte(stmt->superclass->name, OP_INHERIT);
    }

    namedVariable(stmt->name, false);
    for (int i = 0; i < stmt->methods.size(); i++) {
      compileMethod(stmt->methods[i]);
    }
    emitByte(stmt->getStop(), OP_POP); // Pop subclass

    if (stmt->superclass) endScope(stmt->superclass->name);
    currentClass_ = currentClass_->enclosing;
  }

  void Compiler::compileMethod(const Function* method) {
    instruction slot = identifierConstant(method->name);

    compileFunction(method,
                    stringEquals("init", method->name->start, 4) ? TYPE_INITIALIZER : TYPE_METHOD);
    emitBytes(method->name, OP_METHOD, slot);
  }

  void Compiler::visit(const Expression* stmt) {
    stmt->expression->accept(this);
    emitByte(stmt->stop, OP_POP);
  }

  void Compiler::visit(const Function* stmt) {
    instruction slot = parseVariable(stmt->name);
    if (isLocalScope()) markInitialized();

    compileFunction(stmt, TYPE_FUNCTION);
    defineVariable(stmt->name, slot);
  }

  void Compiler::compileFunction(const Function* fn, FunctionType type) {
    Compiler fnCompiler(vm_, this, fn, type);
    fnCompiler.doCompileFunction(fn);

    emitClosure(fn->getStart(), fnCompiler.function_, fnCompiler.upvalues_);
  }

  void Compiler::doCompileFunction(const Function* fn) {
    beginScope();

    ASSERT(fn->params.size() <= MAX_FUNC_PARAMS,
           "Number of function params must be less than 255.");
    for (int i = 0; i < fn->params.size(); i++) {
      // Function params are local variables.
      parseVariable(fn->params[i]);
      defineVariable(fn->params[i]);
    }

    for (int i = 0; i < fn->body.size(); i++) fn->body[i]->accept(this);
    endCompiler(fn->getStop());
  }

  void Compiler::emitClosure(SRC, ObjFunction* fn, const Vector<CompilerUpvalue>& upvalues) {
    emitBytes(token, OP_CLOSURE, makeConstant(token, fn->asValue()));

    for (int i = 0; i < upvalues.size(); i++) {
      // TODO: Token line is not consistent here.
      emitByte(token, upvalues[i].isLocal ? 1 : 0);
      emitByte(token, upvalues[i].index);
    }
  }

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

  void Compiler::visit(const Return* stmt) {
    if (function_->type() == TYPE_SCRIPT)
      error(stmt->getStart(), "Can't return from top-level code.");

    if (stmt->value) {
      if (function_->type() == TYPE_INITIALIZER) {
        error(stmt->getStart(), "Can't return a value from an initializer.");
      }
      stmt->value->accept(this);
      emitByte(stmt->value->getStart(), OP_RETURN);
    } else {
      emitReturn(stmt->getStart());
    }
  }

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
