#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "vm.h"

using namespace lox;

static bool readFile(const char* path, char** bufp) {
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    return false;
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  *bufp = static_cast<char*>(::operator new(fileSize + 1));
  if (*bufp == NULL) {
    return false;
  }

  size_t bytesRead = fread(*bufp, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    return false;
  }
  (*bufp)[bytesRead] = '\0';

  fclose(file);
  return true;
}

int main(int argc, char const* argv[]) {
  if (argc < 2) {
    std::cout << "File path is not given." << std::endl;
    exit(-1);
  }

  char* buf = NULL;
  if (!readFile(argv[1], &buf)) {
    exit(-1);
  }

  VM vm;
  vm.interpret(buf);

  delete buf;

  return 0;
}
