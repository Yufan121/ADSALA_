#ifndef __INTERFACES_ML_H_INCLUDED__
#define __INTERFACES_ML_H_INCLUDED__

#include <string>
#include <vector>

class test_class;

class test_class_ml
{
  public:
    test_class_ml() {};
    virtual ~test_class_ml() {};

    // Level III
    long long run_compare_gemm_(int m, int k, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    long long run_compare_symm_(int m, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    long long run_compare_syrk_(int n, int k, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    long long run_compare_syr2k_(int n, int k, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    long long run_compare_trmm_(int m, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    long long run_compare_trsm_(int m, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    // Level II
    long long run_compare_gemv_(int m, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    long long run_compare_syr_(int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    long long run_compare_trsv_(int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    
    // Level I
    long long run_compare_dot_(int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
    long long run_compare_axpy_(int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib);
};


#endif
