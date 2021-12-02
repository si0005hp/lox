#include "lib/map.h"

#include "../test_common.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lib/vector.h"
#include "value/object.h"
#include "vm.h"

using namespace lox;

class MapTest : public TestBase {};

class StrKey {
 public:
  StrKey()
    : StrKey(nullptr) {}

  StrKey(ObjString* value) // Implicit constructor
    : value_(value) {}

  bool operator==(const StrKey& other) const {
    return value_ == nullptr ? other.value_ == nullptr : value_->eq(other.value_);
  };

  int hashCode() const {
    return value_->hash();
  }

 private:
  ObjString* value_;
};

class IntKey {
 public:
  IntKey()
    : IntKey(-1) {}

  IntKey(int value) // Implicit constructor
    : value_(value) {}

  bool operator==(const IntKey& other) const {
    return value_ == other.value_;
  };

  int hashCode() const {
    return value_;
  }

 private:
  int value_;
};

TEST_F(MapTest, put_get) {
  Map<IntKey, int> map;
  map.put(1, 100);
  map.put(2, 200);

  int value;
  ASSERT_FALSE(map.get(0, &value));
  ASSERT_TRUE(map.get(1, &value));
  ASSERT_EQ(value, 100);
  ASSERT_TRUE(map.get(2, &value));
  ASSERT_EQ(value, 200);

  // overwrite
  map.put(2, 300);
  ASSERT_TRUE(map.get(2, &value));
  ASSERT_EQ(value, 300);
}

TEST_F(MapTest, remove) {
  Map<IntKey, int> map;
  map.put(1, 100);

  int value;
  ASSERT_TRUE(map.get(1, &value));
  ASSERT_EQ(value, 100);

  map.remove(1);
  ASSERT_FALSE(map.get(1, &value));
}

TEST_F(MapTest, str) {
  VM vm;
  Map<StrKey, int> map;

  ObjString* hoge = vm.allocateObjFlex<ObjString>("hoge", 4);
  map.put(hoge, 100);

  ObjString* foo = vm.allocateObjFlex<ObjString>("foo", 3);
  map.put(foo, 200);

  ObjString* key = vm.allocateObjFlex<ObjString>("hoge", 4);
  int value;
  ASSERT_TRUE(map.get(key, &value));
  ASSERT_EQ(value, 100);
}
