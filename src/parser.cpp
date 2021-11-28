#include "parser.h"

#include <cstdarg>
#include <iostream>
#include <new>
#include <sstream>

#include "common.h"
#include "memory.h"

namespace lox {

  Parser::Parser(Lexer& lexer)
    : lexer_(lexer)
    , astBytesAllocated_(0)
    , hadError_(false)
    , panicMode_(false) {}

  Parser::~Parser() {
    for (int i = 0; i < astNodes_.size(); i++) {
      Memory::deallocate<Ast>(astNodes_[i]);
      // TODO: Recover allocated bytes
    }
  }

  bool Parser::parse() {
    while (!isDone()) result_.stmts.push(declaration());

    result_.eof = consume(TOKEN_EOF, "Expect EOF at last.");
    return !hadError_;
  }

  Expr* Parser::expression() {
    return assignment();
  }

  Stmt* Parser::declaration() {
    Stmt* decl;
    if (match(TOKEN_CLASS)) {
      decl = classDeclaration();
    } else if (match(TOKEN_FUN)) {
      decl = funcDeclaration();
    } else if (match(TOKEN_VAR)) {
      decl = varDeclaration();
    } else {
      decl = statement();
    }

    if (hadError_) {
      synchronize();
      return nullptr;
    }
    return decl;
  }

  Stmt* Parser::classDeclaration() {
    Token* name = consume(TOKEN_IDENTIFIER, "Expect class name.");

    Variable* superclass = nullptr;
    if (match(TOKEN_LESS)) {
      consume(TOKEN_IDENTIFIER, "Expect superclass name.");
      superclass = newAstNode<Variable>(last_);
    }

    consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    Vector<Function*> methods;
    while (!lookAhead(TOKEN_RIGHT_BRACE) && !isDone()) {
      methods.push(function("method"));
    }
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");

    return newAstNode<Class>(name, superclass, methods);
  }

  Stmt* Parser::funcDeclaration() {
    return function("function");
  }

  Function* Parser::function(const char* kind) {
    Token* name = consume(TOKEN_IDENTIFIER, "Expect %s name.", kind);

    consume(TOKEN_LEFT_PAREN, "Expect '(' after %s name.", kind);
    Vector<Token*> params;
    if (!lookAhead(TOKEN_RIGHT_PAREN)) {
      do {
        if (params.size() >= 255) {
          error("Can't have more than 255 parameters.");
        }
        params.push(consume(TOKEN_IDENTIFIER, "Expect parameter name."));
      } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

    consume(TOKEN_LEFT_BRACE, "Expect '{' before %s body.", kind);
    Vector<Stmt*> body = blockBody();
    return newAstNode<Function>(name, params, body);
  }

  Stmt* Parser::varDeclaration() {
    Token* name = consume(TOKEN_IDENTIFIER, "Expect variable name.");
    Expr* initializer = match(TOKEN_EQUAL) ? expression() : nullptr;
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    return newAstNode<Var>(name, initializer);
  }

  Stmt* Parser::statement() {
    if (match(TOKEN_FOR)) return forStatement();
    if (match(TOKEN_IF)) return ifStatement();
    if (match(TOKEN_PRINT)) return printStatement();
    if (match(TOKEN_RETURN)) return returnStatement();
    if (match(TOKEN_WHILE)) return whileStatement();
    if (match(TOKEN_LEFT_BRACE)) return block();
    return expressionStatement();
  }

  Stmt* Parser::forStatement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    /* for-initializer */
    Stmt* initializer;
    if (match(TOKEN_SEMICOLON)) {
      initializer = nullptr;
    } else if (match(TOKEN_VAR)) {
      initializer = varDeclaration();
    } else {
      initializer = expressionStatement();
    }
    /* for-condition */
    Expr* cond = lookAhead(TOKEN_SEMICOLON) ? nullptr : expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
    /* for-increment */
    Expr* incr = lookAhead(TOKEN_RIGHT_PAREN) ? nullptr : expression();
    Token* stop = consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    /* body */
    Stmt* body = statement();

    if (incr != nullptr) {
      Vector<Stmt*> stmts{body, newAstNode<Expression>(incr, stop)};
      body = newAstNode<Block>(stmts);
    }
    if (cond == nullptr) cond = newAstNode<Literal>(lexer_.syntheticToken(TOKEN_TRUE, "true"));
    body = newAstNode<While>(cond, body);

    if (initializer != nullptr) {
      Vector<Stmt*> stmts{initializer, body};
      body = newAstNode<Block>(stmts);
    }
    return body;
  }

