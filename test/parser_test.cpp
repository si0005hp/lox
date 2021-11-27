#include "parser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lib/vector.h"
#include "test_common.h"

using namespace lox;

class ParserTest : public TestBase {
 public:
  void TearDown() {
    if (lexer != NULL) delete lexer;
    if (parser != NULL) delete parser;
  }

  void setupParser(const char* source) {
    lexer = new Lexer(source);
    parser = new Parser(*lexer);
  }

  void setupParserAndRun(const char* source, bool expectedParseResult = true) {
    setupParser(source);
    ASSERT_EQ(parser->parse(), expectedParseResult);
  }

  void assertLiteral(const char* expectedValue, TokenType expectedType, Expr* expr) {
    Literal* lit = static_cast<Literal*>(expr);
    ASSERT_EQ(expectedType, lit->value->type);
    ASSERT_EQ(*expectedValue, *lit->value->start);
  }

 public:
  Lexer* lexer;
  Parser* parser;
};

TEST_F(ParserTest, expression_literal_number) {
  setupParser("123");
  assertLiteral("123", TOKEN_NUMBER, parser->expression());
}

TEST_F(ParserTest, expression_literal_bool) {
  setupParser("true");
  assertLiteral("true", TOKEN_TRUE, parser->expression());
}

TEST_F(ParserTest, expression_literal_nil) {
  setupParser("nil");
  assertLiteral("nil", TOKEN_NIL, parser->expression());
}

TEST_F(ParserTest, expression_literal_identifier) {
  setupParser("myvar");

  Variable* var = static_cast<Variable*>(parser->expression());
  ASSERT_TRUE(stringEquals("myvar", var->name->start, var->name->length));
}

TEST_F(ParserTest, expression_literal_string) {
  setupParser("\"someStr\"");
  assertLiteral("\"someStr\"", TOKEN_STRING, parser->expression());
}

TEST_F(ParserTest, expression_grouping) {
  setupParser("(123)");

  Grouping* grouping = static_cast<Grouping*>(parser->expression());
  assertLiteral("123", TOKEN_NUMBER, grouping->expression);
}

TEST_F(ParserTest, expression_super) {
  setupParser("super.foo");

  Super* super = static_cast<Super*>(parser->expression());
  ASSERT_TRUE(stringEquals("foo", super->method->start, super->method->length));
}

TEST_F(ParserTest, expression_call_no_args) {
  setupParser("foo()");

  Call* call = static_cast<Call*>(parser->expression());
  assertLiteral("foo", TOKEN_IDENTIFIER, call->callee);
  ASSERT_EQ(0, call->arguments.size());
}

TEST_F(ParserTest, expression_call_with_args) {
  setupParser("bar(123, myvar)");

  Call* call = static_cast<Call*>(parser->expression());
  assertLiteral("bar", TOKEN_IDENTIFIER, call->callee);

  ASSERT_EQ(2, call->arguments.size());
  assertLiteral("123", TOKEN_NUMBER, call->arguments[0]);
  assertLiteral("myvar", TOKEN_IDENTIFIER, call->arguments[1]);
}

TEST_F(ParserTest, expression_unary_not) {
  setupParser("!true");

  Unary* unary = static_cast<Unary*>(parser->expression());
  ASSERT_TRUE(stringEquals("!", unary->op->start, unary->op->length));
  assertLiteral("true", TOKEN_TRUE, unary->right);
}

TEST_F(ParserTest, expression_unary_minus) {
  setupParser("-29");

  Unary* unary = static_cast<Unary*>(parser->expression());
  ASSERT_TRUE(stringEquals("-", unary->op->start, unary->op->length));
  assertLiteral("29", TOKEN_NUMBER, unary->right);
}

TEST_F(ParserTest, expression_factor) {
  setupParser("1 * 4 / 2");

  Binary* root = static_cast<Binary*>(parser->expression());

  Binary* bin = static_cast<Binary*>(root->left);
  assertLiteral("1", TOKEN_NUMBER, bin->left);
  ASSERT_TRUE(stringEquals("*", bin->op->start, bin->op->length));
  assertLiteral("4", TOKEN_NUMBER, bin->right);

  ASSERT_TRUE(stringEquals("/", root->op->start, root->op->length));
  assertLiteral("2", TOKEN_NUMBER, root->right);
}

TEST_F(ParserTest, expression_term) {
  setupParser("1 + 2 - 3");

  Binary* root = static_cast<Binary*>(parser->expression());

  Binary* bin = static_cast<Binary*>(root->left);
  assertLiteral("1", TOKEN_NUMBER, bin->left);
  ASSERT_TRUE(stringEquals("+", bin->op->start, bin->op->length));
  assertLiteral("2", TOKEN_NUMBER, bin->right);

  ASSERT_TRUE(stringEquals("-", root->op->start, root->op->length));
  assertLiteral("3", TOKEN_NUMBER, root->right);
}

TEST_F(ParserTest, expression_comparison) {
  setupParser("5 > 1 + 2");

  Binary* root = static_cast<Binary*>(parser->expression());

  ASSERT_TRUE(stringEquals(">", root->op->start, root->op->length));
  assertLiteral("5", TOKEN_NUMBER, root->left);

  Binary* bin = static_cast<Binary*>(root->right);
  assertLiteral("1", TOKEN_NUMBER, bin->left);
  ASSERT_TRUE(stringEquals("+", bin->op->start, bin->op->length));
  assertLiteral("2", TOKEN_NUMBER, bin->right);
}

