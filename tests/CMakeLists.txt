target_link_libraries(tester utilities)
target_link_libraries(tester ledger)
target_link_libraries(pool utilities)
target_include_directories(tester INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})