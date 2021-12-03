#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "test_common.h"
#include "value/object.h"
#include "vm.h"

using namespace lox;

class StringTableTest : public TestBase {
 public:
  void assertString(ObjString* actual, const char* expectedValue, int expectedLength,
                    uint32_t expectedHash) {
    ASSERT_TRUE(actual->eqCString(expectedValue));
    ASSERT_EQ(actual->length(), expectedLength);
    ASSERT_EQ(actual->hash(), expectedHash);
  }

 public:
  VM vm_;
};

TEST_F(StringTableTest, String_) {
  StringTable table;
  ObjString* emptyString = vm_.allocateObjFlex<ObjString>("");
  ObjString* foo = vm_.allocateObjFlex<ObjString>("foo");

  table.add(emptyString);
  table.add(foo);

  ASSERT_EQ(emptyString, table.find("", 0));
  ASSERT_EQ(foo, table.find("foo", 3));
  ASSERT_EQ(nullptr, table.find("hoge", 4));
}
