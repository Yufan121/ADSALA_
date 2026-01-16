#include "interfaces.h"
#include "interfaces-ml.h"
#include "interfaces.cpp"
#include "blas_exceptions.h"
#include "blas_config.h"
#include <thread>
#include <unistd.h>
#include <omp.h>
#include <cstdlib>
#include <iostream>
#include "get-ncores.h"
#include <malloc.h>
#include <vector>
#include <pthread.h>
#include <type_traits>
#include "predictor.hpp"
// #include <mlpack.hpp>

// #include <papi.h>
using namespace std;

// Helper function to get BLAS routine name prefix based on type
template<typename T>
const char* get_blas_prefix();

template<>
const char* get_blas_prefix<double>() { return "d"; }

template<>
const char* get_blas_prefix<float>() { return "s"; }

// Helper function to get type character for fill_with_rand
template<typename T>
char get_type_char();

template<>
char get_type_char<double>() { return 'd'; }

template<>
char get_type_char<float>() { return 'f'; }

// Template implementation for GEMM
template<typename T>
long long run_compare_gemm_impl(int m, int k, int n, bool useML, size_t num_of_duplicate, test_class* lib) {
  std::chrono::high_resolution_clock::time_point t1, t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1, t_2, t_3, t_4, t_5;
  int64_t time1, time2, time3, time4;

  std::vector<long long> time_vect;
  time_vect.reserve(num_of_duplicate);

    t_1 = std::chrono::high_resolution_clock::now(); 

  //assign space using std::vector for automatic memory management
  std::vector<T> A(static_cast<size_t>(num_of_duplicate) * m * k);
  std::vector<T> B(static_cast<size_t>(num_of_duplicate) * k * n);
  std::vector<T> C(static_cast<size_t>(num_of_duplicate) * m * n);

    std::cout << "Trying to alloc GB:" << std::endl;  
  std::cout << (((unsigned long long)m)*k + n*k + m*n)*sizeof(T) * num_of_duplicate / 1e9 << std::endl;
  if (A.empty() || B.empty()) {
      std::cout << "Trying to alloc:" << std::endl;  
    std::cout << static_cast<size_t>(num_of_duplicate)*m*k*sizeof(T) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A
  int num_threads = blas_config::Config::getInstance().getNumThreads();
  if (num_threads > 1024) num_threads = 1024; // Safety limit
  pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
  size_t stride = static_cast<size_t>(num_of_duplicate)*m*k / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = A.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*m*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }

    // fill B
  stride = static_cast<size_t>(num_of_duplicate)*n*k   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = B.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  
    // Use predictor to set threads
  std::string routine_name = std::string(get_blas_prefix<T>()) + "gemm";
  Predictor predictor("xgb", routine_name);
    int nt;
    if (useML) {
    nt = predictor.model->predict_num_cores(m, k, n, routine_name);
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    std::cout << "Number of threads: " << nt << std::endl;

  // Call appropriate BLAS function based on type
  for (int i = 0; i < num_of_duplicate; i++) {
      t1 = std::chrono::high_resolution_clock::now(); 

    if constexpr (std::is_same_v<T, double>) {
      lib->dgemm(m, k, n, 1.0, &A[static_cast<size_t>(i)*m*k], &B[static_cast<size_t>(i)*k*n], 0.0, &C[static_cast<size_t>(i)*m*n]);
    } else {
      lib->sgemm(m, k, n, 1.0f, &A[static_cast<size_t>(i)*m*k], &B[static_cast<size_t>(i)*k*n], 0.0f, &C[static_cast<size_t>(i)*m*n]);
    }

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 
    t_5 = std::chrono::high_resolution_clock::now(); 

  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  

  // return average of time_vect
  long long sum = 0;
  for (size_t i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();
}




long long test_class_ml::run_compare_gemm_(int m, int k, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  if (isDouble) {
    return run_compare_gemm_impl<double>(m, k, n, useML, num_of_duplicate, lib);
    } else {
    return run_compare_gemm_impl<float>(m, k, n, useML, num_of_duplicate, lib);
  }
}

// Template implementation for SYMM
template<typename T>
long long run_compare_symm_impl(int m, int n, bool useML, size_t num_of_duplicate, test_class* lib) {
  std::chrono::high_resolution_clock::time_point t1, t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1, t_2, t_3, t_4, t_5;
  int64_t time1, time2, time3, time4;

  std::vector<long long> time_vect;
  time_vect.reserve(num_of_duplicate);

    t_1 = std::chrono::high_resolution_clock::now(); 

  std::vector<T> A(static_cast<size_t>(num_of_duplicate) * m * m);
  std::vector<T> B(static_cast<size_t>(num_of_duplicate) * m * n);
  std::vector<T> C(static_cast<size_t>(num_of_duplicate) * m * n);

    std::cout << "Trying to alloc GB:" << std::endl;  
  std::cout << (((unsigned long long)m)*m + m*n + m*n)*sizeof(T) * num_of_duplicate / 1e9 << std::endl;
  if (A.empty() || B.empty()) {
      std::cout << "Trying to alloc:" << std::endl;  
    std::cout << static_cast<size_t>(num_of_duplicate)*m*m*sizeof(T) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill A (symmetric)
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
    size_t total_elements = m * (m + 1) / 2;
    size_t stride = total_elements   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg_symm *arg_ptr = (struct t_arg_symm *)malloc(sizeof(struct t_arg_symm));
    arg_ptr->M = A.data();
        arg_ptr->m = m;
        size_t start = i * stride;
        size_t end = (i == num_threads - 1) ? total_elements : (i + 1) * stride;
        arg_ptr->row_start = static_cast<size_t>(sqrt(2 * start + 0.25) - 0.5);
        arg_ptr->col_start = start - arg_ptr->row_start * (arg_ptr->row_start + 1) / 2;
        arg_ptr->row_end = static_cast<size_t>(sqrt(2 * end + 0.25) - 0.5);
        arg_ptr->col_end = end - arg_ptr->row_end * (arg_ptr->row_end + 1) / 2;
    arg_ptr->type = get_type_char<T>();
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_symmetric, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }

    // fill B
  stride = static_cast<size_t>(num_of_duplicate)*m*n   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = B.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  
  std::string routine_name = std::string(get_blas_prefix<T>()) + "symm";
  Predictor predictor("xgb", routine_name);
    int nt;
    if (useML) {
    nt = predictor.model->predict_num_cores(m, n, routine_name);
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    std::cout << "Number of threads: " << nt << std::endl;

  for (int i = 0; i < num_of_duplicate; i++) {
      t1 = std::chrono::high_resolution_clock::now(); 
    if constexpr (std::is_same_v<T, double>) {
      lib->dsymm(m, n, 1.0, &A[static_cast<size_t>(i)*m*m], &B[static_cast<size_t>(i)*m*n], 0.0, &C[static_cast<size_t>(i)*m*n]);
    } else {
      lib->ssymm(m, n, 1.0f, &A[static_cast<size_t>(i)*m*m], &B[static_cast<size_t>(i)*m*n], 0.0f, &C[static_cast<size_t>(i)*m*n]);
    }
      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 
    t_5 = std::chrono::high_resolution_clock::now(); 

  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  

  long long sum = 0;
  for (size_t i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();
}

long long test_class_ml::run_compare_symm_(int m, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  if (isDouble) {
    return run_compare_symm_impl<double>(m, n, useML, num_of_duplicate, lib);
  } else {
    return run_compare_symm_impl<float>(m, n, useML, num_of_duplicate, lib);
  }
}

// Template implementation for SYRK
template<typename T>
long long run_compare_syrk_impl(int n, int k, bool useML, size_t num_of_duplicate, test_class* lib) {
  std::chrono::high_resolution_clock::time_point t1, t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1, t_2, t_3, t_4, t_5;
  int64_t time1, time2, time3, time4;

  std::vector<long long> time_vect;
  time_vect.reserve(num_of_duplicate);

    t_1 = std::chrono::high_resolution_clock::now(); 

  std::vector<T> A(static_cast<size_t>(num_of_duplicate) * n * k);
  std::vector<T> C(static_cast<size_t>(num_of_duplicate) * n * n);

    std::cout << "Trying to alloc GB:" << std::endl;  
  std::cout << (((unsigned long long)n)*n + n*k)*sizeof(T) * num_of_duplicate / 1e9 << std::endl;
  if (A.empty() || C.empty()) {
      std::cout << "Trying to alloc:" << std::endl;  
    std::cout << static_cast<size_t>(num_of_duplicate)*n*n*sizeof(T) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill A 
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
  size_t stride = static_cast<size_t>(num_of_duplicate)*n*k   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = A.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }

    // fill C
  stride = static_cast<size_t>(num_of_duplicate)*n*n   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = C.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*n*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  
  std::string routine_name = std::string(get_blas_prefix<T>()) + "syrk";
  Predictor predictor("xgb", routine_name);
    int nt;
    if (useML) {
    nt = predictor.model->predict_num_cores(n, k, routine_name);
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    std::cout << "Number of threads: " << nt << std::endl;

  for (int i = 0; i < num_of_duplicate; i++) {
      t1 = std::chrono::high_resolution_clock::now(); 
    if constexpr (std::is_same_v<T, double>) {
      lib->dsyrk(n, k, 1.0, &A[static_cast<size_t>(i)*n*k], 0.0, &C[static_cast<size_t>(i)*n*n]);
    } else {
      lib->ssyrk(n, k, 1.0f, &A[static_cast<size_t>(i)*n*k], 0.0f, &C[static_cast<size_t>(i)*n*n]);
    }
      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 
    t_5 = std::chrono::high_resolution_clock::now(); 

  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  

  long long sum = 0;
  for (size_t i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();
}

long long test_class_ml::run_compare_syrk_(int n, int k, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  if (isDouble) {
    return run_compare_syrk_impl<double>(n, k, useML, num_of_duplicate, lib);
  } else {
    return run_compare_syrk_impl<float>(n, k, useML, num_of_duplicate, lib);
  }
}

// Template implementation for SYR2K
template<typename T>
long long run_compare_syr2k_impl(int n, int k, bool useML, size_t num_of_duplicate, test_class* lib) {
  std::chrono::high_resolution_clock::time_point t1, t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1, t_2, t_3, t_4, t_5;
  int64_t time1, time2, time3, time4;

  std::vector<long long> time_vect;
  time_vect.reserve(num_of_duplicate);

    t_1 = std::chrono::high_resolution_clock::now(); 

  std::vector<T> A(static_cast<size_t>(num_of_duplicate) * n * k);
  std::vector<T> B(static_cast<size_t>(num_of_duplicate) * n * k);
  std::vector<T> C(static_cast<size_t>(num_of_duplicate) * n * n);

    std::cout << "Trying to alloc GB:" << std::endl;  
  std::cout << (((unsigned long long)n)*n + 2*n*k)*sizeof(T) * num_of_duplicate / 1e9 << std::endl;
  if (A.empty() || C.empty()) {
      std::cout << "Trying to alloc:" << std::endl;  
    std::cout << static_cast<size_t>(num_of_duplicate)*n*n*sizeof(T) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill A 
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
  size_t stride = static_cast<size_t>(num_of_duplicate)*n*k   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = A.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }

    // fill B
  stride = static_cast<size_t>(num_of_duplicate)*n*k   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = B.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }

    // fill C
  stride = static_cast<size_t>(num_of_duplicate)*n*n   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = C.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*n*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  
  std::string routine_name = std::string(get_blas_prefix<T>()) + "syr2k";
  Predictor predictor("xgb", routine_name);
    int nt;
    if (useML) {
    nt = predictor.model->predict_num_cores(n, k, routine_name);
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    std::cout << "Number of threads: " << nt << std::endl;

  for (int i = 0; i < num_of_duplicate; i++) {
      t1 = std::chrono::high_resolution_clock::now(); 
    if constexpr (std::is_same_v<T, double>) {
      lib->dsyr2k(n, k, 1.0, &A[static_cast<size_t>(i)*n*k], &B[static_cast<size_t>(i)*n*k], 0.0, &C[static_cast<size_t>(i)*n*n]);
    } else {
      lib->ssyr2k(n, k, 1.0f, &A[static_cast<size_t>(i)*n*k], &B[static_cast<size_t>(i)*n*k], 0.0f, &C[static_cast<size_t>(i)*n*n]);
    }
      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 
    t_5 = std::chrono::high_resolution_clock::now(); 

  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  

  long long sum = 0;
  for (size_t i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();
}

long long test_class_ml::run_compare_syr2k_(int n, int k, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  if (isDouble) {
    return run_compare_syr2k_impl<double>(n, k, useML, num_of_duplicate, lib);
  } else {
    return run_compare_syr2k_impl<float>(n, k, useML, num_of_duplicate, lib);
  }
}

// Template implementation for TRMM
template<typename T>
long long run_compare_trmm_impl(int m, int n, bool useML, size_t num_of_duplicate, test_class* lib) {
  std::chrono::high_resolution_clock::time_point t1, t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1, t_2, t_3, t_4, t_5;
  int64_t time1, time2, time3, time4;

  std::vector<long long> time_vect;
  time_vect.reserve(num_of_duplicate);

    t_1 = std::chrono::high_resolution_clock::now(); 

  std::vector<T> A(static_cast<size_t>(num_of_duplicate) * m * m);
  std::vector<T> B(static_cast<size_t>(num_of_duplicate) * m * n);

    std::cout << "Trying to alloc GB:" << std::endl;  
  std::cout << (((unsigned long long)m)*m + m*n)*sizeof(T) * num_of_duplicate / 1e9 << std::endl;
  if (A.empty() || B.empty()) {
      std::cout << "Trying to alloc:" << std::endl;  
    std::cout << static_cast<size_t>(num_of_duplicate)*m*m*sizeof(T) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill A (triangular)
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
  size_t stride = static_cast<size_t>(num_of_duplicate)*m*m   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = A.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*m*m;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }
    // make A upper triangular
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < row; col++) {
      A[row*m + col] = T(0.0);
    }
  }

    // fill B
  stride = static_cast<size_t>(num_of_duplicate)*m*n   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = B.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  
  std::string routine_name = std::string(get_blas_prefix<T>()) + "trmm";
  Predictor predictor("xgb", routine_name);
    int nt;
    if (useML) {
    nt = predictor.model->predict_num_cores(m, n, routine_name);
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    std::cout << "Number of threads: " << nt << std::endl;

  for (int i = 0; i < num_of_duplicate; i++) {
      t1 = std::chrono::high_resolution_clock::now(); 
    if constexpr (std::is_same_v<T, double>) {
      lib->dtrmm(m, n, 1.0, &A[static_cast<size_t>(i)*m*m], &B[static_cast<size_t>(i)*m*n]);
    } else {
      lib->strmm(m, n, 1.0f, &A[static_cast<size_t>(i)*m*m], &B[static_cast<size_t>(i)*m*n]);
    }
      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 
  t_5 = std::chrono::high_resolution_clock::now();

  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl;
  std::cout << time1 << std::endl;
  std::cout << time2 << std::endl;
  std::cout << time3 << std::endl;
  std::cout << time4 << std::endl;
  std::cout << "End: time1~time4:" << std::endl;

  long long sum = 0;
  for (size_t i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();
}

// Template implementation for TRSM
template<typename T>
long long run_compare_trsm_impl(int m, int n, bool useML, size_t num_of_duplicate, test_class* lib) {
  std::chrono::high_resolution_clock::time_point t1, t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1, t_2, t_3, t_4, t_5;
  int64_t time1, time2, time3, time4;

  std::vector<long long> time_vect;
  time_vect.reserve(num_of_duplicate);

    t_1 = std::chrono::high_resolution_clock::now(); 

  std::vector<T> A(static_cast<size_t>(num_of_duplicate) * m * m);
  std::vector<T> B(static_cast<size_t>(num_of_duplicate) * m * n);

    std::cout << "Trying to alloc GB:" << std::endl;  
  std::cout << (((unsigned long long)m)*m + m*n)*sizeof(T) * num_of_duplicate / 1e9 << std::endl;
  if (A.empty() || B.empty()) {
      std::cout << "Trying to alloc:" << std::endl;  
    std::cout << static_cast<size_t>(num_of_duplicate)*m*m*sizeof(T) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill A (triangular)
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
  size_t stride = static_cast<size_t>(num_of_duplicate)*m*m   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = A.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*m*m;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }
    // make A upper triangular
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < row; col++) {
      A[row*m + col] = T(0.0);
    }
  }

    // fill B
  stride = static_cast<size_t>(num_of_duplicate)*m*n   / num_threads;
  for (i = 0; i < num_threads; i++) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
    arg_ptr->M = B.data();
        arg_ptr->start = i * stride;
    arg_ptr->type = get_type_char<T>();
    if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        if (rc) {
      throw blas_exceptions::ThreadCreationException(rc);
        }
    }
  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  
  std::string routine_name = std::string(get_blas_prefix<T>()) + "trsm";
  Predictor predictor("xgb", routine_name);
    int nt;
    if (useML) {
    nt = predictor.model->predict_num_cores(m, n, routine_name);
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    std::cout << "Number of threads: " << nt << std::endl;

  for (int i = 0; i < num_of_duplicate; i++) {
      t1 = std::chrono::high_resolution_clock::now(); 
    if constexpr (std::is_same_v<T, double>) {
      lib->dtrsm(m, n, 1.0, &A[static_cast<size_t>(i)*m*m], &B[static_cast<size_t>(i)*m*n]);
    } else {
      lib->strsm(m, n, 1.0f, &A[static_cast<size_t>(i)*m*m], &B[static_cast<size_t>(i)*m*n]);
    }
      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 
    t_5 = std::chrono::high_resolution_clock::now(); 

  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  

  long long sum = 0;
  for (size_t i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();
}

long long test_class_ml::run_compare_trmm_(int m, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  if (isDouble) {
    return run_compare_trmm_impl<double>(m, n, useML, num_of_duplicate, lib);
  } else {
    return run_compare_trmm_impl<float>(m, n, useML, num_of_duplicate, lib);
  }
}


long long test_class_ml::run_compare_trsm_(int m, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  if (isDouble) {
    return run_compare_trsm_impl<double>(m, n, useML, num_of_duplicate, lib);
  } else {
    return run_compare_trsm_impl<float>(m, n, useML, num_of_duplicate, lib);
  }
}

// Old buggy TRSM implementation removed - was calling TRMM functions instead of TRSM
// The correct template-based implementation is now used above

// std::vector<long long> test_syrk(int n, int k, int nt, bool isDouble, int num_of_duplicate);
// std::vector<long long> test_syr2k(int n, int k, int nt, bool isDouble, int num_of_duplicate);
// std::vector<long long> test_trmm(int m, int n, int nt, bool isDouble, int num_of_duplicate);
// std::vector<long long> test_trsm(int m, int n, int nt, bool isDouble, int num_of_duplicate);

long long test_class_ml::run_compare_gemv_(int m, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
    // pthread
    // fill A (triangular)
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*m*m / num_threads;
    for( i = 0; i < num_threads; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A.data();
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == num_threads - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*m;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          throw blas_exceptions::ThreadCreationException(rc);
        }
    }
    for( i = 0; i < num_threads; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }
    // make A upper triangular
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < row; col++) {
            A[row*m + col] = 0.0;
        }
    }


#ifdef DEBUG

    // display in matrix format
    std::cout << "A:" << std::endl;
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < m; col++) {
            std::cout << A[row*m + col] << " ";
        }
        std::cout << std::endl;
    }
    
    // check whether A is upper triangular
    bool is_upper_triangular = true;
    for (int row = 0; row < m; row++) {
        for (int col = row + 1; col < m; col++) {
            if (A[row*m + col] != 0.0) {
                is_upper_triangular = false;
                break;
            }
        }
        if (!is_upper_triangular) break;
    }
#endif

    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*m*n / num_threads;
    for( i = 0; i < num_threads; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B.data();
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == num_threads - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          throw blas_exceptions::ThreadCreationException(rc);
        }
    }
    for( i = 0; i < num_threads; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "dtrmm");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(m, n, "dtrmm");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      lib->dtrmm(m,n,1.0,&A[static_cast <size_t>(i)*m*m],&B[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    // Memory automatically freed when vectors go out of scope

    t_5 = std::chrono::high_resolution_clock::now(); 


  } else { //FLOAT

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space using std::vector for automatic memory management
    std::vector<float> A(static_cast<size_t>(num_of_duplicate) * m * m);
    std::vector<float> B(static_cast<size_t>(num_of_duplicate) * m * n);

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)m)*m + m*n)*sizeof(float) * num_of_duplicate / 1e9 << std::endl;
    if (A.empty() || B.empty()) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*m*sizeof(float) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A (triangular)
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*m*m / num_threads;
    for( i = 0; i < num_threads; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A.data();
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f'; // double
        if (i == num_threads - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*m;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          throw blas_exceptions::ThreadCreationException(rc);
        }
    }
    for( i = 0; i < num_threads; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }
    // make A upper triangular
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < row; col++) {
            A[row*m + col] = 0.0;
        }
    }



#ifdef DEBUG
    // print A in matrix format
    std::cout << "A:" << std::endl;
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < m; col++) {
            std::cout << A[row*m + col] << " ";
        }
        std::cout << std::endl;
    }
     

    // check whether A is upper triangular
    bool is_upper_triangular = true;
    for (int row = 0; row < m; row++) {
        for (int col = row + 1; col < m; col++) {
            if (A[row*m + col] != 0.0) {
                is_upper_triangular = false;
                break;
            }
        }
        if (!is_upper_triangular) break;
    }
    // print 
#endif

    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*m*n / num_threads;
    for( i = 0; i < num_threads; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B.data();
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f';
        if (i == num_threads - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          throw blas_exceptions::ThreadCreationException(rc);
        }
    }
    for( i = 0; i < num_threads; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "strmm");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(m, n, "strmm");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      lib->strmm(m,n,1.0,&A[static_cast <size_t>(i)*m*m],&B[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    // Memory automatically freed when vectors go out of scope

    t_5 = std::chrono::high_resolution_clock::now(); 



  }


  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  


  // return average of time_vect
  long long sum = 0;
  for (int i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();
}



long long test_class_ml::run_compare_gemv_(int m, int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  
  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // memalign
    double *A = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(double)); //strided array of n matrix
    double *x = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(double));
    double *y = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*sizeof(double));

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << ((unsigned long long)m*n)*sizeof(double) * num_of_duplicate / 1e9 << std::endl;  
    if (A == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*sizeof(double) << std::endl;  
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // fill matrices with random numbers
    // pthread
    // fill A
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
    size_t stride = static_cast<size_t>(num_of_duplicate) * m * n / num_threads;

    for (i = 0; i < num_threads; i++) {
        t_arg *arg_ptr = new t_arg;
        arg_ptr->M = A; // matrix start address
        arg_ptr->start = i * stride; // thread start address
        arg_ptr->type = 'd'; // double
        if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate) * m * n;
        else arg_ptr->end = (i + 1) * stride; // thread end address
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, static_cast<void*>(arg_ptr));

        if (rc) {
            std::cout << "Error: unable to create thread, " << rc << std::endl;
            throw blas_exceptions::ThreadCreationException(rc);
        }

        // delete arg_ptr;
    }

  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }

    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    

    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "dgemv");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(m, n, "dgemv");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: estimate the array sizes and finish the code
      lib->dgemv(m,n,1.0,&A[static_cast <size_t>(i)*m*n],&x[static_cast <size_t>(i)*n],0.0,&y[static_cast <size_t>(i)*m]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    free(A);
    free(x);
    free(y);


    t_5 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

 


  } else { //FLOAT

    t_1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    //assign space using memalign
    float *A = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(float)); //strided array of n matrix
    float *x = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(float));
    float *y = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*sizeof(float));

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << ((unsigned long long)(m+1)*n)*sizeof(float) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*(m * n + m+ n) * sizeof(float) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock


    // fill matrices with random numbers
    // pthread
    // fill A
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
    size_t stride = static_cast<size_t>(num_of_duplicate) * m * n / num_threads;

    for (i = 0; i < num_threads; i++) {
        t_arg *arg_ptr = new t_arg;
        arg_ptr->M = A; // matrix start address
        arg_ptr->start = i * stride; // thread start address
        arg_ptr->type = 'f'; // float
        if (i == num_threads - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate) * m * n;
        else arg_ptr->end = (i + 1) * stride; // thread end address
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, static_cast<void*>(arg_ptr));

        if (rc) {
            std::cout << "Error: unable to create thread, " << rc << std::endl;
            throw blas_exceptions::ThreadCreationException(rc);
        }

        // delete arg_ptr;
    }

  for (i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
    }

    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    
    
    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
  

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "sgemv");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(m, n, "sgemv");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    
    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      lib->sgemv(m, n, 1.0, &A[static_cast<size_t>(i) * m * n], &x[static_cast<size_t>(i) * n], 0.0, &y[static_cast<size_t>(i) * m]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    free(A);
    free(x);
    free(y);

    t_5 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock


  }


  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  


  // return average of time_vect
  long long sum = 0;
  for (int i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();

  // return time_vect;
}







