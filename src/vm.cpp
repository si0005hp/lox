#include "vm.h"

#include <stdarg.h>

#include <iostream>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "op_code.h"
#include "value/object.h"
#include "value/value.h"

namespace lox {

  VM::VM(std::ostream& out)
    : out_(out) {
    Memory::initialize(this);
    initString_ = allocateObj<ObjString>("init", 4);
  }

  VM::~VM() {
    freeObjects();
  }

  ObjFunction* VM::compileSource(const char* source) {
    Compiler compiler(*this, nullptr, source);
    return compiler.compile();
  }

  InterpretResult VM::interpret(const char* source) {
    ObjFunction* function = compileSource(source);
    if (!function) return INTERPRET_COMPILE_ERROR;

    pushRoot(function);
    ObjClosure* closure = allocateObj<ObjClosure>(function);
    popRoot();

    push(closure->asValue());
    callValue(closure->asValue(), 0);
    return run();
  }

  void VM::freeObjects() {
    Obj* obj = objects_;
    while (obj) {
      Obj* next = obj->next_;
      freeObject(obj);
      obj = next;
    }
  }

  void VM::freeObject(Obj* obj) {
#ifdef DEBUG_LOG_GC
    std::cout << "free " << *obj << " @ " << obj << std::endl;
#endif
    obj->~Obj(); // TODO: Calling destructor is lame
    Memory::deallocate(obj);
  }

  void VM::appendObj(Obj* obj) {
    obj->next_ = objects_;
    objects_ = obj;
  }

  ObjString* VM::findOrAllocateString(const char* src, int length) {
    ObjString* obj = strings_.find(src, length);
    if (obj) return obj;

    obj = ObjString::allocate(src, length);
    appendObj(obj);

    pushRoot(obj);
    strings_.add(obj);
    popRoot();

    return obj;
  }

  void VM::appendCallFrame(ObjClosure* closure, int stackStart) {
    frames_[frameCount_++] = CallFrame(closure, stackStart);
  }

