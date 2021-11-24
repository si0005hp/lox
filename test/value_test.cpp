#include "value/value.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "test_common.h"

using namespace lox;

class ValueTest : public TestBase {};

TEST_F(ValueTest, Value_toCString) {
  ASSERT_TRUE(stringEquals("23", Number(23).asValue().toCString(), 2));
  ASSERT_TRUE(stringEquals("true", Bool(true).asValue().toCString(), 4));
  ASSERT_TRUE(stringEquals("nil", Nil().asValue().toCString(), 3));
}

TEST_F(ValueTest, Value_NULL_ADDRESS) {
  Value uninitialized;
  ASSERT_FALSE(uninitialized.isNumber());
  ASSERT_FALSE(uninitialized.isBool());
  ASSERT_FALSE(uninitialized.isNil());
  ASSERT_FALSE(uninitialized.isObj());
}

TEST_F(ValueTest, Number_conv) {
  Number one(1);
  Value v = one.asValue();

  ASSERT_EQ(1, v.asNumber().value());
}

TEST_F(ValueTest, Number_toCString) {
  ASSERT_TRUE(stringEquals("2", Number(2).toCString(), 1));
}

TEST_F(ValueTest, Number_operator_unary_minus) {
  Number n(2);
  ASSERT_EQ(-2, -n.value());
}

TEST_F(ValueTest, Number_operator_binary_plus) {
  Number n(2);
  Number m(5);
  ASSERT_EQ(7, (n + m).value());
}

TEST_F(ValueTest, Bool_conv) {
  Bool t(true);
  Bool f(false);
  Value v1 = t.asValue();
  Value v2 = f.asValue();

  ASSERT_TRUE(v1.asBool().value());
  ASSERT_FALSE(v2.asBool().value());
}

TEST_F(ValueTest, Bool_toCString) {
  ASSERT_TRUE(stringEquals("true", Bool(true).toCString(), 4));
  ASSERT_TRUE(stringEquals("false", Bool(false).toCString(), 5));
}

TEST_F(ValueTest, Nil_toCString) {
  ASSERT_TRUE(stringEquals("nil", Nil().toCString(), 3));
}
