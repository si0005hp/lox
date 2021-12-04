#pragma once

#include <type_traits>

#include "common.h"
#include "lib/vector.h"
#include "string_table.h"
#include "value/object.h"
#include "value/value.h"

namespace lox {

  enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
  };

  struct CallFrame {
    int ip = 0;
    ObjFunction* function = nullptr;
  };

  class VM {
   public:
    VM();
    ~VM();

    InterpretResult interpret(const char* source);

    // TODO: Right place to manage heap allocation?
    template <typename T, typename... Args>
    T* allocateObj(Args&&... args) {
      if constexpr (std::is_same_v<T, ObjString>) {
        return findOrAllocateString(std::forward<Args>(args)...);
      }
      T* obj = T::allocate(std::forward<Args>(args)...);
      appendObj(obj);
      return obj;
    }

   private:
    void freeObjects();

    void appendObj(Obj* obj);

    ObjString* findOrAllocateString(const char* src, int length);

    InterpretResult run(ObjFunction* function);

    void traceStack();

    void runtimeError(const char* format, ...) const;

    instruction readByte() {
      return currentChunk().getCode(currentFrame().ip++);
    }

    Value readConstant() {
      return currentChunk().getConstant(readByte());
    }

    ObjString* readString() {
      return readConstant().asString();
    }

    CallFrame& currentFrame() {
      return frame_;
    };

    const CallFrame& currentFrame() const {
      return frame_;
    };

    ObjFunction* currentFn() const {
      return currentFrame().function;
    };

    const Chunk& currentChunk() const {
      return currentFn()->chunk();
    };

    void push(Value value) {
      stack_[stackTop_++] = value;
    }

    Value pop() {
      stackTop_--;
      return stack_[stackTop_];
    }

    Value peek(int offset) const {
      return stack_[stackTop_ - 1 - offset];
    }

   private:
    Obj* objects_;
    CallFrame frame_; // TODO: tmp

    std::array<Value, 256> stack_;
    int stackTop_ = 0;

    StringTable strings;
    Map<StringKey, Value> globals;
  };
} // namespace lox
