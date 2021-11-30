#include "value/value.h"

#include <sstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "test_common.h"

using namespace lox;

std::string valueToString(Value value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

#define VALUE_TO_STRING(value) valueToString(value.asValue())

class ValueTest : public TestBase {};

TEST_F(ValueTest, Value_trace) {
  ASSERT_EQ("23", VALUE_TO_STRING(Number(23)));
  ASSERT_EQ("true", VALUE_TO_STRING(Bool(true)));
  ASSERT_EQ("nil", VALUE_TO_STRING(Nil()));
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
