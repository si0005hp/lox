#pragma once

#include "../chunk.h"
#include "../common.h"
#include "../memory.h"
#include "value.h"

namespace lox {
  struct Obj {
    virtual ~Obj() {}

    void* operator new(size_t s) {
      return Memory::allocate(s);
    }

    Value asValue() const {
      return (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(this));
    }

    Obj* next;
  };

  struct ObjString : public Obj {
    static ObjString* newFlex(const char* src, int length = -1) {
      if (length == -1) length = static_cast<int>(std::strlen(src));

      void* mem = Memory::allocate(sizeof(ObjString) + sizeof(char) * length);
      return ::new (mem) ObjString(src, length);
    }

    ObjString(const char* src, int length)
      : length(length) {
      setValue(src);
      setHash();
    }

    void setValue(const char* src) {
      std::strncpy(value, src, length);
      value[length] = '\0'; // Terminate string
    }

    void setHash() {
      // TODO: implement
    }

    int length;
    uint32_t hash;
    char value[FLEXIBLE_ARRAY];
  };

  struct ObjFunction : public Obj {
    ObjFunction(int arity, ObjString* name)
      : arity(arity)
      , upvalueCount(0)
      , name(name) {}

    int arity;
    int upvalueCount;
    Chunk chunk;
    ObjString* name;
  };

} // namespace lox
