# ADSALA GEMM

### A Brief Introduction

The GEneral Matrix Multiplication (GEMM) is
one of the most important algorithms in scientific computing.
However, due to the complexity of modern multi-core shared
memory systems, it is challenging to determine the number of
threads that minimize the GEMM runtime.

This project presents a proof-of-concept approach to building an
Architecture and Data-Structure Aware Linear Algebra (AD-
SALA) software library that uses Machine Learning to optimize
the runtime performance of BLAS routines. The ADSALA
GEMM implementation automatically selects the best number
of threads for a given GEMM task in multi-core shared-
memory machines, thereby achieving better execution times
than directly running with a number of threads equal to the
the number of available CPU cores. Results obtained on a double-
socket Intel Cascade Lake node, reveal a 30 percent speedup on
average compared to the commonly-adopted one-thread-per-core
execution configuration for matrices smaller than 100 MB.

### Training process:

[Installation Process.pdf](https://github.com/gbarca/ADSALA/files/8562348/Installation.Process.pdf)

### Runtime Process:

[Class Structure.pdf](https://github.com/gbarca/ADSALA/files/8562353/Class.Structure.pdf)

### Code Structure

- Data collection code is in `data_collect` folder. To compile, go to build folder and execute `cmake ..` then `make` to compile the training data collection program.
- Model training code is in `train_model` folder. This is in python code, training data need to be manually moved from `data_collect` 
- Library of implementation is in `library_frame` folder. To compile, go to build folder and execute `cmake ..`; then `make` to compile the dynamic library and the testing program.

### Run Experiments

Please go to `data_collect` and see the illustrations.

### Please use ` git clone --recurse-submodules <url>` to ensure the submodule is also pulled


!Visualization of the codebase

