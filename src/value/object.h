#pragma once

#include "../chunk.h"
#include "../common.h"
#include "../memory.h"
#include "value.h"

namespace lox {
  class Obj {
    friend class VM;

   public:
    virtual ~Obj() {}

    void* operator new(size_t s) {
      return Memory::allocate(s);
    }

    Value asValue() const {
      return (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(this));
    }

   private:
    Obj* next_;
  };

  class ObjString : public Obj {
    friend class VM;

   private:
    static ObjString* newFlex(const char* src, int length = -1) {
      if (length == -1) length = static_cast<int>(std::strlen(src));

      void* mem = Memory::allocate(sizeof(ObjString) + sizeof(char) * length);
      return ::new (mem) ObjString(src, length);
    }

    ObjString(const char* src, int length)
      : length_(length) {
      setValue(src);
      setHash();
    }

    void setValue(const char* src) {
      std::strncpy(value_, src, length_);
      value_[length_] = '\0'; // Terminate string
    }

    void setHash() {
      // TODO: implement
    }

   private:
    int length_;
    uint32_t hash_;
    char value_[FLEXIBLE_ARRAY];
  };

  class ObjFunction : public Obj {
    friend class VM;

   public:
    Chunk& chunk() {
      return chunk_;
    }

   private:
    ObjFunction(int arity, ObjString* name)
      : arity_(arity)
      , name_(name) {}

   private:
    int arity_;
    int upvalueCount_ = 0;
    Chunk chunk_;
    ObjString* name_;
  };

} // namespace lox