  InterpretResult VM::run() {
    instruction inst;
    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
      traceStack();
      Disassembler::disassembleInstruction(currentChunk(), currentFrame().ip);
#endif

#define BINARY_OP(op)                                 \
  do {                                                \
    if (!peek(0).isNumber() || !peek(1).isNumber()) { \
      runtimeError("Operands must be numbers.");      \
      return INTERPRET_RUNTIME_ERROR;                 \
    }                                                 \
    Number b = pop().asNumber();                      \
    Number a = pop().asNumber();                      \
    push((op).asValue());                             \
  } while (false)

      switch (inst = readByte()) {
        case OP_POP: pop(); break;

        case OP_GET_LOCAL: {
          instruction slot = readByte();
          push(load(currentStackStart() + slot));
          break;
        }
        case OP_SET_LOCAL: {
          instruction slot = readByte();
          store(currentStackStart() + slot, peek(0));
          break;
        }

        case OP_GET_GLOBAL: {
          ObjString* name = readString();
          Value value;
          if (!globals_.get(name, &value)) {
            runtimeError("Undefined variable '%s'.", name->value());
            return INTERPRET_RUNTIME_ERROR;
          }
          push(value);
          break;
        }
        case OP_SET_GLOBAL: {
          ObjString* name = readString();
          if (!globals_.containsKey(name)) {
            runtimeError("Undefined variable '%s'.", name->value());
            return INTERPRET_RUNTIME_ERROR;
          }
          globals_.put(name, peek(0));
          break;
        }

        case OP_GET_UPVALUE: {
          instruction slot = readByte();
          push(*currentFrame().closure->upvalues()[slot]->location());
          break;
        }
        case OP_SET_UPVALUE: {
          instruction slot = readByte();
          *currentFrame().closure->upvalues()[slot]->location() = peek(0);
          break;
        }

        case OP_GET_PROPERTY: {
          if (!peek(0).isInstance()) {
            runtimeError("Only instances have properties.");
            return INTERPRET_RUNTIME_ERROR;
          }
          ObjInstance* instance = peek(0).asInstance();
          ObjString* name = readString();

          // If it was the field, push
          Value value;
          if (instance->fields().get(name, &value)) {
            pop(); // Instance
            push(value);
            break;
          }
          // Otherwise try to find method
          Method method;
          if (instance->klass()->methods().get(name, &method)) {
            createBoundMethod(method);
            break;
          }

          runtimeError("Undefined property '%s'.", name->value());
          return INTERPRET_RUNTIME_ERROR;
        }
        case OP_SET_PROPERTY: {
          if (!peek(0).isInstance()) {
            runtimeError("Only instances have fields.");
            return INTERPRET_RUNTIME_ERROR;
          }
          ObjInstance* instance = peek(0).asInstance();
          instance->fields().put(readString(), peek(1));

          pop(); // Instance
          // Leave assigned value on the stack
          break;
        }

        case OP_DEFINE_GLOBAL: {
          ObjString* name = readString();
          globals_.put(name, peek(0));
          pop();
          break;
        }

        case OP_CONSTANT: push(readConstant()); break;
        case OP_NIL: push(Nil().asValue()); break;
        case OP_TRUE: push(Bool(true).asValue()); break;
        case OP_FALSE: push(Bool(false).asValue()); break;

        case OP_EQUAL: {
          Value b = pop();
          Value a = pop();
          push(Bool(a == b).asValue());
          break;
        }

        case OP_NOT: push(Bool(pop().isFalsey()).asValue()); break;
        case OP_NEGATE: {
          if (!peek(0).isNumber()) {
            runtimeError("Operand must be a number.");
            return INTERPRET_RUNTIME_ERROR;
          }
          push((-pop().asNumber()).asValue());
          break;
        }

        case OP_ADD: {
          if (peek(0).isString() && peek(1).isString()) {
            ObjString* b = pop().asString();
            ObjString* a = pop().asString();
            push(concatString(a, b)->asValue());
          } else if (peek(0).isNumber() && peek(1).isNumber()) {
            Number b = pop().asNumber();
            Number a = pop().asNumber();
            push((a + b).asValue());
          } else {
            runtimeError("Operands must be two numbers or two strings.");
            return INTERPRET_RUNTIME_ERROR;
          }
          break;
        }
        case OP_SUBTRACT: BINARY_OP(a - b); break;
        case OP_MULTIPLY: BINARY_OP(a * b); break;
        case OP_DIVIDE: BINARY_OP(a / b); break;
        case OP_GREATER: BINARY_OP(Bool(a > b)); break;
        case OP_LESS: BINARY_OP(Bool(a < b)); break;

        case OP_PRINT: {
          out_ << pop() << std::endl;
          break;
        }

        case OP_JUMP: {
          uint16_t offset = readShort();
          currentFrame().ip += offset;
          break;
        }
        case OP_JUMP_IF_FALSE: {
          uint16_t offset = readShort();
          if (peek(0).isFalsey()) currentFrame().ip += offset;
          break;
        }
        case OP_LOOP: {
          uint16_t offset = readShort();
          currentFrame().ip -= offset;
          break;
        }
        case OP_AND: {
          uint16_t offset = readShort();
          if (peek(0).isFalsey()) {
            currentFrame().ip += offset;
          } else {
            pop();
          }
          break;
        }
        case OP_OR: {
          uint16_t offset = readShort();
          if (peek(0).isFalsey()) {
            pop();
          } else {
            currentFrame().ip += offset;
          }
          break;
        }

        case OP_CALL: {
          int argCount = readByte();
          Value callee = peek(argCount);
          if (!callValue(callee, argCount)) {
            return INTERPRET_RUNTIME_ERROR;
          }
          break;
        }
        case OP_CLOSURE: {
          ObjClosure* closure = allocateObj<ObjClosure>(readConstant().asFunction());
          push(closure->asValue());

          for (int i = 0; i < closure->fn()->upvalueCount(); i++) {
            instruction isLocal = readByte();
            instruction index = readByte();
            if (isLocal == 1) {
              // Make an new upvalue to close over the parent's local variable.
              // TODO: Directly accesing stack here
              closure->upvalues()[i] = captureUpvalue(&stack_[currentStackStart() + index]);
            } else {
              // Grab an upvalue from the enclosing function, which we are executing at the moment.
              closure->upvalues()[i] = currentFrame().closure->upvalues()[index];
            }
          }
          break;
        }
        case OP_CLOSE_UPVALUE: {
          closeUpvalues(&stack_[stackTop_ - 1]);
          pop();
          break;
        }

        case OP_CLASS: {
          push(allocateObj<ObjClass>(readString())->asValue());
          break;
        }
        case OP_METHOD: {
          defineMethod(readString());
          break;
        }

        case OP_RETURN: {
          // Save data for subsequent processes.
          Value result = pop();
          int frameStackStart = currentStackStart();

          closeUpvalues(&stack_[frameStackStart]);

          frameCount_--;
          if (frameCount_ == 0) {
            // Top-level done. Pop global script out and finish.
            pop();
            return INTERPRET_OK;
          }

          // Truncate stack of the frame.
          stackTop_ = frameStackStart;
          push(result);
          break;
        }
      }
    }
  }

  void VM::createBoundMethod(Method method) {
    ObjBoundMethod* boundMethod = allocateObj<ObjBoundMethod>(peek(0), method);
    pop(); // receiver
    push(boundMethod->asValue());
  }

  void VM::defineMethod(ObjString* name) {
    peek(1).asClass()->methods().put(name, Method(peek(0).asClosure()));
    pop(); // Pop ObjClosure on top pf the stack
  }

  ObjUpvalue* VM::captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = nullptr;
    ObjUpvalue* upvalue = openUpvalues_;

    // Walk towards the bottom of the stack until we find a previously existing
    // upvalue or pass where it should be.
    while (upvalue != nullptr && upvalue->location() > local) {
      prevUpvalue = upvalue;
      upvalue = upvalue->next();
    }

    // Found an existing upvalue for this local.
    if (upvalue != nullptr && upvalue->location() == local) return upvalue;

    // Make a new one, link it.
    ObjUpvalue* createdUpvalue = allocateObj<ObjUpvalue>(local);
    if (prevUpvalue == nullptr) {
      openUpvalues_ = createdUpvalue;
    } else {
      prevUpvalue->next_ = createdUpvalue;
    }
    return createdUpvalue;
  }

  void VM::closeUpvalues(Value* last) {
    while (openUpvalues_ != nullptr && openUpvalues_->location() > last) {
      ObjUpvalue* upvalue = openUpvalues_;
      upvalue->doClose();
      openUpvalues_ = upvalue->next();
    }
  }

  bool VM::callValue(Value callee, int argCount) {
    if (callee.isClosure()) {
      return call(callee.asClosure(), argCount);
    } else if (callee.isClass()) {
      store(stackTop_ - argCount - 1, allocateObj<ObjInstance>(callee.asClass())->asValue());
      Method init;
      if (callee.asClass()->methods().get(initString_, &init)) {
        return call(init.asClosure(), argCount);
      } else if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d.", argCount);
        return false;
      }
      return true;
    } else if (callee.isBoundMethod()) {
      ObjBoundMethod* boundMethod = callee.asBoundMethod();
      store(stackTop_ - argCount - 1, boundMethod->receiver());
      return call(boundMethod->method().asClosure(), argCount); // TODO
    }
    runtimeError("Can only call functions and classes.");
    return false;
  }

  bool VM::call(ObjClosure* closure, int argCount) {
    if (argCount != closure->fn()->arity()) {
      runtimeError("Expected %d arguments but got %d.", closure->fn()->arity(), argCount);
      return false;
    }
    if (frameCount_ == FRAMES_MAX) {
      runtimeError("Stack overflow.");
      return false;
    }

    appendCallFrame(closure, stackTop_ - argCount - 1);
    return true;
  }

  void VM::traceStack() {
    std::cout << "          ";
    for (int i = 0; i < stackTop_; i++) std::cout << "[ " << stack_[i] << " ]";
    std::cout << std::endl;
  }

  void VM::runtimeError(const char* format, ...) const {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // print stacktrace
    for (int i = frameCount_ - 1; i >= 0; i--) {
      const CallFrame& frame = frames_[i];
      int line = frame.closure->fn()->chunk().getLine(frame.ip - 1);
      std::cerr << "[line " << line << "] in " << *frame.closure << std::endl;
    }
  }

  ObjString* VM::concatString(ObjString* left, ObjString* right) {
    pushRoot(left);
    pushRoot(right);

    int length = left->length() + right->length();
    char* chars = Memory::allocate<char>(length + 1);
    memcpy(chars, left->value(), left->length());
    memcpy(chars + right->length(), right->value(), right->length());
    chars[length] = '\0';

    ObjString* result = allocateObj<ObjString>(chars, length);
    Memory::reallocate(chars, sizeof(char) * length, 0);
    popRoot();
    popRoot();
    return result;
  }

  void VM::gcMarkRoots() {
    // VM stack
    for (int i = 0; i < stackTop_; i++) {
      gcMarkValue(stack_[i]);
    }

    // Functions in callframes
    for (int i = 0; i < frameCount_; i++) {
      gcMarkObject(frames_[i].closure);
    }

    // Open upvalues.
    for (ObjUpvalue* upvalue = openUpvalues_; upvalue != nullptr; upvalue = upvalue->next()) {
      gcMarkObject(upvalue);
    }

    // Global variables
    // TODO: Lame Map blackening codes
    for (int i = 0; i < globals_.capacity(); ++i) {
      Map<StringKey, Value>::Entry* e = globals_.getEntry(i);
      if (e->isEmpty()) continue;

      gcMarkObject(e->key.value());
      gcMarkValue(e->value);
    }

    // Compiler
    Compiler* compiler = compiler_;
    while (compiler) {
      compiler->gcBlacken(*this);
      compiler = compiler->enclosing_;
    }

    gcMarkObject(initString_);
  }

  void VM::gcBlackenObjects() {
    while (gcGrayStack_.size() > 0) {
      Obj* obj = gcGrayStack_.removeAt(gcGrayStack_.size() - 1);
#ifdef DEBUG_LOG_GC
      std::cout << "blacken " << *obj << " @ " << obj << std::endl;
#endif
      obj->gcBlacken(*this);
    }
  }

  void VM::gcRemoveWeakReferences() {
    strings_.removeUnmarkedStrings();
  }

  void VM::gcSweep() {
    Obj* previous = nullptr;
    Obj* obj = objects_;
    while (obj) {
      if (obj->isGCMarked_) {
        obj->isGCMarked_ = false;
        previous = obj;
        obj = obj->next_;
      } else {
        Obj* unreached = obj;
        obj = obj->next_;
        if (previous) {
          previous->next_ = obj;
        } else {
          objects_ = obj;
        }

        freeObject(unreached);
      }
    }
  }

  void VM::gcMarkValue(Value value) {
    if (!value.isObj()) return;

    gcMarkObject(value.asObj());
  }

  void VM::gcMarkObject(Obj* obj) {
    if (!obj) return;
    if (obj->isGCMarked_) return;

#ifdef DEBUG_LOG_GC
    std::cout << "mark " << *obj << " @ " << obj << std::endl;
#endif
    obj->isGCMarked_ = true;
    gcGrayStack_.push(obj);
  }

} // namespace lox
