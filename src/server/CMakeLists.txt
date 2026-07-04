add_executable(server server.cpp)

target_link_libraries(server ledger)
target_link_libraries(server utilities)