#pragma once

// #include <stdlib.h>
#include <cstdlib>

// #include "vm.h"

namespace lox {

  // TODO
  class VM;

  class Memory {
   public:
    static void initialize(VM* vm) {
      vm_ = vm;
    }

    // https://github.com/v8/v8/blob/9.7.37/src/zone/zone.h#L107
    template <typename T>
    static T* allocate() {
      return allocate<T>(1);
    }
    template <typename T>
    static T* allocate(int count) {
      return static_cast<T*>(allocate(sizeof(T) * count));
    }

    template <typename T>
    static T* reallocate(void* p, size_t oldSize, size_t newSize) {
      return static_cast<T*>(reallocate(p, oldSize, newSize));
    }

    static void* allocate(size_t size) {
      return reallocate(nullptr, 0, size);
    }

    static void* deallocate(void* p) {
      return reallocate(p, 0, 0);
    }

    // TODO: temporal
    template <typename T>
    static void* deallocate(T* p) {
      p->~T();
      return reallocate(p, 0, 0);
    }

    // TODO: Should we use operator new/delete instead of realloc/free?
    static void* reallocate(void* p, size_t oldSize, size_t newSize) {
      totalBytesAllocated_ += newSize - oldSize;

      if (newSize == 0) {
        std::free(p);
        return nullptr;
      }
      return std::realloc(p, newSize);
    }

    static size_t totalBytesAllocated() {
      return totalBytesAllocated_;
    }

   private:
    inline static size_t totalBytesAllocated_ = 0;
    inline static VM* vm_;
  };

}; // namespace lox
