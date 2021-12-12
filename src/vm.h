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
    CallFrame() {}

    CallFrame(ObjFunction* function, int stackStart)
      : function(function)
      , stackStart(stackStart) {}

    int ip = 0;
    ObjFunction* function = nullptr;
    int stackStart = 0;
  };

  class VM {
   public:
    VM(std::ostream& out = std::cout);

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

    InterpretResult run();

    void traceStack();

    void runtimeError(const char* format, ...) const;

    void appendCallFrame(ObjFunction* function, int stackStart);

    bool callValue(Value callee, int argCount);
    bool call(ObjFunction* function, int argCount);

    instruction readByte() {
      return currentChunk().getCode(currentFrame().ip++);
    }

    uint16_t readShort() {
      instruction firstByte = currentChunk().getCode(currentFrame().ip++);
      instruction secondByte = currentChunk().getCode(currentFrame().ip++);
      return (uint16_t)(firstByte << 8 | secondByte);
    }

    Value readConstant() {
      return currentChunk().getConstant(readByte());
    }

    ObjString* readString() {
      return readConstant().asString();
    }

    CallFrame& currentFrame() {
      return frames_[frameCount_ - 1];
    };

    const CallFrame& currentFrame() const {
      return frames_[frameCount_ - 1];
    };

    ObjFunction* currentFn() const {
      return currentFrame().function;
    };

    const Chunk& currentChunk() const {
      return currentFn()->chunk();
    };

    int currentStackStart() const {
      return currentFrame().stackStart;
    }

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

    Value load(int index) const {
      return stack_[index];
    }

    void store(int index, Value value) {
      // Access from the tail if the index is negative
      if (index < 0) index += stackTop_;
      stack_[index] = value;
    }

    ObjString* concatString(ObjString* left, ObjString* right); // TODO: Change place

   private:
    Obj* objects_ = nullptr;

    static constexpr int FRAMES_MAX = 64;
    std::array<CallFrame, FRAMES_MAX> frames_;
    int frameCount_ = 0;

    static constexpr int STACK_MAX = 256;
    std::array<Value, STACK_MAX> stack_;
    int stackTop_ = 0;

    StringTable strings;
    Map<StringKey, Value> globals;

    std::ostream& out_;
  };
} // namespace lox
