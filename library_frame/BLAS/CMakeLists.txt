cmake_minimum_required(VERSION 3.10.2)
include(CheckLanguage)

set(OpenBLAS_DIR "~/openblas/")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(module CXX)

# C++ 调试编译时使用的标志
# set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -ggdb3")
# C++ 发行编译时使用的标志
# set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
# set flag to release
# set(CMAKE_BUILD_TYPE "Release") # will this overwrite the previous setting?, yes

# if ( NOT BLASType )
#   SET(BLASType "MKL")
#   # SET(BLASType "AmdBLIS")
# endif()

# make a variable parent folder for the parent folder of CMAKE_CURRENT_SOURCE_DIR
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

#LIB path
SET(CMAKE_PREFIX_PATH ${PARENT_DIR}/lib)
# 指定头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PARENT_DIR}/predictor)
include_directories(/scratch/kx58/yx7184/miniconda/envs/mlpack/include/)
include_directories(/scratch/kx58/yx7184/ML_algo_cpp/mlpack/build/deps/cereal-1.3.0/include/)
 
link_directories(${CMAKE_BINARY_DIR}/predictor)


message(${PROJECT_NAME} )

# Find yaml-cpp library
find_package(yaml-cpp QUIET)
if(NOT yaml-cpp_FOUND)
    message(WARNING "yaml-cpp not found. Configuration system will use defaults.")
    message(STATUS "To install yaml-cpp: sudo apt-get install libyaml-cpp-dev (Ubuntu/Debian)")
    message(STATUS "                      brew install yaml-cpp (macOS)")
endif()

add_library(${PROJECT_NAME} SHARED #${SOURCE_FILES}
                # ${PROJECT_SOURCE_DIR}/src/BLIS-context.cpp
                # ${PROJECT_SOURCE_DIR}/src/get-ncores.cpp
                ${PROJECT_SOURCE_DIR}/src/interfaces-ml.cpp
                ${PROJECT_SOURCE_DIR}/src/blas_config.cpp
                # ${PROJECT_SOURCE_DIR}/src/library-manager.cpp
                # ${PROJECT_SOURCE_DIR}/src/MKL-context.cpp
                )

target_include_directories(${PROJECT_NAME} PRIVATE 
            ${CMAKE_SOURCE_DIR}/data_collect
            )
target_link_libraries(${PROJECT_NAME} data_collect predictor)
if(yaml-cpp_FOUND)
    target_link_libraries(${PROJECT_NAME} yaml-cpp)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_YAML_CPP)
endif()

# # add AMD BLIS library
# set(BLA_VENDOR Intel10_64lp)
# find_package(BLAS)
# if (BLAS_FOUND)
#     message("BLAS found")
# endif()

# if (BLASType STREQUAL "MKL")
#   message("Looking for MKL...")
#   set(BLA_VENDOR Intel10_64lp)
#   find_package(BLAS)
#   if (BLAS_FOUND)
#     target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/MKL-context.cpp)
#     target_compile_definitions(${PROJECT_NAME} PRIVATE USE_MKL)
#     target_link_libraries(${PROJECT_NAME} ${BLAS_LIBRARIES})
#     set(BLAS_CONFIGURED 1)
#   else()
#     message("Failed to find MKL!")
#   endif()

# elseif (BLASType STREQUAL "OpenBLAS")
#   message("Looking for OpenBLAS...")
#   find_library(OpenBLAS NAMES openblas PATHS "${OpenBLAS_DIR}/lib")
#   if(OpenBLAS)
#     target_include_directories(${PROJECT_NAME} PRIVATE ${OpenBLAS_DIR}/include)
#     target_link_libraries(${PROJECT_NAME} ${OpenBLAS})
#     target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenBLAS-context.cpp)
#     target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENBLAS)
#     set(BLAS_CONFIGURED 1)
#   else()
#     message("Failed to find OpenBLAS!")
#   endif()

# elseif (BLASType STREQUAL "AmdBLIS")
#   message("Looking for AmdBlis...")
#   find_library(BLIS NAMES blis-mt PATHS "${PARENT_DIR}/lib")
#   if(BLIS)
#     target_include_directories(${PROJECT_NAME} PRIVATE .)
#     target_link_libraries(${PROJECT_NAME} ${BLIS})
#     target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/BLIS-context.cpp)
#     target_compile_definitions(${PROJECT_NAME} PRIVATE USE_BLIS)
#     set(BLAS_CONFIGURED 1)
#   else()
#     message("Failed to find AmdBLIS!")
#   endif()

# else()
#   message("UNSUPPORTED BLAS LIBRARY GIVEN ${BLASType}")
# endif()


# target_link_libraries(${PROJECT_NAME} ${BLAS_LIBRARIES})

# target_link_libraries(${PROJECT_NAME} ${BLAS_LIBRARIES} m)

