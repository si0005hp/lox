#include "compiler.h"

#include "common.h"
#include "debug.h"
#include "lexer.h"
#include "op_code.h"
#include "parser.h"
#include "value/value.h"

namespace lox {

  Compiler::Compiler(VM& vm, FunctionType type)
    : vm_(vm)
    , type_(type)
    , hadError_(false) {
    function_ = vm.allocateObj<ObjFunction>(0, nullptr);
  }

  ObjFunction* Compiler::compile(const char* source) {
    Lexer lexer(source);
    Parser parser(lexer);

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
    if (!hadError_) Disassembler::disassembleChunk(currentChunk(), "code");
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

  int Compiler::makeConstant(SRC, Value value) {
    int constant = currentChunk().addConstant(value);
    if (constant > UINT8_MAX) {
      error(token, "Too many constants in one chunk.");
      return 0;
    }
    return constant;
  }

  int Compiler::identifierConstant(SRC) {
    return 0;
  }

  void Compiler::error(SRC, const char* message) {}

  void Compiler::visit(const Assign* expr) {}

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

  void Compiler::visit(const Logical* expr) {}

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

  void Compiler::visit(const Variable* expr) {}

  void Compiler::visit(const Block* stmt) {}

  void Compiler::visit(const Class* stmt) {}

  void Compiler::visit(const Expression* stmt) {
    stmt->expression->accept(this);
    emitByte(stmt->stop, OP_POP);
  }

  void Compiler::visit(const Function* stmt) {}

  void Compiler::visit(const If* stmt) {}

  void Compiler::visit(const Print* stmt) {
    stmt->expression->accept(this);
    emitByte(stmt->print, OP_PRINT);
  }

  void Compiler::visit(const Return* stmt) {}

  void Compiler::visit(const Var* stmt) {}

  void Compiler::visit(const While* stmt) {}

}; // namespace lox
