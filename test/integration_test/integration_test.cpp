#include "../test_common.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lox.h"

using namespace lox;

class IntegrationTest : public TestBase {
 public:
  std::string testPath(const std::string &fileName) {
    std::string path = "test/integration_test/resources/";
    path.append(fileName).append(".lox");
    return path;
  }

  void runTest(const std::string &expected, const std::string &fileName) {
    std::ostringstream testOut;
    Lox::runFile(testPath(fileName).c_str(), testOut);
    ASSERT_EQ(expected, testOut.str());
  }
};

TEST_F(IntegrationTest, expression) {
  runTest("true\n", "expression");
}

TEST_F(IntegrationTest, var) {
  runTest("123\n687\n", "var");
}

TEST_F(IntegrationTest, local_scope) {
  runTest("201\n", "local_scope");
}

TEST_F(IntegrationTest, while_) {
  runTest("0\n1\n2\n3\n4\n", "while");
}

TEST_F(IntegrationTest, while2) {
  runTest("20\n", "while2");
}

TEST_F(IntegrationTest, if_) {
  runTest("negative\n", "if");
}

TEST_F(IntegrationTest, concat) {
  runTest("hogefuga\n", "concat");
}
