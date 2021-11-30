#include "value.h"

#include "object.h"

namespace lox {

#define OBJ_SUBTYPE_APIS(subtype)                          \
  bool Value::is##subtype() const {                        \
    return isObj() && typeid(*asObj()) == typeid(subtype); \
  }                                                        \
                                                           \
  subtype* Value::as##subtype() const {                    \
    return static_cast<subtype*>(asObj());                 \
  }

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

  Obj* Value::asObj() const {
    return ((Obj*)(uintptr_t)((ptr_) & ~(SIGN_BIT | QNAN)));
  }

  OBJ_SUBTYPE_APIS(ObjString)

  void Value::trace(std::ostream& os) const {
    if (isNumber()) {
      asNumber().trace(os);
    } else if (isBool()) {
      asBool().trace(os);
    } else if (isNil()) {
      Nil().trace(os);
    } else if (isObj()) {
      asObj()->trace(os);
    } else {
      UNREACHABLE();
    }
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
