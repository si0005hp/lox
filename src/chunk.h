#pragma once

#include "common.h"
#include "lib/vector.h"
#include "value/value.h"

namespace lox {

  typedef uint8_t instruction;

  class Chunk {
   public:
    void write(instruction inst, int line) {
      code_.push(inst);
      lines_.push(line);
    }

    void rewrite(int index, instruction inst) {
      code_[index] = inst;
    }

    instruction getCode(int index) const {
      return code_[index];
    }

    int getLine(int index) const {
      return lines_[index];
    }

    int count() const {
      return code_.size();
    }

    int addConstant(Value value) {
      constants_.push(value);
      return constants_.size() - 1;
    }

    Value getConstant(int index) const {
      return constants_[index];
    }

   private:
    Vector<instruction> code_;
    Vector<int> lines_;
    Vector<Value> constants_;
  };

} // namespace lox
