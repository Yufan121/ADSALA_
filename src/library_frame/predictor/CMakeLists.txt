#cmake_minimum_required(VERSION 3.20)
project(api-demo LANGUAGES C CXX VERSION 0.0.1)
add_subdirectory(xgboost)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the directories containing the header files
include_directories(/scratch/kx58/yx7184/local/include)
include_directories(/scratch/kx58/yx7184/miniconda/envs/mlpack/include/)

# add the directories containing the libraries
link_directories(/scratch/kx58/yx7184/local/lib64)
link_directories(/scratch/kx58/yx7184/miniconda/envs/mlpack/lib)

# compile predictor lib and link xgboost
add_library(predictor SHARED 
    predictor.c
    predictor.hpp
    # xgb_model.hpp
)

target_link_libraries(predictor xgboost m ) #math library
# target include directories
# target_include_directories(predictor PRIVATE 
#     /scratch/kx58/yx7184/miniconda/envs/mlpack/include
# )
target_link_libraries(predictor /scratch/kx58/yx7184/miniconda/envs/mlpack/lib/libarmadillo.so)