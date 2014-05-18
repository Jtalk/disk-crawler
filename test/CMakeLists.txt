
add_executable(tests tests.cpp)

set(CMAKE_CXX_FLAGS "-DTEST ${CMAKE_CXX_FLAGS}")
find_package(LibArchive REQUIRED)
target_link_libraries(tests crawlerbase iconv cppunit ${LibArchive_LIBRARIES})
