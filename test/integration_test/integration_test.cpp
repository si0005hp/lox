#include "../test_common.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lox.h"

using namespace lox;

#define TO_STR(x) #x
#define INTEGRATION_TEST(expected, testName)     \
  TEST_F(IntegrationTest, testName) {            \
    runTest(TO_STR(expected), TO_STR(testName)); \
  }

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

INTEGRATION_TEST(true\n, expression)
INTEGRATION_TEST(123\n687\n, var)
INTEGRATION_TEST(201\n, local_scope)
INTEGRATION_TEST(0\n1\n2\n3\n4\n, while)
INTEGRATION_TEST(20\n, while2)
INTEGRATION_TEST(negative\n, if)
INTEGRATION_TEST(hogefuga\n, concat)
INTEGRATION_TEST(<fn myFunc>\n, function)
