#pragma once

#include "../chunk.h"
#include "../common.h"
#include "../memory.h"
#include "value.h"

namespace lox {

  class ObjString;

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

    virtual void trace(std::ostream& os) const = 0;

    bool isString() const;
    ObjString* asString();

    virtual bool eq(Obj* other) const {
      // Default identity logic.
      return this == other;
    }

   private:
    Obj* next_;
  };

  class ObjString : public Obj {
    friend class VM;

   public:
    virtual void trace(std::ostream& os) const {
      os << value_;
    }

    virtual bool eq(Obj* other) const {
      if (!other->isString()) return false;

      // TODO: valid?
      return hash_ == other->asString()->hash_;
    }

    bool eqCString(const char* cStr) const {
      return length_ == strlen(cStr) && std::memcmp(value_, cStr, length_) == 0;
    }

    uint32_t hash() const {
      return hash_;
    };

    int length() const {
      return length_;
    };

    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
    static uint32_t calcHash(const char* chars, int length) {
      uint32_t hash = 2166136261u;

      for (int i = 0; i < length; i++) {
        hash ^= chars[i];
        hash *= 16777619;
      }
      return hash;
    }

    class HashMapKey {
     public:
      HashMapKey()
        : isNull_(true) {}

      HashMapKey(ObjString* s)
        : HashMapKey(s->hash()) {}

      HashMapKey(uint32_t hash)
        : isNull_(false)
        , hash_(hash) {}

      bool operator==(const HashMapKey& other) const {
        return isNull_ ? other.isNull_ : hash_ == other.hash_;
      };

      int hashCode() const {
        ASSERT(!isNull_, "Hash must be non-null.");

        return hash_;
      }

     private:
      bool isNull_;
      uint32_t hash_;
    };

   private:
    static ObjString* allocate(const char* src, int length = -1) {
      if (length == -1) length = strlen(src);

      void* mem = Memory::allocate(sizeof(ObjString) + sizeof(char) * length);
      return ::new (mem) ObjString(src, length);
    }

    ObjString(const char* src, int length)
      : hash_(calcHash(src, length))
      , length_(length) {
      // Set value (TODO: Comparison with strncpy)
      std::memcpy(value_, src, length_);
      value_[length_] = '\0'; // Terminate string
    }

    static int strlen(const char* chars) {
      return static_cast<int>(std::strlen(chars));
    }

   public:
    uint32_t hash_;
    int length_;
    char value_[FLEXIBLE_ARRAY];
  };

  typedef ObjString::HashMapKey StringKey;

  class ObjFunction : public Obj {
    friend class VM;

   public:
    virtual void trace(std::ostream& os) const {
      if (name_ == nullptr)
        os << "<script>";
      else
        os << "<fn " << name_ << ">";
    }

    Chunk& chunk() {
      return chunk_;
    }

   private:
    static ObjFunction* allocate(int arity, ObjString* name) {
      return new ObjFunction(arity, name);
    }

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
