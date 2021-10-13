#pragma once

#ifdef DEBUG

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

#else

#define ASSERT(condition, message) ;
#define ASSERT_INDEX(index, max) ;

#endif
