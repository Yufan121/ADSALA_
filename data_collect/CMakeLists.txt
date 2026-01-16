cmake_minimum_required(VERSION 3.10.2)
include(CheckLanguage)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -Wextra -g -ggdb -lpthread -fopenmp ")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -lpthread -fopenmp")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--verbose")

# print build type
message("Build type: ${CMAKE_BUILD_TYPE}")

set(OpenBLAS_DIR "~/openblas/")
set(CMAKE_CXX_STANDARD 11)
# set release flags
# set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -lpthread -fopenmp")
project(interfaces CXX)

SET(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/lib)
# append cmake path /apps
list(APPEND CMAKE_PREFIX_PATH $ENV{MKLROOT})



# SET(BLASType "AmdBLIS")
# # print a warning message that I have override the BLAS library
message("BLAS library: ${BLASType}")


add_library(data_collect SHARED
  library-manager.cpp
  interfaces.cpp
  get-ncores.cpp
  # runtime/lookup.cpp
  # main.cpp
  )

target_include_directories(data_collect PRIVATE .)
# target_include_directories(data_collect PRIVATE ../library_frame/predictor)
# target_include_directories(data_collect PRIVATE ../library_frame/predictor/xgboost/include)
# target_link_libraries(data_collect ${CMAKE_BINARY_DIR}/library_frame/predictor/libpredictor.so)

# add_executable(generate-domain
#   generate-domain.cpp
#   )


if (BLASType STREQUAL "MKL")
  message("Looking for MKL...")
  # find MKL
  set(ENV{MKL_THREADING_LAYER}  "GNU")
  find_package(MKL REQUIRED)
  if (MKL_FOUND)
    # use MKL parallel version
    link_directories($ENV{MKLROOT}/lib/intel64)
    target_include_directories(data_collect PRIVATE $ENV{MKLROOT}/include)  
    target_sources(data_collect PRIVATE MKL-context.cpp)
    target_compile_definitions(data_collect PRIVATE USE_MKL)
    target_link_libraries(data_collect "-lmkl_intel_ilp64 -lmkl_gnu_thread -lmkl_core")
    # target_link_libraries(data_collect ${MKL_LIBRARIES}) # not working
	message("MKL_LIBRARIES: ${MKL_LIBRARIES}")
    set(BLAS_CONFIGURED 1)

  # Rahul old cmake code intel-mkl/2019

  # message("Looking for MKL...")
  # set(BLAS_VENDOR Intel10_64lp)
  # find_package(BLAS)
  # if (BLAS_FOUND)
  #   target_sources(data_collect PRIVATE MKL-context.cpp)
  #   target_compile_definitions(data_collect PRIVATE USE_MKL)
  #   target_link_libraries(data_collect ${BLAS_LIBRARIES})
  #   set(BLAS_CONFIGURED 1)


  else()
    message("Failed to find MKL!")
  endif()

elseif (BLASType STREQUAL "OpenBLAS")
  message("Looking for OpenBLAS...")
  find_library(OpenBLAS NAMES openblas PATHS "${OpenBLAS_DIR}/lib")
  if(OpenBLAS)
    target_include_directories(data_collect PRIVATE ${OpenBLAS_DIR}/include)
    target_link_libraries(data_collect ${OpenBLAS})
    target_sources(data_collect PRIVATE OpenBLAS-context.cpp)
    target_compile_definitions(data_collect PRIVATE USE_OPENBLAS)
    set(BLAS_CONFIGURED 1)
  else()
    message("Failed to find OpenBLAS!")
endif()

elseif (BLASType STREQUAL "AmdBLIS")
  message("Looking for AmdBLIS...")
  find_library(BLIS NAMES blis-mt)
  if(BLIS)
    target_include_directories(data_collect PRIVATE .)
    target_link_libraries(data_collect ${BLIS} pthread)
    target_sources(data_collect PRIVATE BLIS-context.cpp)
    target_compile_definitions(data_collect PRIVATE USE_BLIS)
    set(BLAS_CONFIGURED 1)
  else()
    message("Failed to find AmdBLIS!")
endif()

else()
  message("UNSUPPORTED BLAS LIBRARY GIVEN ${BLASType}")
  # find_package(BLAS)
  # if (BLAS_FOUND)
  #   target_sources(data_collect PRIVATE BLAS-context.cpp)
  #   target_compile_definitions(data_collect PRIVATE USE_BLAS)
  #   target_link_libraries(data_collect ${BLAS_LIBRARIES})
  # endif()
endif()

check_language(CUDA)
#set(CMAKE_CUDA_COMPILER "/usr/local/cuda-11.2/bin/nvcc")
# if (CMAKE_CUDA_COMPILER)
#   enable_language(CUDA)
#   target_sources(data_collect PRIVATE cuBLAS-context.cu)
#   target_compile_definitions(data_collect PRIVATE USE_CUBLAS)
#   target_link_libraries(data_collect -lcublas)
# endif()


add_executable(main main.cpp) 
target_link_libraries(main data_collect)
