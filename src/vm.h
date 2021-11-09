#pragma once

#include "common.h"
#include "lib/vector.h"
#include "value/object.h"
#include "value/value.h"

namespace lox {
  class VM {
   public:
    template <typename T, typename... Args>
    T* allocateObj(Args&&... args);

   private:
    Obj* objects_;
  };
} // namespace lox
