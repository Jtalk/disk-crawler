
add_executable(tests tests.cpp)

set(CMAKE_CXX_FLAGS "-DTEST ${CMAKE_CXX_FLAGS}")

find_package(LibArchive)

target_link_libraries(tests ${LibArchive_LIBRARIES} cppunit)

include_directories(..)