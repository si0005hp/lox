#pragma once

namespace lox {

  class ObjClosure;

  class Method {
   public:
    enum Type { METHOD_CLOSURE, };

    Method(ObjClosure* closure)
      : type_(METHOD_CLOSURE) {
      as_.closure = closure; // TODO
    }

    ObjClosure* asClosure() const {
      return as_.closure;
    }

   private:
    Type type_;

    union {
      ObjClosure* closure;
    } as_;
  };

} // namespace lox
