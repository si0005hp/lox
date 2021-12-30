#pragma once

#include <iostream>

namespace lox {

  class ObjClosure;

  class Method {
   public:
    enum Type { METHOD_CLOSURE, };

    Method() {} // TODO: For Map

    Method(ObjClosure* closure)
      : type_(METHOD_CLOSURE) {
      as_.closure = closure; // TODO
    }

    ObjClosure* asClosure() const {
      return as_.closure;
    }

    void trace(std::ostream& os) const;

   private:
    Type type_;

    union {
      ObjClosure* closure;
    } as_;
  };

} // namespace lox
