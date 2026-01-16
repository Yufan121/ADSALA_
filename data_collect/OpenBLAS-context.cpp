#include "OpenBLAS-context.h"
extern "C" {
  #include <cblas.h>
}

OpenBLAS_context::OpenBLAS_context() : library("openblas") {}
OpenBLAS_context::~OpenBLAS_context() {}

void OpenBLAS_context::dgemm(int m, int k, int n, double alpha, double *A, double *B, double beta, double *C) {
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k, B, n, beta, C, n);
}

void OpenBLAS_context::sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C){
   //TODO
}

void OpenBLAS_context::set_num_threads(int nt) {
  openblas_set_num_threads(nt);
}
