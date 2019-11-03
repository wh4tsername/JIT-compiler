cmake_minimum_required(VERSION 3.0)
project(JIT_COMPILER)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -marm")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -marm")

add_executable(JIT_COMPILER main.c parser/parser.cpp arm_compiler/arm_compiler.cpp)

#add_executable(JIT_COMPILER parser/parser.cpp arm_compiler/arm_compiler.cpp parser_test.cpp)
