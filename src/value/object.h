#pragma once

#include <cstring>

#include "../chunk.h"
#include "../common.h"
#include "../memory.h"
#include "../utils.h"
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
      return length_ == strlen(cStr) && stringEquals(value_, cStr, length_);
    }

    uint32_t hash() const {
      return hash_;
    };

    int length() const {
      return length_;
    };

    const char* value() const {
      return value_;
    }

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
        : isNull_(true)
        , hash_(0) {}

      HashMapKey(ObjString* s)
        : HashMapKey(s->hash()) {}

      HashMapKey(uint32_t hash)
        : isNull_(false)
        , hash_(hash) {}

      bool operator==(const HashMapKey& other) const {
        if (isNull_) return other.isNull_;

        ASSERT(hash_ != 0, "Hash must be non-null.");
        return hash_ == other.hash_;
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
    static ObjString* allocate(const char* src, int length) {
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

  enum FunctionType {
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT
  };

  class ObjFunction : public Obj {
    friend class VM;

   public:
    virtual void trace(std::ostream& os) const {
      if (name_)
        os << "<fn " << name_ << ">";
      else
        os << "<script>";
    }

    FunctionType type() const {
      return type_;
    }

    int arity() const {
      return arity_;
    }

    int upvalueCount() const {
      return upvalueCount_;
    }

    Chunk& chunk() {
      return chunk_;
    }

    ObjString* name() const {
      return name_;
    }

   private:
    static ObjFunction* allocate(FunctionType type, int arity, ObjString* name) {
      return new ObjFunction(type, arity, name);
    }

    ObjFunction(FunctionType type, int arity, ObjString* name)
      : type_(type)
      , arity_(arity)
      , name_(name) {}

   private:
    FunctionType type_;
    int arity_;
    int upvalueCount_ = 0;
    Chunk chunk_;
    ObjString* name_; // Name can be null for script instance, otherwise it is function's name;
  };

} // namespace lox
