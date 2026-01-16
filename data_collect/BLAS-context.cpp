#include "BLAS-context.h"
extern "C" {
  #include <cblas.h>
}

BLAS_context::BLAS_context() : library("blas", 1) {}
BLAS_context::~BLAS_context() {}

void BLAS_context::dgemm(int m, int k, int n, double alpha, double *A, double *B, double beta, double *C) {
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k, B, n, beta, C, n);
}
void BLAS_context::sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C){
   //TODO
}