long long test_class_ml::run_compare_syr_(int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  
  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1, t_2, t_3, t_4, t_5;
  int64_t time1, time2, time3, time4;

  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now();

    // Assign space using memalign
    double *x = (double *)memalign(128, static_cast<size_t>(num_of_duplicate) * n * sizeof(double));
    double *A = (double *)memalign(128, static_cast<size_t>(num_of_duplicate) * n * n * sizeof(double));

    std::cout << "Trying to alloc GB:" << std::endl;
    std::cout << ((unsigned long long)(n + n * n)) * sizeof(double) * num_of_duplicate / 1e9 << std::endl;

    t_2 = std::chrono::high_resolution_clock::now();

    // No obvious need for vectors to be parallelly filled
    for (size_t i = 0; i < (static_cast<size_t>(num_of_duplicate) * n); i++) {
        x[i] = 1.79769e+10 * (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
    }

    t_3 = std::chrono::high_resolution_clock::now();

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "dsyr");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(n, "dsyr");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    

    for (int i = 0; i < num_of_duplicate; i++) {
        t1 = std::chrono::high_resolution_clock::now();

        // TODO:
        lib->dsyr(n, 1.0, &x[static_cast<size_t>(i) * n], &A[static_cast<size_t>(i) * n * n]); // Strided matrix array input

        t2 = std::chrono::high_resolution_clock::now();
        time = std::chrono::duration_cast<std::chrono::microseconds>((t2 - t1)).count();
        time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();

    free(A);

    t_5 = std::chrono::high_resolution_clock::now();


  } else { //FLOAT
    // The precision depends on isDouble, use template to avoid code duplication
    

    t_1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    //assign space using memalign
    float *x = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(float));
    float *A = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*n*sizeof(float)); //strided array of n matrix


    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << ((unsigned long long)(2*n + n*n))*sizeof(float) * num_of_duplicate / 1e9 << std::endl;

    t_2 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // No obvious need for vectors to be parallelly filled 

    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x, y non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    
    
    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
  

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "ssyr");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(n, "ssyr");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      lib->ssyr(n, 1.0, &x[static_cast<size_t>(i) * n], &A[static_cast<size_t>(i) * n * n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    free(A);

    t_5 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

  }


  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  


  // return average of time_vect
  long long sum = 0;
  for (int i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();

  // return time_vect;
}

long long test_class_ml::run_compare_trsv_(int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  

  // TODO: how to construct A matrix??
  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;

  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // memalign
    double *A = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*n*sizeof(double)); //strided array of n matrix
    double *x = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(double)); // x will be overwritten by the result, so no y is needed

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << ((unsigned long long)n*n + n)*sizeof(double) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*n*n*sizeof(double) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // fill matrices with random numbers
    // pthread
    // fill A
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
    size_t total_elements = n * (n + 1) / 2;
    size_t stride = total_elements / num_threads;

    for( i = 0; i < num_threads; i++ ) {
        struct t_arg_symm *arg_ptr = (struct t_arg_symm *)malloc(sizeof(struct t_arg_symm));
        arg_ptr->M = A;
        arg_ptr->m = n;
        size_t start = i * stride;
        size_t end = (i == num_threads - 1) ? total_elements : (i + 1) * stride;

        arg_ptr->row_start = static_cast<size_t>((sqrt(2 * start + 0.25) - 0.5)); 
        arg_ptr->col_start = start - arg_ptr->row_start * (arg_ptr->row_start + 1) / 2;

        arg_ptr->row_end = static_cast<size_t>((sqrt(2 * end + 0.25) - 0.5));
        arg_ptr->col_end = end - arg_ptr->row_end * (arg_ptr->row_end + 1) / 2;

        arg_ptr->type = 'd'; // double
        arg_ptr->seed = i;

        rc = pthread_create(&threads[i], NULL, fill_with_triangular, (void *)arg_ptr);

        if (rc) {
          throw blas_exceptions::ThreadCreationException(rc);
        }
    }
    for( i = 0; i < num_threads; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }



    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    

    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "dtrsv");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(n, "dtrsv");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: estimate the array sizes and finish the code
      lib->dtrsv(n, &A[static_cast <size_t>(i)*n*n], &x[static_cast <size_t>(i)*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    free(A);
    free(x);

    t_5 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

 


  } else { //FLOAT

    t_1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    //assign space using memalign
    float *A = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*n*sizeof(float)); //strided array of n matrix
    float *x = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(float));

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << ((unsigned long long)n*n + n)*sizeof(float)* num_of_duplicate / 1e9 << std::endl; 
    if (A == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*(n*n + n) * sizeof(float) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock


    // fill matrices with random numbers
    // pthread
    // fill A
    int num_threads = blas_config::Config::getInstance().getNumThreads();
    if (num_threads > 1024) num_threads = 1024; // Safety limit
    pthread_t threads[1024]; // Fixed size array with bounds checking
    int rc;
    int i;
    size_t total_elements = n * (n + 1) / 2;
    size_t stride = total_elements / num_threads;

    for( i = 0; i < num_threads; i++ ) {
        struct t_arg_symm *arg_ptr = (struct t_arg_symm *)malloc(sizeof(struct t_arg_symm));
        arg_ptr->M = A;
        arg_ptr->m = n;
        size_t start = i * stride;
        size_t end = (i == num_threads - 1) ? total_elements : (i + 1) * stride;

        arg_ptr->row_start = static_cast<int>((sqrt(2 * start + 0.25) - 0.5));
        arg_ptr->col_start = start - arg_ptr->row_start * (arg_ptr->row_start + 1) / 2;

        arg_ptr->row_end = static_cast<int>((sqrt(2 * end + 0.25) - 0.5));
        arg_ptr->col_end = end - arg_ptr->row_end * (arg_ptr->row_end + 1) / 2;

        // std::cout << "row_start: " << arg_ptr->row_start << std::endl;
        // std::cout << "row_end: " << arg_ptr->row_end << std::endl;
        // std::cout << "col_start: " << arg_ptr->col_start << std::endl;
        // std::cout << "col_end: " << arg_ptr->col_end << std::endl;

        arg_ptr->type = 'f'; // float
        arg_ptr->seed = i;

        rc = pthread_create(&threads[i], NULL, fill_with_triangular, (void *)arg_ptr);

        if (rc) {
          throw blas_exceptions::ThreadCreationException(rc);
        }
    }
    for( i = 0; i < num_threads; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }


    // TODO: fix it, but now it works as a workaround
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < i; j++) {
        A[i * n + j] = 0;
        if (i == j) A[i * n + j] = 1;
        if (A[j * n + i] == 0) A[j * n + i] = rand() % 100 + 1;
      }
    }
    


    int counter = 0;
    // check if A is upper-triangular
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (A[i * n + j] != 0) {
                std::cout << "Error: A is not upper-triangular." << std::endl;
                // print details
                std::cout << "i: " << i << ", j: " << j << std::endl;
                std::cout << "A[i * n + j]: " << A[i * n + j] << std::endl;
                std::cout << "counter: " << ++counter << std::endl;
            }
        }
    } 

    // check if A's upper part is non-zero
    counter = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            if (A[i * n + j] == 0) {
                std::cout << "Error: A's upper part is zero." << std::endl;
                // print details
                std::cout << "i: " << i << ", j: " << j << std::endl;
                std::cout << "A[i * n + j]: " << A[i * n + j] << std::endl;
                std::cout << "counter: " << ++counter << std::endl;
            }
        }
    }

    // fill x
    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    
    
    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
  

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "strsv");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(n, "strsv");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    
    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      lib->strsv(n, &A[static_cast<size_t>(i) * n * n], &x[static_cast<size_t>(i) * n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    free(A);
    free(x);

    t_5 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock


  }


  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  


  // return average of time_vect
  long long sum = 0;
  for (int i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();

  // return time_vect;
}



long long test_class_ml::run_compare_dot_(int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  
  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;
  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE
      
    t_1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    //assign space
    double *x = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(double));
    double *y = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(double)); //strided array of n matrix

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << ((unsigned long long)(2*n))*sizeof(double) * num_of_duplicate / 1e9 << std::endl;

    t_2 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // No obvious need for vectors to be parallelly filled
    for (size_t i = 0; i < (static_cast<size_t>(num_of_duplicate) * n); i++) {
        x[i] = 1.79769e+10 * (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
        y[i] = 1.79769e+10 * (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
    }

    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
  
    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "ddot");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(n, "ddot");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    
    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
      
      lib->ddot(n, &x[static_cast<size_t>(i) * n], &y[static_cast<size_t>(i) * n]);

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    free(x);
    free(y);
      
    t_5 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
  } else { //FLOAT
    // The precision depends on isDouble, use template to avoid code duplication

    t_1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    //assign space
    float *x = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(float));
    float *y = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(float)); //strided array of n matrix

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << ((unsigned long long)(2*n))*sizeof(float) * num_of_duplicate / 1e9 << std::endl;

    t_2 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // No obvious need for vectors to be parallelly filled 

    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x, y non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
      y[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    
    
    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
  

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "sdot");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(n, "sdot");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      lib->sdot(n, &x[static_cast<size_t>(i) * n], &y[static_cast<size_t>(i) * n]);

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    free(x);
    free(y);

    t_5 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock


  }

  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  


  // return average of time_vect
  long long sum = 0;
  for (int i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();

  // return time_vect;

}


