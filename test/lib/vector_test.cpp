#include "lib/vector.h"

#include "../test_common.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lox;

class VectorTest : public TestBase {};

TEST_F(VectorTest, construct) {
  Vector<int> v;
  ASSERT_EQ(0, v.size());

  Vector<int> v2(3);
  ASSERT_EQ(0, v2.size());
}

TEST_F(VectorTest, construct_multiple_args) {
  Vector<int> v{1, 2, 3};
  ASSERT_EQ(1, v[0]);
  ASSERT_EQ(2, v[1]);
  ASSERT_EQ(3, v[2]);
}

TEST_F(VectorTest, copy_construct) {
  Vector<int> v{1, 2, 3};
  Vector<int> v2(v);

  ASSERT_EQ(1, v2[0]);
  ASSERT_EQ(2, v2[1]);
  ASSERT_EQ(3, v2[2]);
}

TEST_F(VectorTest, operator_subscript) {
  Vector<int> v;
  for (int i = 1; i <= 3; i++) {
    v.push(i);
  }

  ASSERT_EQ(1, v[0]);
  ASSERT_EQ(2, v[1]);
  ASSERT_EQ(3, v[2]);
  ASSERT_EQ(3, v[-1]);
  ASSERT_EQ(2, v[-2]);
  ASSERT_EQ(1, v[-3]);
}

TEST_F(VectorTest, removeAt) {
  Vector<int> v;
  for (int i = 1; i <= 3; i++) {
    v.push(i);
  }

  v.removeAt(1);
  ASSERT_EQ(2, v.size());
  ASSERT_EQ(1, v[0]);
  ASSERT_EQ(3, v[1]);

  v.removeAt(-1);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(1, v[0]);
}
