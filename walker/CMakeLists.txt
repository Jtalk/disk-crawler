cmake_minimum_required(VERSION 2.8)
project(walker)

add_executable(walker DiskWalker.cpp main.cpp)

install(TARGETS walker RUNTIME DESTINATION bin)

set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -pedantic-errors -Wall -Wextra ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})

include_directories(..)
