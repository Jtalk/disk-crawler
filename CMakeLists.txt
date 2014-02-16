cmake_minimum_required(VERSION 2.8)
project(crawler)

add_library(crawl ZipDecoder.cpp PlainDecoder.cpp base/BaseDecoder.cpp FATFileStream.cpp base/FSWalker.cpp base/FSFileStream.cpp FATWalker.cpp )
find_package(LibArchive REQUIRED)
target_link_libraries(crawl  ${LibArchive_LIBRARIES})

add_executable(crawler main.cpp)

install(TARGETS crawler RUNTIME DESTINATION bin)
# 
# SET (CMAKE_C_COMPILER             "/usr/bin/clang")
# SET (CMAKE_C_FLAGS                "-Wall -std=c11")
# SET (CMAKE_C_FLAGS_DEBUG          "-g")
# SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
# SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
# SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
# 
# SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
# SET (CMAKE_CXX_FLAGS                "")
# SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
# SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
# SET (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
# SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
# 
# SET (CMAKE_AR      "/usr/bin/llvm-ar")
# SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
# SET (CMAKE_NM      "/usr/bin/llvm-nm")
# SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
# SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")

set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wdeprecated -march=native -O0 -g ${CMAKE_CXX_FLAGS}")

target_link_libraries(crawler crawl)

add_subdirectory(test)
enable_testing()
add_test(NAME Tests COMMAND test/tests)
set_tests_properties(Tests PROPERTIES PASS_REGULAR_EXPRESSION "TEST PASSED;Pass")
        