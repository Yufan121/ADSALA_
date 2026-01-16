cmake_minimum_required(VERSION 3.10)
project(ADSALA VERSION 0.0)

SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

# find_program(INTEL_COMPILER icc)
# if(INTEL_COMPILER)
#     include(CMakeForceCompiler)
#     cmake_force_c_compiler(icx "Intel C Compiler")
#     cmake_force_cxx_compiler(icx "Intel C++ Compiler")
# endif(INTEL_COMPILER)

# set BLAS library
if ( NOT BLASType )
	  # detect CPU vendor, if Intel, use MKL, if AMD, use BLIS
	    execute_process(COMMAND lscpu OUTPUT_VARIABLE CPU_INFO)
	      string(REGEX MATCH "Vendor ID:[ ]*([^\n]*)" CPU_VENDOR_ID ${CPU_INFO})
	        string(REGEX MATCH "GenuineIntel" CPU_VENDOR_ID ${CPU_VENDOR_ID})
		  if (CPU_VENDOR_ID)
			      SET(BLASType "MKL") # auto detect for intel not tested
			        else()
					    SET(BLASType "AmdBLIS")
					      endif() 
					        message("CPU_VENDOR_ID: ${CPU_VENDOR_ID}")

					endif() 

					# include data_collect and library_frame as two subdirectories

					add_subdirectory(${PROJECT_SOURCE_DIR}/src/data_collect)
					add_subdirectory(${PROJECT_SOURCE_DIR}/src/library_frame)

					# Enable testing if requested
					option(BUILD_TESTS "Build test suite" OFF)
					if(BUILD_TESTS)
					    enable_testing()
					    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
					endif()


