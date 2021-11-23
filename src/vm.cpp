#include "vm.h"

#include "chunk.h"
#include "compiler.h"
#include "memory.h"
#include "value/value.h"

namespace lox {

  VM::VM()
    : objects_(nullptr) {
    Memory::initialize(this);
  }

  VM::~VM() {
    freeObjects();
  }

  InterpretResult VM::interpret(const char* source) {
    Compiler compiler(*this, TYPE_SCRIPT);
    ObjFunction* function = compiler.compile(source);
    if (function == nullptr) return INTERPRET_COMPILE_ERROR;

    run(function->chunk);
    return INTERPRET_OK;
  }

  void VM::freeObjects() {
    Obj* obj = objects_;
    while (obj != nullptr) {
      Obj* next = obj->next;
      Memory::deallocate<Obj>(obj);
      obj = next;
    }
  }

  void VM::run(Chunk& chunk) {}
} // namespace lox
