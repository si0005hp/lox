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
    , type_(type) {
    function_ = vm.allocateObj<ObjFunction>(0, nullptr);
  }

  ObjFunction* Compiler::compile(const char* source) {
    Lexer lexer(source);
    Parser parser(lexer);

    if (!parser.parse()) return nullptr;

    int constant = function_->chunk.addConstant(Number(-2.3).asValue());
    function_->chunk.write(OP_CONSTANT, 123);
    function_->chunk.write(constant, 123);

    constant = function_->chunk.addConstant(Number(4.5).asValue());
    function_->chunk.write(OP_CONSTANT, 123);
    function_->chunk.write(constant, 123);

    function_->chunk.write(OP_ADD, 123);

    function_->chunk.write(OP_RETURN, 123);

    return function_;
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
      case TOKEN_PLUS: emitByte(expr->op, OP_ADD); break;
      case TOKEN_MINUS: emitByte(expr->op, OP_SUBTRACT); break;
      case TOKEN_STAR: emitByte(expr->op, OP_MULTIPLY); break;
      case TOKEN_SLASH: emitByte(expr->op, OP_DIVIDE); break;
      default: UNREACHABLE();
    }
  }

  void Compiler::visit(const Call* expr) {}

  void Compiler::visit(const Get* expr) {}

  void Compiler::visit(const Grouping* expr) {}

  void Compiler::visit(const Literal* expr) {
    if (expr->value->type == TOKEN_NUMBER) {
      number(expr);
    }
  }

  void Compiler::number(const Literal* literal) {
    double value = std::strtod(literal->value->start, 0);
    emitConstant(literal->value, Number(value).asValue());
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

  void Compiler::visit(const Expression* stmt) {}

  void Compiler::visit(const Function* stmt) {}

  void Compiler::visit(const If* stmt) {}

  void Compiler::visit(const Print* stmt) {}

  void Compiler::visit(const Return* stmt) {}

  void Compiler::visit(const Var* stmt) {}

  void Compiler::visit(const While* stmt) {}

}; // namespace lox
