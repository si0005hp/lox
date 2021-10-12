#include "lexer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lib/vector.h"
#include "test_common.h"

using namespace lox;

#define ASSERT_TOKEN(_type, _text, _length, _line) \
  ASSERT_EQ(_type, token->type);                   \
  ASSERT_TRUE(stringEquals(_text, token->start));  \
  ASSERT_EQ(_length, token->length);               \
  ASSERT_EQ(_line, token->line);

class LexerTest : public TestBase {
public:
  void readTokenTest(TokenType type, const char* start, int length, int line, const char* source) {
    Lexer lexer(source);
    Token* token = lexer.readToken();
    ASSERT_TOKEN(type, start, length, line);
  }
};

TEST_F(LexerTest, empty) {
  readTokenTest(TOKEN_EOF, "", 0, 1, "");
}

TEST_F(LexerTest, whitespace) {
  readTokenTest(TOKEN_EOF, "", 0, 1, " \r\t");
  readTokenTest(TOKEN_EOF, "", 0, 2, "\n");
  readTokenTest(TOKEN_EOF, "", 0, 1, "// this is comment");
}

TEST_F(LexerTest, number) {
  readTokenTest(TOKEN_NUMBER, "123", 3, 1, "123");
  readTokenTest(TOKEN_NUMBER, "40.25", 5, 1, "40.25");
}

TEST_F(LexerTest, keyword) {
  readTokenTest(TOKEN_AND, "and", 3, 1, "and");
  readTokenTest(TOKEN_WHILE, "while", 5, 1, "while");
}

TEST_F(LexerTest, identifier) {
  readTokenTest(TOKEN_IDENTIFIER, "foo", 3, 1, "foo");
  readTokenTest(TOKEN_IDENTIFIER, "Hoge", 4, 1, "Hoge");
  readTokenTest(TOKEN_IDENTIFIER, "doSomething", 11, 1, "doSomething");
}

TEST_F(LexerTest, char_tokens) {
  readTokenTest(TOKEN_LEFT_PAREN, "(", 1, 1, "(");
  readTokenTest(TOKEN_BANG_EQUAL, "!=", 2, 1, "!=");
}

TEST_F(LexerTest, string) {
  readTokenTest(TOKEN_STRING, "\"foo\"", 5, 1, "\"foo\"");
}

TEST_F(LexerTest, destructor) {
  Vector<Token*> tokens;

  {
    Lexer lexer("{ 123; }");

    while (true) {
      Token* token = lexer.readToken();
      tokens.push(token);
      if (token->type == TOKEN_EOF) break;
    }
  }

  // TODO: How to check if the pointer is freed?
  // for (int i = 0; i < tokens.size(); i++)
  //   ASSERT_TRUE(tokens[i] == NULL);
}
