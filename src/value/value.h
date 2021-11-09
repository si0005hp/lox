#pragma once

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

  class Number;
  class Bool;

  class Value {
   public:
    Value(uint64_t ptr);

    bool isNumber() const;
    Number asNumber() const;

    bool isBool() const;
    Bool asBool() const;

    bool isNil() const;

    uint64_t ptr() const {
      return ptr_;
    }

   private:
    uint64_t ptr_;
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

   private:
    bool value_;
  };

  class Nil {};
} // namespace lox
