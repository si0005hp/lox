#include "object.h"

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

#undef OBJ_TYPE_APIS

} // namespace lox
