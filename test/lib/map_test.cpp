#include "lib/map.h"

#include "../test_common.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lib/vector.h"
#include "value/object.h"
#include "vm.h"

using namespace lox;

class MapTest : public TestBase {};

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

TEST_F(MapTest, containsKey) {
  Map<IntKey, int> map;
  map.put(1, 100);

  ASSERT_TRUE(map.containsKey(1));
  ASSERT_FALSE(map.containsKey(2));
}

TEST_F(MapTest, str) {
  VM vm;
  Map<StringKey, int> map;

  ObjString* hoge = vm.allocateObj<ObjString>("hoge", 4);
  map.put(hoge, 100);

  ObjString* foo = vm.allocateObj<ObjString>("foo", 3);
  map.put(foo, 200);

  ObjString* key = vm.allocateObj<ObjString>("hoge", 4);
  int value;
  ASSERT_TRUE(map.get(key, &value));
  ASSERT_EQ(value, 100);
}

TEST_F(MapTest, entries) {
  Map<IntKey, int> map;
  map.put(1, 100);
  map.put(2, 200);

  int entriesCount = 0;
  for (int i = 0; i < map.capacity(); ++i) {
    Map<IntKey, int>::Entry* e = map.getEntry(i);
    if (e->isEmpty()) continue;

    entriesCount++;
    if (e->key == 1) {
      ASSERT_EQ(100, e->value);
    } else if (e->key == 2) {
      ASSERT_EQ(200, e->value);
    } else {
      FAIL();
    }
  }
  ASSERT_EQ(2, entriesCount);
}

TEST_F(MapTest, putAll) {
  Map<IntKey, int> src;
  src.put(1, 100);
  src.put(2, 200);

  Map<IntKey, int> dst;
  dst.putAll(src);

  int v1, v2;
  ASSERT_TRUE(dst.get(1, &v1));
  ASSERT_EQ(v1, 100);
  ASSERT_TRUE(dst.get(2, &v2));
  ASSERT_EQ(v2, 200);
}
