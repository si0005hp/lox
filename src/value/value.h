#pragma once

#include <cstring>
#include <iomanip>
#include <string>

#include "../common.h"

namespace lox {

#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define QNAN ((uint64_t)0x7ffc000000000000)

#define TAG_NIL 1   // 01.
#define TAG_FALSE 2 // 10.
#define TAG_TRUE 3  // 11.

#define FALSE_VAL ((uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL ((uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL ((uint64_t)(QNAN | TAG_NIL))

// TODO: valid?
#define TAG_UNINITIALIZED 0
#define UNINITIALIZED ((uint64_t)(QNAN | TAG_UNINITIALIZED))

  class Number;
  class Bool;
  class Obj;
  class ObjString;
  class ObjFunction;
  class ObjClosure;
  class ObjClass;
  class ObjInstance;

  // TODO: The whole Value abstraction could be better desined.
  class Value {
   public:
    Value()
      : ptr_(UNINITIALIZED){};

    Value(uint64_t ptr)
      : ptr_(ptr) {}

    bool isNumber() const;
    Number asNumber() const;

    bool isBool() const;
    Bool asBool() const;

    bool isNil() const;

    bool isObj() const;
    Obj* asObj() const;

    bool isString() const;
    ObjString* asString() const;

    bool isFunction() const;
    ObjFunction* asFunction() const;

    bool isClosure() const;
    ObjClosure* asClosure() const;

    bool isClass() const;
    ObjClass* asClass() const;

    bool isInstance() const;
    ObjInstance* asInstance() const;

    uint64_t ptr() const {
      return ptr_;
    }

    void trace(std::ostream& os) const;

    bool isFalsey() const;

    bool operator==(Value other) const;

   private:
    uint64_t ptr_;
  };

  inline std::ostream& operator<<(std::ostream& os, Value value) {
    value.trace(os);
    return os;
  };

  class Number {
   public:
    Number(double value)
      : value_(value) {}

    Number(Value value)
      : value_(castToNum(value.ptr())) {}

    Value asValue() const {
      return Value(castToPtr(value_));
    }

    double value() const {
      return value_;
    }

    void trace(std::ostream& os) const {
      os << std::noshowpoint << value_;
    }

   public:
    Number operator-() const {
      return Number(-value_);
    }

    Number operator+(Number other) const {
      return Number(value_ + other.value_);
    }

    Number operator-(Number other) const {
      return Number(value_ - other.value_);
    }

    Number operator*(Number other) const {
      return Number(value_ * other.value_);
    }

    Number operator/(Number other) const {
      return Number(value_ / other.value_);
    }

    bool operator==(Number other) const {
      return value_ == other.value_;
    }

    bool operator>(Number other) const {
      return value_ > other.value_;
    }

    bool operator<(Number other) const {
      return value_ < other.value_;
    }

   private:
    typedef union {
      uint64_t ptr;
      double num;
    } NumOrPtr;

    double castToNum(uint64_t ptr) const {
      NumOrPtr u;
      u.ptr = ptr;
      return u.num;
    }

    uint64_t castToPtr(double num) const {
      NumOrPtr u;
      u.num = num;
      return u.ptr;
    }

   private:
    double value_;
  };

  class Bool {
   public:
    // TODO: Singleton base
    Bool(bool value)
      : value_(value) {}

    Bool(Value value)
      : value_(value.ptr() == TRUE_VAL) {}

    Value asValue() const {
      return value_ ? TRUE_VAL : FALSE_VAL;
    }

    bool value() const {
      return value_;
    }

    void trace(std::ostream& os) const {
      os << (value_ ? "true" : "false");
    }

    bool operator==(Bool other) const {
      return value_ == other.value_;
    }

   private:
    bool value_;
  };

  class Nil {
   public:
    // TODO: Singleton base
    Value asValue() const {
      return NIL_VAL;
    }

    void trace(std::ostream& os) const {
      os << "nil";
    }
  };
} // namespace lox
