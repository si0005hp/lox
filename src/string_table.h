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

    void removeUnmarkedStrings() {
      // Interned strings
      for (int i = 0; i < map_.capacity(); ++i) {
        Map<StringKey, ObjString*>::Entry* e = map_.getEntry(i);
        if (e->isEmpty()) continue;

        if (!e->value->isGCMarked()) map_.remove(e->key);
      }
    }

   private:
    Map<StringKey, ObjString*> map_;
  };

} // namespace lox
