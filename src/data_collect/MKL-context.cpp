#include "MKL-context.h"
#include <iostream>
#include <string>

extern "C" {
  #include <mkl_cblas.h>
  #include <mkl.h>
}
#include <omp.h>

MKL_context::MKL_context() : library("mkl", 48) {
  // get_ncores();
}
MKL_context::~MKL_context() {}

/* Level III */

// gemm

void MKL_context::impl_dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C) {
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k, B, n, beta, C, n);
}
void MKL_context::impl_sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C) {
  // print max threads
  // mkl_set_dynamic(0);
  // std::cout << "max threads: " << mkl_get_max_threads() << std::endl;
  // mkl_set_num_threads_local(10);
  // std::cout << "max threads after set: " << mkl_get_max_threads() << std::endl;
  // set omp threads

  // std::cout << "omp threads: " << omp_get_max_threads() << std::endl;
  // omp_set_num_threads(10);
  // std::cout << "omp threads after set: " << omp_get_max_threads() << std::endl;
  cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k, B, n, beta, C, n);
}

// symm
void MKL_context::impl_dsymm(int m, int n, double alpha, double *A, double *B, double beta, double *C) {
  cblas_dsymm(CblasRowMajor, CblasLeft, CblasUpper, m, n, alpha, A, m, B, n, beta, C, n);
}
void MKL_context::impl_ssymm(int m, int n, float alpha, float *A, float *B, float beta, float *C) {
  cblas_ssymm(CblasRowMajor, CblasLeft, CblasUpper, m, n, alpha, A, m, B, n, beta, C, n);
}

// syrk
void MKL_context::impl_dsyrk(int n, int k, double alpha, double *A, double beta, double *C) {
  cblas_dsyrk(CblasRowMajor, CblasUpper, CblasNoTrans, n, k, alpha, A, k, beta, C, n);
}
void MKL_context::impl_ssyrk(int n, int k, float alpha, float *A, float beta, float *C) {
  cblas_ssyrk(CblasRowMajor, CblasUpper, CblasNoTrans, n, k, alpha, A, k, beta, C, n);
}

//syr2k
void MKL_context::impl_dsyr2k(int n, int k, double alpha, double *A, double *B, double beta, double *C) {
  cblas_dsyr2k(CblasRowMajor, CblasUpper, CblasNoTrans, n, k, alpha, A, k, B, k, beta, C, n);
}
void MKL_context::impl_ssyr2k(int n, int k, float alpha, float *A, float *B, float beta, float *C) {
  cblas_ssyr2k(CblasRowMajor, CblasUpper, CblasNoTrans, n, k, alpha, A, k, B, k, beta, C, n);
}

// trmm
void MKL_context::impl_dtrmm(int m, int n, double alpha, double *A, double *B) {
  cblas_dtrmm(CblasRowMajor, CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, m, n, alpha, A, m, B, n);
}
void MKL_context::impl_strmm(int m, int n, float alpha, float *A, float *B) {
  cblas_strmm(CblasRowMajor, CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, m, n, alpha, A, m, B, n);
}

// trsm
void MKL_context::impl_dtrsm(int m, int n, double alpha, double *A, double *B) {
  cblas_dtrsm(CblasRowMajor, CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, m, n, alpha, A, m, B, n);
}
void MKL_context::impl_strsm(int m, int n, float alpha, float *A, float *B) {
  cblas_strsm(CblasRowMajor, CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, m, n, alpha, A, m, B, n);
}


/* Level II */

// gemv
void MKL_context::impl_dgemv(int m, int n, double alpha, double *A, double *x, double beta, double *y) {
  cblas_dgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, A, n, x, 1, beta, y, 1);
}
void MKL_context::impl_sgemv(int m, int n, float alpha, float *A, float *x, float beta, float *y) {
  cblas_sgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, A, n, x, 1, beta, y, 1); 
} 

// void MKL_context::impl_dgemv(int m, int n, double alpha, double *A, double *x, double beta, double *y) {
//   cblas_dgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, A, n, x, 1, beta, y, 1);  // row major A matrix has leading dim == n
// }

// syr
void MKL_context::impl_dsyr(int n, double alpha, double *x, double *A) {
  cblas_dsyr(CblasRowMajor, CblasUpper, n, alpha, x, 1, A, n);
}
void MKL_context::impl_ssyr(int n, float alpha, float *x, float *A) {
  cblas_ssyr(CblasRowMajor, CblasUpper, n, alpha, x, 1, A, n);
}

// trsv
void MKL_context::impl_dtrsv(int n, double *A, double *x) {
  cblas_dtrsv(CblasRowMajor, CblasUpper, CblasNoTrans, CblasNonUnit, n, A, n, x, 1);
}
void MKL_context::impl_strsv(int n, float *A, float *x) {
  cblas_strsv(CblasRowMajor, CblasUpper, CblasNoTrans, CblasNonUnit, n, A, n, x, 1);
}



/* Level I */


// dot 
void MKL_context::impl_ddot(int n, double *x, double *y) {
  cblas_ddot(n, x, 1, y, 1);
}
void MKL_context::impl_sdot(int n, float *x, float *y) {
  cblas_sdot(n, x, 1, y, 1);
}

// axpy
void MKL_context::impl_daxpy(int n, double alpha, double *x, double *y) {
  cblas_daxpy(n, alpha, x, 1, y, 1);
}
void MKL_context::impl_saxpy(int n, float alpha, float *x, float *y) {
  cblas_saxpy(n, alpha, x, 1, y, 1);
}



void MKL_context::impl_set_num_threads(int nt) {
  mkl_set_num_threads(nt);
  //omp_set_num_threads(nt);
}
