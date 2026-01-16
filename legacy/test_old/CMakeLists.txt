# 指定CMake版本
cmake_minimum_required(VERSION 3.2)
 
# 指定项目名称
project(test)
 
# 判断编译器类型,如果是gcc编译器,则在编译选项中加入c++11支持
if(CMAKE_COMPILER_IS_GNUCC)
    message("COMPILER IS GNUCC")
    add_definitions ( -std=c++11 )
endif(CMAKE_COMPILER_IS_GNUCC)
  
# Include header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/library_frame/BLAS/include)
include_directories(${CMAKE_SOURCE_DIR}/library_frame/predictor)
include_directories(${CMAKE_SOURCE_DIR}/data_collect)


# 指定源文件目录
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
message(${CMAKE_CURRENT_SOURCE_DIR})
message(${SOURCE_FILES})
 
# 设置环境变量，编译用到的源文件全部都要放到这里，否则编译能够通过，
# 但是执行的时候会出现各种问题，比如"symbol lookup error xxxxx , undefined symbol"
set(ALL_SRCS ${SOURCE_FILES})
message(${ALL_SRCS})
 
# 添加该工程编译时所需的依赖库

# 依赖库所在路径
# link_directories(${CMAKE_BINARY_DIR}/BLAS)
link_directories(${CMAKE_BINARY_DIR}/data_collect)
link_directories(${CMAKE_BINARY_DIR}/library_frame/predictor)
link_directories(${CMAKE_BINARY_DIR}/BLAS)

# # 链接依赖库
# link_directories(${CMAKE_BINARY_DIR}/predictor)
# link_libraries(predictor)


# 添加该工程编译完成后的可执行文件
add_executable(${PROJECT_NAME} ${ALL_SRCS})
target_link_libraries(${PROJECT_NAME} predictor data_collect module)

#Link MKL_BLAS
# set(BLA_VENDOR Intel10_64lp)
# find_package(BLAS)
# if (BLAS_FOUND)
#     message("BLAS found")
# endif()

# target_link_libraries(${PROJECT_NAME} ${BLAS_LIBRARIES})
