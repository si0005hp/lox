#include "value.h"

namespace lox {

  /* Value */
  Value::Value(uint64_t ptr)
    : ptr_(ptr) {}

  bool Value::isNumber() const {
    return (ptr_ & QNAN) != QNAN;
  }

  Number Value::asNumber() const {
    return Number(*this);
  }

  bool Value::isBool() const {
    return ptr_ == TRUE_VAL || ptr_ == FALSE_VAL;
  }

  Bool Value::asBool() const {
    return Bool(*this);
  }

  bool Value::isNil() const {
    return ptr_ == NIL_VAL;
  }

} // namespace lox
