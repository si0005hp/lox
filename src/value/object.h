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
      uint32_t hash = 2166136261u;

      for (int i = 0; i < length_; i++) {
        hash ^= value_[i];
        hash *= 16777619;
      }
      hash_ = hash;
    }

   private:
    int length_;
    uint32_t hash_;
    char value_[FLEXIBLE_ARRAY];
  };

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
