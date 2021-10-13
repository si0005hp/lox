BUILD_DIR=build
# BUILD_TYPE?=Release
BUILD_TYPE?=Debug # Temporary

all: build

build:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	$(MAKE) -C $(BUILD_DIR) -j

clean:
	@$(MAKE) -C $(BUILD_DIR) clean

test: build
	$(MAKE) -C $(BUILD_DIR) -j lox_test
	./build/test/lox_test

format:
	find src test -type f -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -i


build_gen_ast: tool/gen_ast.cpp
	@g++ -g -std=c++2a -Wall tool/gen_ast.cpp -o /tmp/gen_ast

gen_ast_expr: build_gen_ast
	@/tmp/gen_ast > src/ast/expr.h
	@$(MAKE) format

gen_ast_stmt: build_gen_ast
	@/tmp/gen_ast stmt > src/ast/stmt.h
	@$(MAKE) format


NO_PHONY = /^(z):/
.PHONY: $(shell cat $(MAKEFILE_LIST) | awk -F':' '/^[a-z0-9_-]+:/ && !$(NO_PHONY) {print $$1}')
