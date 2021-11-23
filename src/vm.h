#pragma once

#include "common.h"
#include "lib/vector.h"
#include "value/object.h"
#include "value/value.h"

namespace lox {

  enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
  };

  class VM {
   public:
    ~VM();

    InterpretResult interpret(const char* source);

    template <typename T, typename... Args>
    T* allocateObj(Args&&... args) {
      T* obj = new T(std::forward<Args>(args)...);

      // Append to objects list
      obj->next = objects_;
      objects_ = obj;
      return obj;
    }

   private:
    void freeObject();

   private:
    Obj* objects_;
  };
} // namespace lox