long long test_class_ml::run_compare_axpy_(int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
  
  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;
  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    //assign space
    double *x = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(double));
    double *y = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(double)); //strided array of n matrix

    std::cout << "Trying to alloc GB:" << std::endl;
    std::cout << ((unsigned long long)(2*n))*sizeof(double) * num_of_duplicate / 1e9 << std::endl;
     
    t_2 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // No obvious need for vectors to be parallelly filled
    for (size_t i = 0; i < (static_cast<size_t>(num_of_duplicate) * n); i++) {
      x[i] = 1.79769e+10 * (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
      y[i] = 1.79769e+10 * (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
    }

    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "daxpy");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(n, "daxpy");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    
    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      lib->daxpy(n, 1.0, &x[static_cast<size_t>(i) * n], &y[static_cast<size_t>(i) * n]);

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    free(x);
    free(y);

    t_5 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

  } else { //FLOAT
    // The precision depends on isDouble, use template to avoid code duplication
    
    t_1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    //assign space
    float *x = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(float));
    float *y = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*sizeof(float)); //strided array of n matrix
  

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << ((unsigned long long)(2*n))*sizeof(float) * num_of_duplicate / 1e9 << std::endl;

    t_2 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // No obvious need for vectors to be parallelly filled 

    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x, y non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
      y[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    
    
    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
  

    // this->set_num_threads(nt);
    // Use predictor to set threads
    Predictor predictor("xgb", "saxpy");
    int nt;
    if (useML) {
      // nt = get_optimum_num_cores(m,k,n);
      nt = predictor.model->predict_num_cores(n, "saxpy");
      omp_set_num_threads(nt);
    } else {
      nt = max_num_threads;
      omp_set_num_threads(nt);
    }
    // print out the number of threads
    std::cout << "Number of threads: " << nt << std::endl;

    
    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      lib->saxpy(n, 1.0, &x[static_cast<size_t>(i) * n], &y[static_cast<size_t>(i) * n]);

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    free(x);
    free(y);

    t_5 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock


  }


  time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
  time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
  time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
  time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();

  std::cout << "time1~time4:" << std::endl; 
  std::cout << time1 << std::endl;  
  std::cout << time2 << std::endl;  
  std::cout << time3 << std::endl;  
  std::cout << time4 << std::endl;  
  std::cout << "End: time1~time4:" << std::endl;  


  // return average of time_vect
  long long sum = 0;
  for (int i = 0; i < time_vect.size(); i++) {
    sum += time_vect[i];
  }
  return sum / time_vect.size();

  // return time_vect;
}