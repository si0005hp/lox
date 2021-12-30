#include "method.h"

#include "object.h"

namespace lox {

  void Method::trace(std::ostream& os) const {
    os << *as_.closure; // TODO
  }

} // namespace lox
