#include "vm.h"

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "op_code.h"
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

    return run(function);
  }

  void VM::freeObjects() {
    Obj* obj = objects_;
    while (obj != nullptr) {
      Obj* next = obj->next;
      Memory::deallocate<Obj>(obj);
      obj = next;
    }
  }

  InterpretResult VM::run(ObjFunction* function) {
    frame_.function = function;

    instruction inst;
    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
      Disassembler::disassembleInstruction(currentChunk(), currentFrame().ip);
#endif

      switch (inst = readByte()) {
        case OP_CONSTANT: {
          Value constant = readConstant();
          printf("%s", constant.toCString());
          printf("\n");
          break;
        }

        case OP_RETURN: return INTERPRET_OK;
      }
    }
  }
} // namespace lox
