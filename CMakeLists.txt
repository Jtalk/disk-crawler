cmake_minimum_required(VERSION 2.8)
project(crawler)

# Compilation settings
set (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
set (CMAKE_CXX_FLAGS "-std=c++11 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wdeprecated -march=native -fsanitize=address -fno-omit-frame-pointer ${CMAKE_CXX_FLAGS}")

set (CMAKE_CXX_FLAGS_DEBUG          "-g -O0 -DDEBUG") # -DVERBOSE
set (CMAKE_CXX_FLAGS_RELEASE        "-Ofast -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -g")

set (CMAKE_AR      "/usr/bin/llvm-ar")
set (CMAKE_LINKER  "/usr/bin/llvm-ld")
set (CMAKE_NM      "/usr/bin/llvm-nm")
set (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")

# Base library building
add_subdirectory(base)

# Building libcrawl libarary

find_package(LibArchive REQUIRED)

# Building final executable
add_executable(crawler main.cpp)

target_link_libraries(crawler crawlerbase iconv ${LibArchive_LIBRARIES})

install(TARGETS crawler RUNTIME DESTINATION bin)

# Enabling testing capabilities of CMake
add_subdirectory(test)

enable_testing()
add_test(NAME Tests COMMAND test/tests)
set_tests_properties(Tests PROPERTIES PASS_REGULAR_EXPRESSION "TEST PASSED;Pass")
        