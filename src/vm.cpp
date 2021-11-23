#include "vm.h"

#include "compiler.h"
#include "memory.h"

namespace lox {

  VM::VM() {
    Memory::initialize(this);
  }

  VM::~VM() {
    freeObject();
  }

  InterpretResult VM::interpret(const char* source) {
    Compiler compiler(*this, TYPE_SCRIPT);
    compiler.compile(source);

    return INTERPRET_OK;
  }

  void VM::freeObject() {
    Obj* obj = objects_;
    while (obj != nullptr) {
      Obj* next = obj->next;
      Memory::deallocate<Obj>(obj);
      obj = next;
    }
  }
} // namespace lox
