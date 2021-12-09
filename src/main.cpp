#include <iostream>

#include "lox.h"

using namespace lox;

int main(int argc, char const* argv[]) {
  if (argc < 2) {
    std::cout << "File path is not given." << std::endl;
    exit(-1);
  }

  InterpretResult result = Lox::runFile(argv[1]);

  if (result == INTERPRET_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) exit(70);

  return 0;
}