TEST_F(ParserTest, expression_equality) {
  setupParser("3 == 4");

  Binary* root = static_cast<Binary*>(parser->expression());

  assertLiteral("3", TOKEN_NUMBER, root->left);
  ASSERT_TRUE(stringEquals("==", root->op->start, root->op->length));
  assertLiteral("4", TOKEN_NUMBER, root->right);
}

TEST_F(ParserTest, expression_and) {
  setupParser("true and 123");

  Logical* root = static_cast<Logical*>(parser->expression());

  assertLiteral("true", TOKEN_TRUE, root->left);
  ASSERT_TRUE(stringEquals("and", root->op->start, root->op->length));
  assertLiteral("123", TOKEN_NUMBER, root->right);
}

TEST_F(ParserTest, expression_or) {
  setupParser("false or 123");

  Logical* root = static_cast<Logical*>(parser->expression());

  assertLiteral("false", TOKEN_FALSE, root->left);
  ASSERT_TRUE(stringEquals("or", root->op->start, root->op->length));
  assertLiteral("123", TOKEN_NUMBER, root->right);
}

TEST_F(ParserTest, expression_assignment_assign) {
  setupParser("myvar = 123");

  Assign* as = static_cast<Assign*>(parser->expression());

  ASSERT_TRUE(stringEquals("myvar", as->name->start, as->name->length));
  assertLiteral("123", TOKEN_NUMBER, as->value);
}

TEST_F(ParserTest, expression_assignment_set) {
  setupParser("foo.bar = 123");

  Set* set = static_cast<Set*>(parser->expression());

  assertLiteral("foo", TOKEN_IDENTIFIER, set->object);
  ASSERT_TRUE(stringEquals("bar", set->name->start, set->name->length));
  assertLiteral("123", TOKEN_NUMBER, set->value);
}

TEST_F(ParserTest, expressionStatement) {
  setupParserAndRun("123;");

  Expression* stmt = static_cast<Expression*>(parser->result().stmts[0]);
  assertLiteral("123", TOKEN_NUMBER, stmt->expression);
}

TEST_F(ParserTest, returnStatement) {
  setupParserAndRun("return 123;");

  Return* stmt = static_cast<Return*>(parser->result().stmts[0]);
  assertLiteral("123", TOKEN_NUMBER, stmt->value);
}

TEST_F(ParserTest, printStatement) {
  setupParserAndRun("print 123;");

  Print* stmt = static_cast<Print*>(parser->result().stmts[0]);
  assertLiteral("123", TOKEN_NUMBER, stmt->expression);
}

TEST_F(ParserTest, block) {
  setupParserAndRun("{ print 123; return; }");

  Block* stmt = static_cast<Block*>(parser->result().stmts[0]);
  ASSERT_EQ(2, stmt->statements.size());
}

TEST_F(ParserTest, whileStatement) {
  setupParserAndRun("while (true) { print 123; return; }");

  While* stmt = static_cast<While*>(parser->result().stmts[0]);
  assertLiteral("true", TOKEN_TRUE, stmt->condition);

  Block* body = static_cast<Block*>(stmt->body);
  ASSERT_EQ(2, body->statements.size());
}

TEST_F(ParserTest, ifStatement) {
  setupParserAndRun("if (999) { print 123; return; } else { print false; }");

  If* stmt = static_cast<If*>(parser->result().stmts[0]);
  assertLiteral("999", TOKEN_NUMBER, stmt->condition);
  ASSERT_EQ(2, static_cast<Block*>(stmt->thenBranch)->statements.size());
  ASSERT_EQ(1, static_cast<Block*>(stmt->elseBranch)->statements.size());
}

TEST_F(ParserTest, forStatement) {
  setupParserAndRun("for (; false; 999) { print 123; }");

  While* stmt = static_cast<While*>(parser->result().stmts[0]);
  assertLiteral("false", TOKEN_FALSE, stmt->condition);

  Block* sugarBody = static_cast<Block*>(stmt->body);

  Block* orgnBody = static_cast<Block*>(sugarBody->statements[0]);
  Print* print = static_cast<Print*>(orgnBody->statements[0]);
  assertLiteral("123", TOKEN_NUMBER, print->expression);

  Expression* incr = static_cast<Expression*>(sugarBody->statements[1]);
  assertLiteral("999", TOKEN_NUMBER, incr->expression);
}

TEST_F(ParserTest, varDeclaration) {
  setupParserAndRun("var myvar = 123;");

  Var* stmt = static_cast<Var*>(parser->result().stmts[0]);
  ASSERT_TRUE(stringEquals("myvar", stmt->name->start, stmt->name->length));
  assertLiteral("123", TOKEN_NUMBER, stmt->initializer);
}

TEST_F(ParserTest, funcDeclaration) {
  setupParserAndRun("fun myFunc(p1, p2) { print p1; return p2; }");

  Function* f = static_cast<Function*>(parser->result().stmts[0]);
  ASSERT_TRUE(stringEquals("myFunc", f->name->start, f->name->length));

  ASSERT_TRUE(stringEquals("p1", f->params[0]->start, f->params[0]->length));
  ASSERT_TRUE(stringEquals("p2", f->params[1]->start, f->params[1]->length));

  ASSERT_EQ(2, f->body.size());
}

TEST_F(ParserTest, classDeclaration) {
  setupParserAndRun(
    "class Child < Parent { \
       m1() { print 123; } \
       m2() { print 999; } \
  }");

  Class* c = static_cast<Class*>(parser->result().stmts[0]);
  ASSERT_TRUE(stringEquals("Child", c->name->start, c->name->length));
  ASSERT_TRUE(stringEquals("Parent", c->superclass->name->start, c->superclass->name->length));
  ASSERT_EQ(2, c->methods.size());
}
