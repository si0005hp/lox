#include "vm.h"

#include "compiler.h"

namespace lox {

  InterpretResult VM::interpret(const char* source) {
    Compiler compiler(*this, TYPE_SCRIPT);
    compiler.compile(source);

    return INTERPRET_OK;
  }
} // namespace lox
