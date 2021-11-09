#include "value/value.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "test_common.h"

using namespace lox;

class ValueTest : public TestBase {};

TEST_F(ValueTest, Number_) {
  Number one(1);
  Value v = one.asValue();

  ASSERT_EQ(1, v.asNumber().value());
}

TEST_F(ValueTest, Bool_) {
  Bool t(true);
  Bool f(false);
  Value v1 = t.asValue();
  Value v2 = f.asValue();

  ASSERT_TRUE(v1.asBool().value());
  ASSERT_FALSE(v2.asBool().value());
}
