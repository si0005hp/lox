#pragma once

#include <cstring>

namespace lox {

  inline bool stringEquals(const char* a, const char* b, int len) {
    return std::memcmp(a, b, len) == 0;
  }

} // namespace lox
