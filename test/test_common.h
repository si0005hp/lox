#pragma once

#include <cstring>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

inline bool stringEquals(const char* a, const char* b) {
  return std::strcmp(a, b) == 0 ? true : false;
}

class TestBase : public ::testing::Test {};
