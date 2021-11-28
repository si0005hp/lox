#include "value.h"

namespace lox {

  /* Value */
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

  bool Value::isObj() const {
    return (ptr_ & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT);
  }

  // TODO
  const char* Value::toCString() const {
    if (isNumber()) return asNumber().toCString();
    if (isBool()) return asBool().toCString();
    if (isNil()) return Nil().toCString();
    return "";
  }

  // TODO: operator?
  bool Value::isFalsey() const {
    return isNil() || (isBool() && !asBool().value());
  }

  // TODO
  bool Value::operator==(Value other) const {
    if (isNumber() && other.isNumber()) return asNumber() == other.asNumber();
    if (isBool() && other.isBool()) return asBool() == other.asBool();
    if (isNil() && other.isNil()) return true;
    return false;
  }

} // namespace lox
