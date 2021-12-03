#pragma once

#include "common.h"
#include "lib/map.h"
#include "value/object.h"

namespace lox {

  class StringTable {
   public:
    void add(ObjString* s) {
      map_.put(s, s);
    }

    ObjString* find(const char* s, int length) const {
      uint32_t hash = ObjString::calcHash(s, length);

      ObjString* result;
      if (map_.get(hash, &result)) return result;

      return nullptr;
    }

   private:
    Map<StringKey, ObjString*> map_;
  };

} // namespace lox
