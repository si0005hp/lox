#pragma once

#include "../chunk.h"
#include "../common.h"
#include "../memory.h"

namespace lox {
  struct Obj {
    virtual ~Obj() {}

    void* operator new(size_t s) {
      return Memory::allocate(s);
    }

    Obj* next;
  };

  struct ObjString : public Obj {
    int length;
    char* chars;
    uint32_t hash;
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
