cmake_minimum_required(VERSION 4.2.3)

set(CMAKE_CXX_STANDARD 20)
project(order_book)

find_package(Boost REQUIRED COMPONENTS thread)

add_executable(server src/server/server.cpp)
add_executable(tester tests/tester.cpp)
add_executable(pool tests/test_pool.cpp)

add_subdirectory(src)
add_subdirectory(utilities)
add_subdirectory(tests)

target_link_libraries(server Boost::thread)
target_link_libraries(tester Boost::thread)
