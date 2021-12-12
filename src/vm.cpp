#include "vm.h"

#include <stdarg.h>

#include <iostream>

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "op_code.h"
#include "value/object.h"
#include "value/value.h"

namespace lox {

  VM::VM(std::ostream& out)
    : out_(out) {
    Memory::initialize(this);
  }

  VM::~VM() {
    freeObjects();
  }

  InterpretResult VM::interpret(const char* source) {
    Compiler compiler(*this, nullptr, source);
    ObjFunction* function = compiler.compile();
    if (!function) return INTERPRET_COMPILE_ERROR;

    push(function->asValue());
    callValue(function->asValue(), 0);
    return run();
  }

  void VM::freeObjects() {
    Obj* obj = objects_;
    while (obj) {
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
    if (obj) return obj;

    obj = ObjString::allocate(src, length);
    appendObj(obj);
    strings.add(obj);
    return obj;
  }

  void VM::appendCallFrame(ObjFunction* function, int stackStart) {
    frames_[frameCount_++] = CallFrame(function, stackStart);
  }

  InterpretResult VM::run() {
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

        case OP_GET_LOCAL: {
          instruction slot = readByte();
          push(load(currentStackStart() + slot));
          break;
        }
        case OP_SET_LOCAL: {
          instruction slot = readByte();
          store(currentStackStart() + slot, peek(0));
          break;
        }

        case OP_GET_GLOBAL: {
          ObjString* name = readString();
          Value value;
          if (!globals.get(name, &value)) {
            runtimeError("Undefined variable '%s'.", name->value());
            return INTERPRET_RUNTIME_ERROR;
          }
          push(value);
          break;
        }
        case OP_SET_GLOBAL: {
          ObjString* name = readString();
          if (!globals.containsKey(name)) {
            runtimeError("Undefined variable '%s'.", name->value());
            return INTERPRET_RUNTIME_ERROR;
          }
          globals.put(name, peek(0));
          break;
        }

        case OP_DEFINE_GLOBAL: {
          ObjString* name = readString();
          globals.put(name, peek(0));
          pop();
          break;
        }

        case OP_CONSTANT: push(readConstant()); break;
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
            ObjString* b = pop().asString();
            ObjString* a = pop().asString();
            push(concatString(a, b)->asValue());
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
          out_ << pop() << std::endl;
          break;
        }

        case OP_JUMP: {
          uint16_t offset = readShort();
          currentFrame().ip += offset;
          break;
        }
        case OP_JUMP_IF_FALSE: {
          uint16_t offset = readShort();
          if (peek(0).isFalsey()) currentFrame().ip += offset;
          break;
        }
        case OP_LOOP: {
          uint16_t offset = readShort();
          currentFrame().ip -= offset;
          break;
        }
        case OP_AND: {
          uint16_t offset = readShort();
          if (peek(0).isFalsey()) {
            currentFrame().ip += offset;
          } else {
            pop();
          }
          break;
        }
        case OP_OR: {
          uint16_t offset = readShort();
          if (peek(0).isFalsey()) {
            pop();
          } else {
            currentFrame().ip += offset;
          }
          break;
        }

        case OP_CALL: {
          int argCount = readByte();
          Value callee = peek(argCount);
          if (!callValue(callee, argCount)) {
            return INTERPRET_RUNTIME_ERROR;
          }
          break;
        }

        case OP_RETURN: {
          // Save data for subsequent processes.
          Value result = pop();
          int frameStackStart = currentStackStart();

          frameCount_--;
          if (frameCount_ == 0) {
            // Top-level done. Pop global script out and finish.
            pop();
            return INTERPRET_OK;
          }

          // Truncate stack of the frame.
          stackTop_ = frameStackStart;
          push(result);
          break;
        }
      }
    }
  }

  bool VM::callValue(Value callee, int argCount) {
    if (callee.isFunction()) {
      return call(callee.asFunction(), argCount);
    }
    runtimeError("Can only call functions and classes.");
    return false;
  }

  bool VM::call(ObjFunction* function, int argCount) {
    if (argCount != function->arity()) {
      runtimeError("Expected %d arguments but got %d.", function->arity(), argCount);
      return false;
    }
    if (frameCount_ == FRAMES_MAX) {
      runtimeError("Stack overflow.");
      return false;
    }

    appendCallFrame(function, stackTop_ - argCount - 1);
    return true;
  }

  void VM::traceStack() {
    std::cout << "          ";
    for (int i = 0; i < stackTop_; i++) std::cout << "[ " << stack_[i] << " ]";
    std::cout << std::endl;
  }

  void VM::runtimeError(const char* format, ...) const {
    // TODO: implement
  }

  ObjString* VM::concatString(ObjString* left, ObjString* right) {
    // Avoid GC
    push(left->asValue());
    push(right->asValue());

    int length = left->length() + right->length();
    char* chars = Memory::allocate<char>(length + 1);
    memcpy(chars, left->value(), left->length());
    memcpy(chars + right->length(), right->value(), right->length());
    chars[length] = '\0';

    ObjString* result = allocateObj<ObjString>(chars, length);
    Memory::reallocate(chars, sizeof(char) * length, 0);
    pop();
    pop();
    return result;
  }

} // namespace lox
