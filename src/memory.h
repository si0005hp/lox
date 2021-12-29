#pragma once

#include <cstdlib>

namespace lox {

  class VM;

  class Memory {
   public:
    class DefaultReallocator {
     public:
      static void* reallocate(void* p, size_t oldSize, size_t newSize) {
        if (newSize == 0) {
          std::free(p);
          return nullptr;
        }
        return std::realloc(p, newSize);
      }
    };

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

    // TODO: Should we use operator new/delete instead of realloc/free?
    static void* reallocate(void* p, size_t oldSize, size_t newSize) {
#ifdef DEBUG_LOG_GC
      printf("reallocate %p %lu -> %lu\n", p, (unsigned long)oldSize, (unsigned long)newSize);
#endif
      totalBytesAllocated_ += newSize - oldSize;

      // TODO: GC cycle.
      if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
      }

      return DefaultReallocator::reallocate(p, oldSize, newSize);
    }

    static size_t totalBytesAllocated() {
      return totalBytesAllocated_;
    }

   private:
    static void collectGarbage();

   private:
    inline static size_t totalBytesAllocated_ = 0;
    inline static VM* vm_;
  };

}; // namespace lox
