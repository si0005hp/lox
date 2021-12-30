#pragma once

#include <cstring>

#include "../chunk.h"
#include "../common.h"
#include "../lib/map.h"
#include "../memory.h"
#include "../utils.h"
#include "method.h"
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

    virtual void trace(std::ostream& os) const = 0;

    virtual bool eq(Obj* other) const {
      // Default identity logic.
      return this == other;
    }

    bool isGCMarked() const {
      return isGCMarked_;
    }

#define OBJ_TYPE_APIS(subtype) \
  bool is##subtype() const;    \
  Obj##subtype* as##subtype();

    OBJ_TYPE_APIS(String)
    OBJ_TYPE_APIS(Function)
    OBJ_TYPE_APIS(Closure)
    OBJ_TYPE_APIS(Class)
    OBJ_TYPE_APIS(Instance)
    OBJ_TYPE_APIS(BoundMethod)

#undef OBJ_TYPE_APIS

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
        , value_(s) {}

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

      ObjString* value() const {
        return value_;
      }

     private:
      bool isNull_ = true;
      uint32_t hash_ = 0;
      ObjString* value_ = nullptr;
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
        os << "<fn " << *name_ << ">";
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
      os << *fn_;
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

  typedef Map<StringKey, Method> MethodTable;

  class ObjClass : public Obj {
    friend class VM;

   public:
    virtual void trace(std::ostream& os) const {
      os << "class " << *name_;
    }

    ObjString* name() const {
      return name_;
    }

    MethodTable& methods() {
      return methods_;
    }

   private:
    static ObjClass* allocate(ObjString* name) {
      return new ObjClass(name);
    }

    ObjClass(ObjString* name)
      : name_(name) {}

    void gcBlacken(VM& vm) const;

   private:
    ObjString* name_;
    MethodTable methods_;
  };

  typedef Map<StringKey, Value> FieldTable;

  class ObjInstance : public Obj {
    friend class VM;

   public:
    virtual void trace(std::ostream& os) const {
      os << *klass_->name() << " instance";
    }

    ObjClass* klass() const {
      return klass_;
    }

    FieldTable& fields() {
      return fields_;
    }

   private:
    static ObjInstance* allocate(ObjClass* klass) {
      return new ObjInstance(klass);
    }

    ObjInstance(ObjClass* klass)
      : klass_(klass) {}

    void gcBlacken(VM& vm) const;

   private:
    ObjClass* klass_;
    FieldTable fields_;
  };

  class ObjBoundMethod : public Obj {
    friend class VM;

   public:
    virtual void trace(std::ostream& os) const {}

   private:
    static ObjBoundMethod* allocate(Value receiver, Method method) {
      return new ObjBoundMethod(receiver, method);
    }

    ObjBoundMethod(Value receiver, Method method)
      : receiver_(receiver)
      , method_(method) {}

    void gcBlacken(VM& vm) const;

   private:
    Value receiver_;
    Method method_;
  };

} // namespace lox
