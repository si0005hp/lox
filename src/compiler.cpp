#include "compiler.h"

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

    Vector<Stmt*> stmts = parser.parse();
    if (parser.hadError()) return nullptr;

    int constant = function_->chunk.addConstant(Number(2.3).asValue());
    function_->chunk.write(OP_CONSTANT, 123);
    function_->chunk.write(constant, 123);

    function_->chunk.write(OP_RETURN, 123);
    Disassembler::disassembleChunk(function_->chunk, "test");

    return nullptr;
  }

  void Compiler::visit(const Assign* expr) {}
  void Compiler::visit(const Binary* expr) {}
  void Compiler::visit(const Call* expr) {}
  void Compiler::visit(const Get* expr) {}
  void Compiler::visit(const Grouping* expr) {}
  void Compiler::visit(const Literal* expr) {}
  void Compiler::visit(const Logical* expr) {}
  void Compiler::visit(const Set* expr) {}
  void Compiler::visit(const Super* expr) {}
  void Compiler::visit(const This* expr) {}
  void Compiler::visit(const Unary* expr) {}
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
