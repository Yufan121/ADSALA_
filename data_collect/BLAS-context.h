#include "interfaces.h"

class BLAS_context : public test_class
{
  public:
    BLAS_context();
    ~BLAS_context();
    void dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C);
    void sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C);

};
