#pragma once

#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "vm.h"

namespace lox {

  class Lox {
   public:
    static InterpretResult runFile(const char* filePath, std::ostream& out = std::cout) {
      char* buf = readFile(filePath);
      if (!buf) {
        std::cerr << "Failed to load file." << std::endl;
        exit(-1); // TODO: Fix handling
      }

      VM vm(out);
      InterpretResult result = vm.interpret(buf);
      delete buf;

      return result;
    }

   private:
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
  };

} // namespace lox
