cmake_minimum_required(VERSION 3.4)
project(main)

set(CMAKE_CXX_FLAGS "-g -std=c++2a -Wall")
if(ADDITIONAL_CXX_FLAGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")


set(LOX_SRC_DIR "${PROJECT_SOURCE_DIR}/src")
file(GLOB_RECURSE lox_src_files ${LOX_SRC_DIR}/*.cpp)
add_library(lox_lib ${lox_src_files})

add_executable(lox ${LOX_SRC_DIR}/main.cpp)
target_link_libraries(lox lox_lib)

# tests
option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(test)
endif()
