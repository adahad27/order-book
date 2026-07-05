cmake_minimum_required(VERSION 4.2.3)

set(CMAKE_CXX_STANDARD 20)
project(order_book)

find_package(Boost REQUIRED COMPONENTS thread)

add_executable(main main.cpp)
add_executable(server src/server/server.cpp)

add_subdirectory(src)
add_subdirectory(utilities)

target_link_libraries(main ledger)
target_link_libraries(main utilities)
target_link_libraries(server Boost::thread)