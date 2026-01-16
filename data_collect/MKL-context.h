#include "interfaces.h"

class MKL_context : public test_class
{
  public:
    MKL_context();
    ~MKL_context();

    // LEVEL III
    // gemm
    void dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C);
    void sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C);
    // symm
    void dsymm(int m, int n, double alpha, double *A, double *B, double beta, double *C);
    void ssymm(int m, int n, float alpha, float *A, float *B, float beta, float *C);
    // syrk
    void dsyrk(int n, int k, double alpha, double *A, double beta, double *C);
    void ssyrk(int n, int k, float alpha, float *A, float beta, float *C);
    //syr2k
    void dsyr2k(int n, int k, double alpha, double *A, double *B, double beta, double *C);
    void ssyr2k(int n, int k, float alpha, float *A, float *B, float beta, float *C);
    // trmm
    void dtrmm(int m, int n, double alpha, double *A, double *B);
    void strmm(int m, int n, float alpha, float *A, float *B);
    // trsm
    void dtrsm(int m, int n, double alpha, double *A, double *B);
    void strsm(int m, int n, float alpha, float *A, float *B);

    // LEVEL II
    // gemv
    void dgemv(int m, int n, double alpha, double *A, double *x, double beta, double *y);
    void sgemv(int m, int n, float alpha, float *A, float *x, float beta, float *y);
    // syr
    void dsyr(int n, double alpha, double *x, double *A);
    void ssyr(int n, float alpha, float *x, float *A); 
    // trsv
    void dtrsv(int n, double *A, double *x);
    void strsv(int n, float *A, float *x);

    // LEVEL I
    // dot 
    void ddot(int n, double *x, double *y);
    void sdot(int n, float *x, float *y);
    // axpy
    void daxpy(int n, double alpha, double *x, double *y);
    void saxpy(int n, float alpha, float *x, float *y);

    void set_num_threads(int nt); 
};


