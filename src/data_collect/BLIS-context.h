#include "interfaces.h"

class BLIS_context : public BaseBLASContext<BLIS_context>
{
  public:
    BLIS_context();
    ~BLIS_context();

    // Implementation methods for CRTP base class
    void impl_dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C);
    void impl_sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C);
    void impl_dsymm(int m, int n, double alpha, double *A, double *B, double beta, double *C);
    void impl_ssymm(int m, int n, float alpha, float *A, float *B, float beta, float *C);
    void impl_dsyrk(int n, int k, double alpha, double *A, double beta, double *C);
    void impl_ssyrk(int n, int k, float alpha, float *A, float beta, float *C);
    void impl_dsyr2k(int n, int k, double alpha, double *A, double *B, double beta, double *C);
    void impl_ssyr2k(int n,int k,float alpha,float *A,float *B,float beta,float *C);
    void impl_dtrmm(int m,int n, double alpha, double *A, double *B);
    void impl_strmm(int m,int n, float alpha, float *A, float *B);
    void impl_dtrsm(int m,int n, double alpha, double *A, double *B);
    void impl_strsm(int m,int n, float alpha, float *A, float *B);
    void impl_dgemv(int m, int n, double alpha, double *A, double *x, double beta, double *y);
    void impl_sgemv(int m, int n, float alpha, float *A, float *x, float beta, float *y);
    void impl_dsyr(int n, double alpha, double *x, double *A);
    void impl_ssyr(int n, float alpha, float *x, float *A);
    void impl_dtrsv(int n, double *A, double *x);
    void impl_strsv(int n, float *A, float *x);
    void impl_ddot(int n, double *x, double *y);
    void impl_sdot(int n, float *x, float *y);
    void impl_daxpy(int n, double alpha, double *x, double *y);
    void impl_saxpy(int n, float alpha, float *x, float *y);

    void set_num_threads(int nt);
};
