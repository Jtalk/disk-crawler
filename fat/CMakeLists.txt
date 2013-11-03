project(fat)

add_executable(fat FATWalker.cpp main.cpp)

install(TARGETS fat RUNTIME DESTINATION bin)

set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -pedantic-errors -Wall -Wextra ${CMAKE_CXX_FLAGS}")

include_directories(..)