#pragma once

#include <stdlib.h>
#include <unistd.h>

namespace lox {

  class Memory {
  public:
    template <typename T>
    static T* allocate(size_t size) {
      return static_cast<T*>(allocate(size));
    }

    template <typename T>
    static T* reallocate(void* p, size_t oldSize, size_t newSize) {
      return static_cast<T*>(reallocate(p, oldSize, newSize));
    }

    static void* allocate(size_t size) {
      return reallocate(NULL, 0, size);
    }

    static void* deallocate(void* p) {
      return reallocate(p, 0, 0);
    }

    static void* reallocate(void* p, size_t oldSize, size_t newSize) {
      totalBytesAllocated_ += newSize - oldSize;

      if (newSize == 0) {
        free(p);
        return NULL;
      }
      return realloc(p, newSize);
    }

  private:
    inline static size_t totalBytesAllocated_ = 0;
  };

}; // namespace lox
