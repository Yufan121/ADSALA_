#include "cuBLAS-context.h"
#include <cublas_v2.h>
#include <stdio.h>
#include <iostream>
#include <chrono>

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

cuBLAS_context::cuBLAS_context() : library("cublas", 1) { cublasCreate((cublasHandle_t *)&handle); }
cuBLAS_context::~cuBLAS_context() { cublasDestroy((cublasHandle_t)handle); }

void cuBLAS_context::dgemm(int m, int k, int n, double alpha, double *A, double *B, double beta, double *C) {
  double *d_A, *d_B, *d_C;
  std::chrono::high_resolution_clock::time_point t1, t2;
  gpuErrchk(cudaMalloc(&d_A, m*k*sizeof(double)));
  gpuErrchk(cudaMalloc(&d_B, k*n*sizeof(double)));
  gpuErrchk(cudaMalloc(&d_C, m*n*sizeof(double)));

  t1 = std::chrono::high_resolution_clock::now();
  gpuErrchk(cudaMemcpy(d_A, A, m*k*sizeof(double), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_B, B, k*n*sizeof(double), cudaMemcpyHostToDevice));
  // if (beta != 0.0)
  gpuErrchk(cudaMemcpy(d_C, C, m*n*sizeof(double), cudaMemcpyHostToDevice));

  cublasDgemm((cublasHandle_t)handle, CUBLAS_OP_N, CUBLAS_OP_N, n, m, k, &alpha, d_B, n, d_A, k, &beta, d_C, n);
  cudaDeviceSynchronize();

  cudaMemcpy(C, d_C, m*n*sizeof(double), cudaMemcpyDeviceToHost);
  t2 = std::chrono::high_resolution_clock::now();//how does time it?
  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() << std::endl;
  cudaFree(d_A);
  cudaFree(d_B);
  cudaFree(d_C);
}

void cuBLAS_context::sgemm(int m, int k, int n, float alpha, float *A, float *B, float beta, float *C){
   //TODO
   // double *d_A, *d_B, *d_C;
   // std::chrono::high_resolution_clock::time_point t1, t2;
   // gpuErrchk(cudaMalloc(&d_A, m*k*sizeof(double)));
   // gpuErrchk(cudaMalloc(&d_B, k*n*sizeof(double)));
   // gpuErrchk(cudaMalloc(&d_C, m*n*sizeof(double)));

   // t1 = std::chrono::high_resolution_clock::now();
   // gpuErrchk(cudaMemcpy(d_A, A, m*k*sizeof(double), cudaMemcpyHostToDevice));
   // gpuErrchk(cudaMemcpy(d_B, B, k*n*sizeof(double), cudaMemcpyHostToDevice));
   // // if (beta != 0.0)
   // gpuErrchk(cudaMemcpy(d_C, C, m*n*sizeof(double), cudaMemcpyHostToDevice));

   // cublasDgemm((cublasHandle_t)handle, CUBLAS_OP_N, CUBLAS_OP_N, n, m, k, &alpha, d_B, n, d_A, k, &beta, d_C, n);
   // cudaDeviceSynchronize();

   // cudaMemcpy(C, d_C, m*n*sizeof(double), cudaMemcpyDeviceToHost);
   // t2 = std::chrono::high_resolution_clock::now();
   // std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() << std::endl;
   // cudaFree(d_A);
   // cudaFree(d_B);
   // cudaFree(d_C);

}


