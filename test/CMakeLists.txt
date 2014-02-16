
add_executable(tests tests.cpp)

set(CMAKE_CXX_FLAGS "-DTEST ${CMAKE_CXX_FLAGS}")

target_link_libraries(tests crawl cppunit)
