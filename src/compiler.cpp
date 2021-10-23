#include "compiler.h"

namespace lox {
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
