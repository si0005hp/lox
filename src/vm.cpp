#include "vm.h"

#include <stdarg.h>

#include <iostream>

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
      Obj* next = obj->next_;
      Memory::deallocate<Obj>(obj);
      obj = next;
    }
  }

  void VM::appendObj(Obj* obj) {
    obj->next_ = objects_;
    objects_ = obj;
  }

  ObjString* VM::findOrAllocateString(const char* src, int length) {
    ObjString* obj = strings.find(src, length);
    if (obj != nullptr) return obj;

    obj = ObjString::allocate(src, length);
    appendObj(obj);
    strings.add(obj);
    return obj;
  }

  InterpretResult VM::run(ObjFunction* function) {
    frame_.function = function;

    instruction inst;
    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
      traceStack();
      Disassembler::disassembleInstruction(currentChunk(), currentFrame().ip);
#endif

#define BINARY_OP(op)                                 \
  do {                                                \
    if (!peek(0).isNumber() || !peek(1).isNumber()) { \
      runtimeError("Operands must be numbers.");      \
      return INTERPRET_RUNTIME_ERROR;                 \
    }                                                 \
    Number b = pop().asNumber();                      \
    Number a = pop().asNumber();                      \
    push((op).asValue());                             \
  } while (false)

      switch (inst = readByte()) {
        case OP_POP: pop(); break;

        case OP_CONSTANT: {
          Value constant = readConstant();
          push(constant);
          break;
        }

        case OP_NIL: push(Nil().asValue()); break;
        case OP_TRUE: push(Bool(true).asValue()); break;
        case OP_FALSE: push(Bool(false).asValue()); break;

        case OP_EQUAL: {
          Value b = pop();
          Value a = pop();
          push(Bool(a == b).asValue());
          break;
        }

        case OP_NOT: push(Bool(pop().isFalsey()).asValue()); break;
        case OP_NEGATE: {
          if (!peek(0).isNumber()) {
            runtimeError("Operand must be a number.");
            return INTERPRET_RUNTIME_ERROR;
          }
          push((-pop().asNumber()).asValue());
          break;
        }

        case OP_ADD: {
          if (peek(0).isString() && peek(1).isString()) {
            UNREACHABLE(/* Not implemented. */);
          } else if (peek(0).isNumber() && peek(1).isNumber()) {
            Number b = pop().asNumber();
            Number a = pop().asNumber();
            push((a + b).asValue());
          } else {
            runtimeError("Operands must be two numbers or two strings.");
            return INTERPRET_RUNTIME_ERROR;
          }
          break;
        }
        case OP_SUBTRACT: BINARY_OP(a - b); break;
        case OP_MULTIPLY: BINARY_OP(a * b); break;
        case OP_DIVIDE: BINARY_OP(a / b); break;
        case OP_GREATER: BINARY_OP(Bool(a > b)); break;
        case OP_LESS: BINARY_OP(Bool(a < b)); break;

        case OP_PRINT: {
          std::cout << pop() << std::endl;
          break;
        }
        case OP_RETURN: {
          return INTERPRET_OK;
        }
      }
    }
  }

  void VM::traceStack() {
    std::cout << "          ";
    for (int i = 0; i < stackTop_; i++) std::cout << "[ " << stack_[i] << " ]";
    std::cout << std::endl;
  }

  void VM::runtimeError(const char* format, ...) const {
    // TODO
  }

} // namespace lox
