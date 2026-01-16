#ifndef __INTERFACES_H_INCLUDED__
#define __INTERFACES_H_INCLUDED__

#include <string>
#include <vector>

class library
{
  public:
    library(std::string, int);
    library(std::string);
    library();
    virtual ~library();
    std::string name;
    const int max_threads; 
    virtual void set_num_threads(int);
};


class test_class : virtual public library // virtual inheritance means that only one copy of library is inherited by derived class
{
  // TODO: add ops (trans/notrans), row/column major, type templates
  public:
    // LEVEL III
    // gemm
    virtual void dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C) = 0;
    virtual void sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C) = 0;
    // symm
    virtual void dsymm(int m, int n, double alpha, double *A, double *B, double beta, double *C) = 0;
    virtual void ssymm(int m, int n, float alpha, float *A, float *B, float beta, float *C) = 0;
    // syrk
    virtual void dsyrk(int n, int k, double alpha, double *A, double beta, double *C) = 0;
    virtual void ssyrk(int n, int k, float alpha, float *A, float beta, float *C) = 0;
    // syr2k
    virtual void dsyr2k(int n, int k, double alpha, double *A, double *B, double beta, double *C) = 0;
    virtual void ssyr2k(int n,int k,float alpha,float *A,float *B,float beta,float *C) = 0;
    // trmm
    virtual void dtrmm(int m,int n, double alpha, double *A, double *B) = 0;
    virtual void strmm(int m,int n, float alpha, float *A, float *B) = 0;
    // trsm
    virtual void dtrsm(int m,int n, double alpha, double *A, double *B) = 0;
    virtual void strsm(int m,int n, float alpha, float *A, float *B) = 0;

    // LEVEL II  
    // gemv
    virtual void dgemv(int m, int n, double alpha, double *A, double *x, double beta, double *y) = 0;
    virtual void sgemv(int m, int n, float alpha, float *A, float *x, float beta, float *y) = 0;
    // syr
    virtual void dsyr(int n, double alpha, double *x, double *A) = 0;
    virtual void ssyr(int n, float alpha, float *x, float *A) = 0; // =0 means pure virtual, mush be implemented in derived class
    // trsv
    virtual void dtrsv(int n, double *A, double *x) = 0;
    virtual void strsv(int n, float *A, float *x) = 0;

    // LEVEL I
    // dot product
    virtual void ddot(int n, double *x, double *y) = 0;
    virtual void sdot(int n, float *x, float *y) = 0;
    // axpy
    virtual void daxpy(int n, double alpha, double *x, double *y) = 0;
    virtual void saxpy(int n, float alpha, float *x, float *y) = 0;

    std::vector<long long> test_gemm(int m, int n, int k, int nt, bool isDouble, int num_of_duplicate);
    std::vector<long long> test_symm(int m, int n, int nt, bool isDouble, int num_of_duplicate); 
    std::vector<long long> test_syrk(int n, int k, int nt, bool isDouble, int num_of_duplicate);
    std::vector<long long> test_syr2k(int n, int k, int nt, bool isDouble, int num_of_duplicate);
    std::vector<long long> test_trmm(int m, int n, int nt, bool isDouble, int num_of_duplicate);
    std::vector<long long> test_trsm(int m, int n, int nt, bool isDouble, int num_of_duplicate);
     

    std::vector<long long> test_gemv(int m, int n, int nt, bool isDouble, int num_of_duplicate);
    std::vector<long long> test_syr(int n, int nt, bool isDouble, int num_of_duplicate);
    std::vector<long long> test_trsv(int n, int nt, bool isDouble, int num_of_duplicate); 

    std::vector<long long> test_dot(int n, int nt, bool isDouble, int num_of_duplicate);
    std::vector<long long> test_axpy(int n, int nt, bool isDouble, int num_of_duplicate);
 
    virtual ~test_class();
};


#endif
