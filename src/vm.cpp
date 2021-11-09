#include "vm.h"

namespace lox {
  template <typename T, typename... Args>
  T* VM::allocateObj(Args&&... args) {
    T* obj = new T(std::forward<Args>(args)...);

    // Append to objects list
    obj->next = objects_;
    objects_ = obj;
    return obj;
  }
} // namespace lox
