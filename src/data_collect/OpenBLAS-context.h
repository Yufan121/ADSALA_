#include "interfaces.h"

class OpenBLAS_context : public BaseBLASContext<OpenBLAS_context>
{
  public:
    OpenBLAS_context();
    ~OpenBLAS_context();

    // Only implement gemm methods for now (incomplete implementation)
    void impl_dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C);
    void impl_sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C);

    // TODO: Implement all other BLAS operations to match the interface
    // For now, these will cause compilation errors if called
    void impl_dsymm(int m, int n, double alpha, double *A, double *B, double beta, double *C) { /* TODO */ }
    void impl_ssymm(int m, int n, float alpha, float *A, float *B, float beta, float *C) { /* TODO */ }
    void impl_dsyrk(int n, int k, double alpha, double *A, double beta, double *C) { /* TODO */ }
    void impl_ssyrk(int n, int k, float alpha, float *A, float beta, float *C) { /* TODO */ }
    void impl_dsyr2k(int n, int k, double alpha, double *A, double *B, double beta, double *C) { /* TODO */ }
    void impl_ssyr2k(int n,int k,float alpha,float *A,float *B,float beta,float *C) { /* TODO */ }
    void impl_dtrmm(int m,int n, double alpha, double *A, double *B) { /* TODO */ }
    void impl_strmm(int m,int n, float alpha, float *A, float *B) { /* TODO */ }
    void impl_dtrsm(int m,int n, double alpha, double *A, double *B) { /* TODO */ }
    void impl_strsm(int m,int n, float alpha, float *A, float *B) { /* TODO */ }
    void impl_dgemv(int m, int n, double alpha, double *A, double *x, double beta, double *y) { /* TODO */ }
    void impl_sgemv(int m, int n, float alpha, float *A, float *x, float beta, float *y) { /* TODO */ }
    void impl_dsyr(int n, double alpha, double *x, double *A) { /* TODO */ }
    void impl_ssyr(int n, float alpha, float *x, float *A) { /* TODO */ }
    void impl_dtrsv(int n, double *A, double *x) { /* TODO */ }
    void impl_strsv(int n, float *A, float *x) { /* TODO */ }
    void impl_ddot(int n, double *x, double *y) { /* TODO */ }
    void impl_sdot(int n, float *x, float *y) { /* TODO */ }
    void impl_daxpy(int n, double alpha, double *x, double *y) { /* TODO */ }
    void impl_saxpy(int n, float alpha, float *x, float *y) { /* TODO */ }

    void set_num_threads(int nt);
};