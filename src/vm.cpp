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
      traceStack();
      Disassembler::disassembleInstruction(currentChunk(), currentFrame().ip);
#endif

      switch (inst = readByte()) {
        case OP_CONSTANT: {
          Value constant = readConstant();
          push(constant);
          break;
        }
        case OP_RETURN: {
          printf("%s\n", pop().toCString());
          return INTERPRET_OK;
        }
      }
    }
  }

  void VM::traceStack() {
    printf("          ");
    for (int i = 0; i < stackTop_; i++) {
      printf("[ %s ]", stack_[i].toCString());
    }
    printf("\n");
  }
} // namespace lox
