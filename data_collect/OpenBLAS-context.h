#include "interfaces.h"

class OpenBLAS_context : public gemm_int
{
  public:
    OpenBLAS_context();
    ~OpenBLAS_context();
    void dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C);
    void sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C);
    void set_num_threads(int nt); 
};