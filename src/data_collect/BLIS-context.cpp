#include "BLIS-context.h"
// #include "cblas.h"
#include "blis.h"
#include <iostream>
#include <string>

#include <omp.h>
#include "get-ncores.h"



// set max threads automatically
BLIS_context::BLIS_context() : library("blis", 128*2) {
  get_ncores();
}
BLIS_context::~BLIS_context() {}


/* Level III */

// gemm
// void dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C);
// void sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C);
// // symm
// void dsymm(int m, int n, double alpha, double *A, double *B, double beta, double *C);
// void ssymm(int m, int n, float alpha, float *A, float *B, float beta, float *C);
// // syrk
// void dsyrk(int n, int k, double alpha, double *A, double beta, double *C);
// void ssyrk(int n, int k, float alpha, float *A, float beta, float *C);
// //syr2k
// void dsyr2k(int n, int k, double alpha, double *A, double *B, double beta, double *C);
// void ssyr2k(int n, int k, float alpha, float *A, float *B, float beta, float *C);
// // trmm
// void dtrmm(int m, int n, double alpha, double *A, double *B);
// void strmm(int m, int n, float alpha, float *A, float *B);
// // trsm
// void dtrsm(int m, int n, double alpha, double *A, double *B);
// void strsm(int m, int n, float alpha, float *A, float *B);

//gemm
void BLIS_context::impl_dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C) {
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k, B, n, beta, C, n);
}
void BLIS_context::impl_sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C) {
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k, B, n, beta, C, n);
}

// symm
void BLIS_context::impl_dsymm(int m, int n, double alpha, double *A, double *B, double beta, double *C) {
  cblas_dsymm(CblasRowMajor, CblasLeft, CblasUpper, m, n, alpha, A, m, B, n, beta, C, n);
}
void BLIS_context::impl_ssymm(int m, int n, float alpha, float *A, float *B, float beta, float *C) {
  cblas_ssymm(CblasRowMajor, CblasLeft, CblasUpper, m, n, alpha, A, m, B, n, beta, C, n);
}

// syrk
void BLIS_context::impl_dsyrk(int n, int k, double alpha, double *A, double beta, double *C) {
  cblas_dsyrk(CblasRowMajor, CblasUpper, CblasNoTrans, n, k, alpha, A, k, beta, C, n);
}
void BLIS_context::impl_ssyrk(int n, int k, float alpha, float *A, float beta, float *C) {
    cblas_ssyrk(CblasRowMajor, CblasUpper, CblasNoTrans, n, k, alpha, A, k, beta, C, n);
}

//syr2k
void BLIS_context::impl_dsyr2k(int n, int k, double alpha, double *A, double *B, double beta, double *C) {
  cblas_dsyr2k(CblasRowMajor, CblasUpper, CblasNoTrans, n, k, alpha, A, k, B, k, beta, C, n);
}
void BLIS_context::impl_ssyr2k(int n, int k, float alpha, float *A, float *B, float beta, float *C) {
    cblas_ssyr2k(CblasRowMajor, CblasUpper, CblasNoTrans, n, k, alpha, A, k, B, k, beta, C, n);
}

// trmm
void BLIS_context::impl_dtrmm(int m, int n, double alpha, double *A, double *B) {
  cblas_dtrmm(CblasRowMajor, CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, m, n, alpha, A, m, B, n);
}
void BLIS_context::impl_strmm(int m, int n, float alpha, float *A, float *B) {
    cblas_strmm(CblasRowMajor, CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, m, n, alpha, A, m, B, n);
}

// trsm
void BLIS_context::impl_dtrsm(int m, int n, double alpha, double *A, double *B) {
  cblas_dtrsm(CblasRowMajor, CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, m, n, alpha, A, m, B, n);
}
void BLIS_context::impl_strsm(int m, int n, float alpha, float *A, float *B) {
    cblas_strsm(CblasRowMajor, CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, m, n, alpha, A, m, B, n);
}

/* Level II */


// gemv
void BLIS_context::impl_dgemv(int m, int n, double alpha, double *A, double *x, double beta, double *y) {
  cblas_dgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, A, n, x, 1, beta, y, 1);
}
void BLIS_context::impl_sgemv(int m, int n, float alpha, float *A, float *x, float beta, float *y) {
    cblas_sgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, A, n, x, 1, beta, y, 1);
}

// void BLIS_context::dgemv(int m, int n, double alpha, double *A, double *x, double beta, double *y) {
//   cblas_dgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, A, n, x, 1, beta, y, 1);  // row major A matrix has leading dim == n
// }
// syr
void BLIS_context::impl_dsyr(int n, double alpha, double *x, double *A) {
  cblas_dsyr(CblasRowMajor, CblasUpper, n, alpha, x, 1, A, n);
}
void BLIS_context::impl_ssyr(int n, float alpha, float *x, float *A) {
    cblas_ssyr(CblasRowMajor, CblasUpper, n, alpha, x, 1, A, n);
}

// trsv
void BLIS_context::impl_dtrsv(int n, double *A, double *x) {
  cblas_dtrsv(CblasRowMajor, CblasUpper, CblasNoTrans, CblasNonUnit, n, A, n, x, 1);
}
void BLIS_context::impl_strsv(int n, float *A, float *x) {
    cblas_strsv(CblasRowMajor, CblasUpper, CblasNoTrans, CblasNonUnit, n, A, n, x, 1);
}

/* Level I */


// dot product
void BLIS_context::impl_ddot(int n, double *x, double *y) {
  cblas_ddot(n, x, 1, y, 1);
}
void BLIS_context::impl_sdot(int n, float *x, float *y) {
  cblas_sdot(n, x, 1, y, 1);
}

// axpy
void BLIS_context::impl_daxpy(int n, double alpha, double *x, double *y) {
  cblas_daxpy(n, alpha, x, 1, y, 1);
}
void BLIS_context::impl_saxpy(int n, float alpha, float *x, float *y) {
  cblas_saxpy(n, alpha, x, 1, y, 1);
}


void BLIS_context::set_num_threads(int nt) {
  bli_thread_set_num_threads(nt);
  //omp_set_num_threads(nt);
}
