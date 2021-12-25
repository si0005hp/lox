#pragma once

#include <cstring>

#include "../chunk.h"
#include "../common.h"
#include "../memory.h"
#include "../utils.h"
#include "value.h"

namespace lox {

  class ObjString;
  class ObjFunction;
  class ObjClosure;

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

    // TODO: lame
    bool isString() const;
    ObjString* asString();

    bool isFunction() const;
    ObjFunction* asFunction();

    bool isClosure() const;
    ObjClosure* asClosure();

    virtual bool eq(Obj* other) const {
      // Default identity logic.
      return this == other;
    }

   private:
    virtual void gcBlacken(VM& vm) const {
      // TODO: Fix leave this as default behavior, or change to pure virtual function?
    }

   private:
    bool isGCMarked_ = false;
    Obj* next_ = nullptr;
  };

  inline std::ostream& operator<<(std::ostream& os, const Obj& obj) {
    obj.trace(os);
    return os;
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
      HashMapKey() {}

      HashMapKey(ObjString* s)
        : isNull_(false)
        , hash_(s->hash_)
        , s_(s) {}

      // TODO
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
      bool isNull_ = true;
      uint32_t hash_ = 0;
      ObjString* s_ = nullptr;
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
        os << "<fn " << name_->value() << ">";
      else
        os << "<script>";
    }

    int getAndIncrementUpvalue() {
      return upvalueCount_++;
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

    void gcBlacken(VM& vm) const;

   private:
    FunctionType type_;
    int arity_;
    int upvalueCount_ = 0;
    Chunk chunk_;
    ObjString* name_; // Name can be null for script instance, otherwise it is function's name;
  };

  class ObjUpvalue : public Obj {
    friend class VM;

   public:
    virtual void trace(std::ostream& os) const {
      os << "upvalue"; // TODO
    }

    void doClose() {
      closed_ = *location_;
      location_ = &closed_;
    }

    Value* location() const {
      return location_;
    }

    Value closed() const {
      return closed_;
    }

    ObjUpvalue* next() const {
      return next_;
    }

   private:
    static ObjUpvalue* allocate(Value* location) {
      return new ObjUpvalue(location);
    }

    ObjUpvalue(Value* location)
      : location_(location) {}

    void gcBlacken(VM& vm) const;

   private:
    Value* location_;
    Value closed_ = Nil().asValue();
    ObjUpvalue* next_ = nullptr;
  };

  class ObjClosure : public Obj {
    friend class VM;

   public:
    virtual void trace(std::ostream& os) const {
      fn_->trace(os);
    }

    ObjFunction* fn() const {
      return fn_;
    }

    ObjUpvalue** upvalues() {
      return upvalues_;
    }

   private:
    static ObjClosure* allocate(ObjFunction* fn) {
      void* mem = Memory::allocate(sizeof(ObjClosure) + sizeof(ObjUpvalue) * fn->upvalueCount());
      return ::new (mem) ObjClosure(fn);
    }

    ObjClosure(ObjFunction* fn)
      : fn_(fn) {
      for (int i = 0; i < fn->upvalueCount(); i++) upvalues_[i] = nullptr;
    }

    void gcBlacken(VM& vm) const;

   private:
    ObjFunction* fn_;
    ObjUpvalue* upvalues_[FLEXIBLE_ARRAY];
  };

} // namespace lox
