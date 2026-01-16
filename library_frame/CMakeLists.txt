# 指定CMake版本
cmake_minimum_required(VERSION 3.5)

# Project Name
project(TestingADSALA)

message("Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb -Wextra  -lpthread")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -Wextra -lpthread")

SET(CMAKE_CXX_FLAGS_RelWithDebInfo "$ENV{CXXFLAGS} -lpthread")
# set(CMAKE_BUILD_TYPE "RelWithDebInfo")

# if ( NOT BLASType )
#   SET(BLASType "MKL")
#   #SET(BLASType "AmdBLIS")
# endif()


get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)


# 加载并处理一个子文件夹下的 CMakeLists.txt 文件
add_subdirectory(${CMAKE_SOURCE_DIR}/library_frame/predictor)
add_subdirectory(${CMAKE_SOURCE_DIR}/library_frame/BLAS)
add_subdirectory(${CMAKE_SOURCE_DIR}/library_frame/test)

