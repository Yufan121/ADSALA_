cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "Google Test not found. Fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    set(GTest_FOUND TRUE)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src/library_frame/BLAS/include
    ${CMAKE_SOURCE_DIR}/src/data_collect
    ${CMAKE_SOURCE_DIR}/src/library_frame/predictor
)

# Test executables will be added in subdirectories
add_subdirectory(unit)
add_subdirectory(integration)

