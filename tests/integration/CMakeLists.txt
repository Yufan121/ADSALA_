# Integration tests directory

# Integration test: End-to-end workflow
add_executable(test_integration test_integration.cpp)
target_link_libraries(test_integration
    GTest::gtest
    GTest::gtest_main
    data_collect
    module
)
target_include_directories(test_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/data_collect
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/include
)
add_test(NAME IntegrationTest COMMAND test_integration)

# Memory safety tests
add_executable(test_memory_safety test_memory_safety.cpp)
target_link_libraries(test_memory_safety
    GTest::gtest
    GTest::gtest_main
    data_collect
    module
)
target_include_directories(test_memory_safety PRIVATE
    ${CMAKE_SOURCE_DIR}/src/data_collect
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/include
)
add_test(NAME MemorySafetyTest COMMAND test_memory_safety)

