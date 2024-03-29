#include "value/object.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "test_common.h"
#include "vm.h"

using namespace lox;

class ObjectTest : public TestBase {
 public:
  void assertString(ObjString* actual, const char* expectedValue, int expectedLength,
                    uint32_t expectedHash) {
    ASSERT_TRUE(actual->eqCString(expectedValue));
    ASSERT_TRUE(stringEquals(actual->value(), expectedValue)); // TODO: Can be skipped
    ASSERT_EQ(actual->length(), expectedLength);
    ASSERT_EQ(actual->hash(), expectedHash);
  }

 public:
  VM vm_;
};

TEST_F(ObjectTest, String_) {
  ObjString* s = vm_.allocateObj<ObjString>("", 0);
  assertString(s, "", 0, 2166136261);

  s = vm_.allocateObj<ObjString>("foo", 3);
  assertString(s, "foo", 3, 2851307223);
}
