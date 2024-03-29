#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "test_common.h"
#include "value/object.h"
#include "vm.h"

using namespace lox;

class StringTableTest : public TestBase {
 public:
  VM vm_;
};

TEST_F(StringTableTest, String_) {
  StringTable table;
  ObjString* emptyString = vm_.allocateObj<ObjString>("", 0);
  ObjString* foo = vm_.allocateObj<ObjString>("foo", 3);

  table.add(emptyString);
  table.add(foo);

  ASSERT_EQ(emptyString, table.find("", 0));
  ASSERT_EQ(foo, table.find("foo", 3));
  ASSERT_EQ(nullptr, table.find("hoge", 4));
}
