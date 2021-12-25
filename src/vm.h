#pragma once

#include <type_traits>

#include "common.h"
#include "compiler.h"
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

    CallFrame(ObjClosure* closure, int stackStart)
      : closure(closure)
      , stackStart(stackStart) {}

    int ip = 0;
    ObjClosure* closure = nullptr;
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

    // GC procedures
    void gcMarkRoots();
    void markValue(Value value);
    void markObject(Obj* obj);

   private:
    ObjFunction* compileSource(const char* source);

    void freeObjects();
    void freeObject(Obj* obj);

    void appendObj(Obj* obj);

    ObjString* findOrAllocateString(const char* src, int length);

    InterpretResult run();

    void traceStack();

    void runtimeError(const char* format, ...) const;

    void appendCallFrame(ObjClosure* closure, int stackStart);

    bool callValue(Value callee, int argCount);
    bool call(ObjClosure* closure, int argCount);

    ObjUpvalue* captureUpvalue(Value* location);
    void closeUpvalues(Value* last);

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
      return currentFrame().closure->fn();
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

    StringTable strings_;
    Map<StringKey, Value> globals_;

    ObjUpvalue* openUpvalues_ = nullptr;

    std::ostream& out_;

    // Pointer to the Compiler that is currently compiling.
    Compiler* compiler_ = nullptr;
  };
} // namespace lox
