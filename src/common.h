#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <iostream>

#ifdef DEBUG

#include "../tmp/dbg.h" // TODO: path

#define ASSERT(condition, message)                                                      \
  if (!(condition)) {                                                                   \
    std::cerr << "ASSERTION FAILED " << __FILE__ << ":" << __LINE__ << " - " << message \
              << std::endl;                                                             \
    abort();                                                                            \
  }

#define ASSERT_INDEX(index, max)                                                               \
  if (((index) < 0) || ((index) >= max)) {                                                     \
    std::cerr << "[ASSERTION FAILURE] " << __FILE__ << ":" << __LINE__ << " - "                \
              << "Index " << index << " out of bounds for length " << max << "." << std::endl; \
    abort();                                                                                   \
  }

#define UNREACHABLE()                                                                     \
  std::cerr << "[" __FILE__ << ":" << __LINE__ << "] This code should not be reached in " \
            << __func__ << "()" << std::endl;                                             \
  abort();

//< Debug flags
#define DEBUG_TRACE_EXECUTION
#define DEBUG_PRINT_CODE
//> Debug flags

#else

#define ASSERT(condition, message) ;
#define ASSERT_INDEX(index, max) ;
#define UNREACHABLE() __builtin_unreachable()

#endif

#define FLEXIBLE_ARRAY (1)

// GC flags
#ifdef STRESS_GC
#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC
#endif
