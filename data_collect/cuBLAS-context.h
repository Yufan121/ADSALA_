#include "interfaces.h"

class cuBLAS_context : public test_class
{
  public:
    cuBLAS_context();
    ~cuBLAS_context();
    void dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C);
    void sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C);

  private:
    void *handle; // For the cublasHandle. Void pointer so we don't have to #include. Better way?
};
