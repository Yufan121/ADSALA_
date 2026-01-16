# Unit tests directory

# Test: Exception handling
add_executable(test_exceptions test_exceptions.cpp)
target_link_libraries(test_exceptions 
    GTest::gtest 
    GTest::gtest_main
)
target_include_directories(test_exceptions PRIVATE
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/include
)
add_test(NAME ExceptionsTest COMMAND test_exceptions)

# Test: Configuration system
add_executable(test_config test_config.cpp)
target_link_libraries(test_config 
    GTest::gtest 
    GTest::gtest_main
    module
)
target_include_directories(test_config PRIVATE
    ${CMAKE_SOURCE_DIR}/library_frame/BLAS/include
)
add_test(NAME ConfigTest COMMAND test_config)

# Test: CSV operations
add_executable(test_csv test_csv.cpp)
target_link_libraries(test_csv 
    GTest::gtest 
    GTest::gtest_main
    data_collect
)
target_include_directories(test_csv PRIVATE
    ${CMAKE_SOURCE_DIR}/src/data_collect
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/include
)
add_test(NAME CSVTest COMMAND test_csv)

# Test: Routine dispatcher
add_executable(test_dispatcher test_dispatcher.cpp)
target_link_libraries(test_dispatcher 
    GTest::gtest 
    GTest::gtest_main
    data_collect
)
target_include_directories(test_dispatcher PRIVATE
    ${CMAKE_SOURCE_DIR}/data_collect
    ${CMAKE_SOURCE_DIR}/library_frame/BLAS/include
)
add_test(NAME DispatcherTest COMMAND test_dispatcher)

# Test: Template functions
add_executable(test_templates test_templates.cpp)
target_link_libraries(test_templates
    GTest::gtest
    GTest::gtest_main
    module
    data_collect
)
target_include_directories(test_templates PRIVATE
    ${CMAKE_SOURCE_DIR}/library_frame/BLAS/include
    ${CMAKE_SOURCE_DIR}/data_collect
)
add_test(NAME TemplatesTest COMMAND test_templates)

# Test: Helper functions
add_executable(test_helpers test_helpers.cpp)
target_link_libraries(test_helpers
    GTest::gtest
    GTest::gtest_main
    module
)
target_include_directories(test_helpers PRIVATE
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/include
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/src
)
add_test(NAME HelpersTest COMMAND test_helpers)

# Test: Predictor helper functions
add_executable(test_predictor_helpers test_predictor_helpers.cpp)
target_link_libraries(test_predictor_helpers
    GTest::gtest
    GTest::gtest_main
    module
)
target_include_directories(test_predictor_helpers PRIVATE
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/include
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/src
    ${CMAKE_SOURCE_DIR}/src/library_frame/predictor
)
add_test(NAME PredictorHelpersTest COMMAND test_predictor_helpers)

# Test: Template implementations
add_executable(test_template_implementations test_template_implementations.cpp)
target_link_libraries(test_template_implementations
    GTest::gtest
    GTest::gtest_main
    module
)
target_include_directories(test_template_implementations PRIVATE
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/include
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/src
)
add_test(NAME TemplateImplementationsTest COMMAND test_template_implementations)

