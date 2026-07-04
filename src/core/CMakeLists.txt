add_library(ledger ledger.cpp)
target_link_libraries(ledger utilities)
target_include_directories(ledger INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})