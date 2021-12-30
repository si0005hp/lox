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
  OBJ_TYPE_APIS(Class)
  OBJ_TYPE_APIS(Instance)
  OBJ_TYPE_APIS(BoundMethod)

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

  void ObjClass::gcBlacken(VM& vm) const {
    vm.gcMarkObject(name_);
    for (int i = 0; i < methods_.capacity(); ++i) {
      Map<StringKey, Method>::Entry* e = methods_.getEntry(i);
      if (e->isEmpty()) continue;

      vm.gcMarkObject(e->key.value());
      vm.gcMarkObject(e->value.asClosure()); // TODO: Fix according to other method type
    }
  }

  void ObjInstance::gcBlacken(VM& vm) const {
    vm.gcMarkObject(klass_);
    for (int i = 0; i < fields_.capacity(); ++i) {
      Map<StringKey, Value>::Entry* e = fields_.getEntry(i);
      if (e->isEmpty()) continue;

      vm.gcMarkObject(e->key.value());
      vm.gcMarkValue(e->value);
    }
  }

  void ObjBoundMethod::gcBlacken(VM& vm) const {
    vm.gcMarkValue(receiver_);
    vm.gcMarkObject(method_.asClosure()); // TODO: Fix according to other method type
  }

} // namespace lox
