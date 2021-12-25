#include "object.h"

#include "../vm.h"

namespace lox {

#define OBJ_TYPE_APIS(subtype)                    \
  bool Obj::is##subtype() const {                 \
    return typeid(*this) == typeid(Obj##subtype); \
  }                                               \
                                                  \
  Obj##subtype* Obj::as##subtype() {              \
    return static_cast<Obj##subtype*>(this);      \
  }

  OBJ_TYPE_APIS(String)
  OBJ_TYPE_APIS(Function)
  OBJ_TYPE_APIS(Closure)

#undef OBJ_TYPE_APIS

  void ObjFunction::gcBlacken(VM& vm) const {
    vm.gcMarkObject(name_);
    for (int i = 0; i < chunk_.constants().size(); i++) vm.gcMarkValue(chunk_.getConstant(i));
  }

  void ObjUpvalue::gcBlacken(VM& vm) const {
    vm.gcMarkValue(closed_);
  }

  void ObjClosure::gcBlacken(VM& vm) const {
    vm.gcMarkObject(fn_);
    for (int i = 0; i < fn_->upvalueCount(); i++) vm.gcMarkObject(upvalues_[i]);
  }

} // namespace lox
