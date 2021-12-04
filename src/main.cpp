#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "vm.h"

using namespace lox;

static char* readFile(const char* path) {
  FILE* file = fopen(path, "rb");
  if (!file) return nullptr;

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buf = static_cast<char*>(::operator new(fileSize + 1));
  if (!buf) return nullptr;

  size_t bytesRead = fread(buf, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) return nullptr;

  buf[bytesRead] = '\0';

  fclose(file);
  return buf;
}

int main(int argc, char const* argv[]) {
  if (argc < 2) {
    std::cout << "File path is not given." << std::endl;
    exit(-1);
  }

  char* buf = readFile(argv[1]);
  if (!buf) {
    exit(-1);
  }

  VM vm;
  InterpretResult result = vm.interpret(buf);
  delete buf;

  if (result == INTERPRET_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) exit(70);

  return 0;
}