  Stmt* Parser::ifStatement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    Expr* cond = expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    Stmt* thenBody = statement();
    Stmt* elseBody = match(TOKEN_ELSE) ? statement() : nullptr;
    return newAstNode<If>(cond, thenBody, elseBody);
  }

  Stmt* Parser::printStatement() {
    Expr* value = expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    return newAstNode<Print>(value);
  }

  Stmt* Parser::returnStatement() {
    Expr* value = lookAhead(TOKEN_SEMICOLON) ? nullptr : expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
    return newAstNode<Return>(value);
  }

  Stmt* Parser::expressionStatement() {
    Expr* expr = expression();
    Token* stop = consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    return newAstNode<Expression>(expr, stop);
  }

  Stmt* Parser::whileStatement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    Expr* cond = expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    Stmt* body = statement();
    return newAstNode<While>(cond, body);
  }

  Stmt* Parser::block() {
    return newAstNode<Block>(blockBody());
  }

  Vector<Stmt*> Parser::blockBody() {
    Vector<Stmt*> stmts;
    while (!lookAhead(TOKEN_RIGHT_BRACE) && !isDone()) stmts.push(declaration());
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
    return stmts;
  }

  Expr* Parser::assignment() {
    Expr* lhs = or_();

    if (match(TOKEN_EQUAL)) {
      Token* op = last_;
      Expr* rhs = assignment();

      if (typeid(*lhs) == typeid(Variable)) {
        return newAstNode<Assign>(static_cast<Variable*>(lhs)->name, rhs);
      } else if (typeid(*lhs) == typeid(Get)) {
        Get* get = static_cast<Get*>(lhs);
        return newAstNode<Set>(get->object, get->name, rhs);
      }
      errorAt(*op, "Invalid assignment target.");
    }
    return lhs;
  }

  Expr* Parser::or_() {
    Expr* expr = and_();

    while (match(TOKEN_OR)) {
      Token* op = last_;
      expr = newAstNode<Logical>(expr, op, and_());
    }
    return expr;
  }

  Expr* Parser::and_() {
    Expr* expr = equality();

    while (match(TOKEN_AND)) {
      Token* op = last_;
      expr = newAstNode<Logical>(expr, op, equality());
    }
    return expr;
  }

  Expr* Parser::equality() {
    Expr* expr = comparison();

    while (match({TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL})) {
      Token* op = last_;
      expr = newAstNode<Binary>(expr, op, comparison());
    }
    return expr;
  }

  Expr* Parser::comparison() {
    Expr* expr = term();

    while (match({TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL})) {
      Token* op = last_;
      expr = newAstNode<Binary>(expr, op, term());
    }
    return expr;
  }

  Expr* Parser::term() {
    Expr* expr = factor();

    while (match({TOKEN_MINUS, TOKEN_PLUS})) {
      Token* op = last_;
      expr = newAstNode<Binary>(expr, op, factor());
    }
    return expr;
  }

  Expr* Parser::factor() {
    Expr* expr = unary();

    while (match({TOKEN_SLASH, TOKEN_STAR})) {
      Token* op = last_;
      expr = newAstNode<Binary>(expr, op, unary());
    }
    return expr;
  }

  Expr* Parser::unary() {
    if (match({TOKEN_BANG, TOKEN_MINUS})) {
      Token* op = last_;
      return newAstNode<Unary>(op, unary());
    }
    return call();
  }

  Expr* Parser::call() {
    Expr* expr = primary();

    while (true) {
      if (match(TOKEN_LEFT_PAREN)) {
        expr = finishCall(expr);
      } else if (match(TOKEN_DOT)) {
        Token* name = consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
        expr = newAstNode<Get>(expr, name);
      } else {
        break;
      }
    }
    return expr;
  }

  Expr* Parser::finishCall(Expr* callee) {
    Vector<Expr*> args;
    if (!lookAhead(TOKEN_RIGHT_PAREN)) {
      do {
        if (args.size() >= 255) {
          error("Can't have more than 255 arguments.");
        }
        args.push(expression());
      } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return newAstNode<Call>(callee, args);
  }

  Expr* Parser::primary() {
    /* literal */
    if (match({TOKEN_FALSE, TOKEN_TRUE, TOKEN_NIL, TOKEN_NUMBER, TOKEN_STRING, TOKEN_THIS})) {
      return newAstNode<Literal>(last_);
    }
    /* variable */
    if (match(TOKEN_IDENTIFIER)) {
      return newAstNode<Variable>(last_);
    }
    /* expression */
    if (match(TOKEN_LEFT_PAREN)) {
      Expr* expr = expression();
      consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
      return newAstNode<Grouping>(expr);
    }
    /* super attr */
    if (match(TOKEN_SUPER)) {
      Token* super = last_;
      consume(TOKEN_DOT, "Expect '.' after 'super'.");
      Token* property = consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
      return newAstNode<Super>(super, property);
    }

    error("Expect expression.");
    return nullptr;
  }

  bool Parser::match(TokenType type) {
    if (lookAhead(type)) {
      consume();
      return true;
    }
    return false;
  }

  bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto type : types) {
      if (match(type)) return true;
    }
    return false;
  }

  bool Parser::lookAhead(TokenType type) {
    fillLookAhead(1);
    return read_[0]->type == type;
  }

  void Parser::fillLookAhead(int count) {
    while (read_.count() < count) {
      Token* token = lexer_.readToken();
      if (token->type == TOKEN_ERROR)
        errorAt(*token, token->start);
      else
        read_.enqueue(token);
    }
  }

  Token* Parser::consume() {
    fillLookAhead(1);
    last_ = read_.dequeue();
    return last_;
  }

  Token* Parser::consume(TokenType type, const char* format, ...) {
    if (lookAhead(type)) return consume();

    error(format);
    return nullptr;
  }

  const Token& Parser::current() {
    fillLookAhead(1);
    return *read_[0];
  }

  bool Parser::isDone() {
    return current().type == TOKEN_EOF;
  }

  void Parser::error(const char* format, ...) {
    errorAt(current(), format);
  }

  void Parser::errorAt(const Token& token, const char* format, ...) {
    if (panicMode_) return;
    panicMode_ = true;

    // TODO: Enough?
    char message[1024];

    va_list args;
    va_start(args, format);
    vsprintf(message, format, args);
    va_end(args);

    std::cerr << "[line " << token.line << "] Error";
    if (token.type == TOKEN_EOF) {
      std::cerr << " at end";
    } else if (token.type == TOKEN_ERROR) {
      // Nothing.
    } else {
      std::cerr << " at " << token.start;
    }
    std::cerr << ": " << message << std::endl;

    hadError_ = true;
  }

  // TODO: valid?
  void Parser::synchronize() {
    panicMode_ = false;

    while (!isDone()) {
      if (last_->type == TOKEN_SEMICOLON) return;

      switch (current().type) {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN: return;
        default:
          // Do nothing.
          ;
      }
      consume();
    }
  }
}; // namespace lox
