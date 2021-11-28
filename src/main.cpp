#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "vm.h"

using namespace lox;

static bool readFile(const char* path, char** bufp) {
  FILE* file = fopen(path, "rb");
  if (file == nullptr) return false;

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  *bufp = static_cast<char*>(::operator new(fileSize + 1));
  if (*bufp == nullptr) return false;

  size_t bytesRead = fread(*bufp, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) return false;

  (*bufp)[bytesRead] = '\0';
  fclose(file);
  return true;
}

int main(int argc, char const* argv[]) {
  if (argc < 2) {
    std::cout << "File path is not given." << std::endl;
    exit(-1);
  }

  char* buf = nullptr;
  if (!readFile(argv[1], &buf)) {
    exit(-1);
  }

  VM vm;
  InterpretResult result = vm.interpret(buf);
  delete buf;

  if (result == INTERPRET_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) exit(70);

  return 0;
}
