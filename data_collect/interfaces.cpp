#include "interfaces.h"
#include <thread>
#include <unistd.h>
#include <omp.h>
#include <cstdlib>
#include <iostream>
#include "get-ncores.h"
#include <malloc.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <cmath>

using namespace std;

#define NUM_THREADS 24


struct t_arg { // struct for pthread arguments
    void* M; // matrix to fill
    size_t start; // start index
    size_t end; // end index
    unsigned int seed; // seed for random number generator
    char type; // data type ('f' for float, 'd' for double)
};

struct t_arg_symm { // struct for pthread arguments
    int m; // matrix size
    void* M; // matrix to fill
    size_t row_start; // start row
    size_t col_start; // start column
    size_t row_end; // end row
    size_t col_end; // end column
    unsigned int seed; // seed for random number generator
    char type; // data type ('f' for float, 'd' for double)
};

/**
 * Fills the given memory block with random data.
 * 
 * @param arg A pointer to the memory block to be filled with random data.
 * @return void* A null pointer.
 */
void *fill_with_rand(void *arg) {
    struct t_arg *arg_ptr = (struct t_arg*) arg;
    if (arg_ptr->type == 'f') {
        float *M = (float*) arg_ptr->M;
        for (size_t i = arg_ptr->start; i < arg_ptr->end; i++) {
            M[i] = (float)rand_r(&(arg_ptr->seed))/RAND_MAX;
        }
    } else if (arg_ptr->type == 'd') {
        double *M = (double*) arg_ptr->M;
        for (size_t i = arg_ptr->start; i < arg_ptr->end; i++) {
            M[i] = (double)rand_r(&(arg_ptr->seed))/RAND_MAX;
        }
    }

    pthread_exit(NULL);
}

void *fill_with_symmetric(void *arg) {
    struct t_arg_symm *arg_ptr = (struct t_arg_symm*) arg;
    int m = arg_ptr->m;
    if (arg_ptr->type == 'f') {
        float *M = (float*) arg_ptr->M;
        for (size_t row = arg_ptr->row_start; row <= arg_ptr->row_end; row++) {
            size_t col_start = (row == arg_ptr->row_start) ? arg_ptr->col_start : row;
            size_t col_end = (row == arg_ptr->row_end) ? arg_ptr->col_end : m;
            for (size_t col = col_start; col < col_end; col++) {
                M[row*m + col] = (float)rand_r(&(arg_ptr->seed))/RAND_MAX;
                if (row != col) {
                    M[col*m + row] = M[row*m + col];
                }
            }
        }
    } else if (arg_ptr->type == 'd') {
        double *M = (double*) arg_ptr->M;
        for (size_t row = arg_ptr->row_start; row <= arg_ptr->row_end; row++) {
            size_t col_start = (row == arg_ptr->row_start) ? arg_ptr->col_start : row;
            size_t col_end = (row == arg_ptr->row_end) ? arg_ptr->col_end : m;
            for (size_t col = col_start; col < col_end; col++) {
                M[row*m + col] = (double)rand_r(&(arg_ptr->seed))/RAND_MAX;
                if (row != col) {
                    M[col*m + row] = M[row*m + col];
                }
            }
        }
    }

    pthread_exit(NULL);
}

void *fill_with_triangular(void *arg) {
    struct t_arg_symm *arg_ptr = (struct t_arg_symm*) arg;
    int m = arg_ptr->m;
    if (arg_ptr->type == 'f') {
        float *M = (float*) arg_ptr->M;
        for (size_t row = arg_ptr->row_start; row <= arg_ptr->row_end; row++) {
            size_t col_start = (row == arg_ptr->row_start) ? arg_ptr->col_start : row; 
            size_t col_end = (row == arg_ptr->row_end) ? arg_ptr->col_end : m;
            for (size_t col = col_start; col < col_end; col++) { 
                M[row*m + col] = (float)rand_r(&(arg_ptr->seed))/RAND_MAX;
                if (row != col) { 
                    M[col*m + row] = 0;
                }
            }
        }
    } else if (arg_ptr->type == 'd') {
        double *M = (double*) arg_ptr->M;
        for (size_t row = arg_ptr->row_start; row <= arg_ptr->row_end; row++) {
            size_t col_start = (row == arg_ptr->row_start) ? arg_ptr->col_start : row;
            size_t col_end = (row == arg_ptr->row_end) ? arg_ptr->col_end : m;
            for (size_t col = col_start; col < col_end; col++) {
                M[row*m + col] = (double)rand_r(&(arg_ptr->seed))/RAND_MAX;
                if (row != col) {
                    M[col*m + row] = 0;
                }
            }
        }
    }

    pthread_exit(NULL);
}

static int ncores = get_ncores();

library::library(std::string n, int nt) : name(n), max_threads(nt) {} 
library::library(std::string n) : name(n), max_threads(ncores) {}
library::library() : name("NO_NAME_PROVIDED"), max_threads(1) {} 
library::~library() {}

void library::set_num_threads(int nt) {}
test_class::~test_class() {}

/**
 * Performs a test of the GEMM function with the specified parameters.
 * @param m The number of rows in the matrix A and C.
 * @param k The number of columns in the matrix A and rows in the matrix B.
 * @param n The number of columns in the matrix B and C.
 * @param nt The number of threads to use for the computation.
 * @param isDouble Whether to use double-precision floating-point numbers.
 * @param num_of_duplicate The number of times to duplicate the test.
 * @return A vector containing the execution times for each test.
 */
std::vector<long long> test_class::test_gemm(int m, int k, int n, int nt, bool isDouble, int num_of_duplicate) {
  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;

  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    double *A = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*k*sizeof(double)); //strided array of n matrix
    double *B = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*k*n*sizeof(double));
    double *C = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(double));


    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)m)*k + n*k + m*n)*sizeof(double) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || B == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*k*sizeof(double) << std::endl;  
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*m*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*n*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      dgemm(m,k,n,1.0,&A[static_cast <size_t>(i)*m*k],&B[static_cast <size_t>(i)*k*n],0.0,&C[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);
    free(C);

    t_5 = std::chrono::high_resolution_clock::now(); 


  } else { //FLOAT

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    float *A = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*k*sizeof(float)); //strided array of n matrix
    float *B = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*k*n*sizeof(float));
    float *C = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(float));

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)m)*k + n*k + m*n)*sizeof(float) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || B == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*k*sizeof(float) << std::endl;  
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*m*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f';
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*n*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f';
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      sgemm(m,k,n,1.0,&A[static_cast <size_t>(i)*m*k],&B[static_cast <size_t>(i)*k*n],0.0,&C[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);
    free(C);

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


  return time_vect;
}



std::vector<long long> test_class::test_symm(int m, int n, int nt, bool isDouble, int num_of_duplicate) {
  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;

  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    double *A = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*m*sizeof(double)); //strided array of n matrix
    double *B = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(double));
    double *C = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(double));


    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)m)*m + m*n + m*n)*sizeof(double) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || B == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*m*sizeof(double) << std::endl;  
      std::cout << "Allocation error." << std::endl;  
    }


    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A (symmetric)
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t total_elements = m * (m + 1) / 2;
    size_t stride = total_elements / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg_symm *arg_ptr = (struct t_arg_symm *)malloc(sizeof(struct t_arg_symm));
        arg_ptr->M = A;
        arg_ptr->m = m;
        size_t start = i * stride;
        size_t end = (i == NUM_THREADS - 1) ? total_elements : (i + 1) * stride;

        arg_ptr->row_start = static_cast<size_t>(sqrt(2 * start + 0.25) - 0.5);
        arg_ptr->col_start = start - arg_ptr->row_start * (arg_ptr->row_start + 1) / 2;

        arg_ptr->row_end = static_cast<size_t>(sqrt(2 * end + 0.25) - 0.5);
        arg_ptr->col_end = end - arg_ptr->row_end * (arg_ptr->row_end + 1) / 2;

        arg_ptr->type = 'd'; // float
        arg_ptr->seed = i;

        rc = pthread_create(&threads[i], NULL, fill_with_symmetric, (void *)arg_ptr);

        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
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
    
    // check whether A is symmetric
    bool is_symmetric = true;
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < row; col++) {
            if (A[row*m + col] != A[col*m + row]) {
                is_symmetric = false;
                break;
            }
        }
        if (!is_symmetric) break;
    }
#endif

    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*m*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      dsymm(m,n,1.0,&A[static_cast <size_t>(i)*m*m],&B[static_cast <size_t>(i)*m*n],0.0,&C[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);
    free(C);

    t_5 = std::chrono::high_resolution_clock::now(); 


  } else { //FLOAT

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    float *A = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*m*sizeof(float)); //strided array of n matrix
    float *B = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(float));
    float *C = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(float));

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)m)*m + m*n + m*n)*sizeof(float) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || B == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*m*sizeof(float) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A (symmetric)
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t total_elements = m * (m + 1) / 2;
    size_t stride = total_elements / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg_symm *arg_ptr = (struct t_arg_symm *)malloc(sizeof(struct t_arg_symm));
        arg_ptr->M = A;
        arg_ptr->m = m;
        size_t start = i * stride;
        size_t end = (i == NUM_THREADS - 1) ? total_elements : (i + 1) * stride;

        arg_ptr->row_start = static_cast<size_t>(sqrt(2 * start + 0.25) - 0.5);
        arg_ptr->col_start = start - arg_ptr->row_start * (arg_ptr->row_start + 1) / 2;

        arg_ptr->row_end = static_cast<size_t>(sqrt(2 * end + 0.25) - 0.5);
        arg_ptr->col_end = end - arg_ptr->row_end * (arg_ptr->row_end + 1) / 2;

        arg_ptr->type = 'f'; // float
        arg_ptr->seed = i;

        rc = pthread_create(&threads[i], NULL, fill_with_symmetric, (void *)arg_ptr);

        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
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
     

    // check if A is symmetric
    bool is_symmetric = true;
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < row; col++) {
            if (A[row*m + col] != A[col*m + row]) {
                is_symmetric = false;
                break;
            }
        }
        if (!is_symmetric) break;
    }
    // print 
    std::cout << "A is symmetric: " << is_symmetric << std::endl;
#endif

    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*m*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f';
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      ssymm(m,n,1.0,&A[static_cast <size_t>(i)*m*m],&B[static_cast <size_t>(i)*m*n],0.0,&C[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);
    free(C);

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


  return time_vect;
}




std::vector<long long> test_class::test_syrk(int n, int k, int nt, bool isDouble, int num_of_duplicate) {

  // C = alpha * A * A.T + beta * C
  // A is n x k
  // C is n x n

  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;

  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    double *A = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*k*sizeof(double)); //strided array of n matrix
    double *C = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*n*sizeof(double));


    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)n)*n + n*k)*sizeof(double) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || C == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*n*n*sizeof(double) << std::endl;  
      std::cout << "Allocation error." << std::endl;  
    }


    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A 
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*n*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }



    // fill C
    // pthread_t threads[NUM_THREADS];
    // int rc;
    // int i;
    stride = static_cast <size_t>(num_of_duplicate)*n*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = C;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      dsyrk(n,k,1.0,&A[static_cast <size_t>(i)*n*k],0.0,&C[static_cast <size_t>(i)*n*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(C);

    t_5 = std::chrono::high_resolution_clock::now(); 


  } else { //FLOAT

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    float *A = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*k*sizeof(float)); //strided array of n matrix
    float *C = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*n*sizeof(float));

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)n)*n + n*k)*sizeof(float) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || C == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*n*n*sizeof(float) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A 
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*n*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }



    // fill C
    stride = static_cast <size_t>(num_of_duplicate)*n*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = C;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      ssyrk(n,k,1.0,&A[static_cast <size_t>(i)*n*k],0.0,&C[static_cast <size_t>(i)*n*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(C);

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


  return time_vect;
}



std::vector<long long> test_class::test_syr2k(int n, int k, int nt, bool isDouble, int num_of_duplicate) {

  // C = alpha * A * B.T + alpha * B * A.T + beta * C
  // A, B are n x k
  // C is n x n

  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;

  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    double *A = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*k*sizeof(double)); //strided array of n matrix
    double *B = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*k*sizeof(double));
    double *C = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*n*sizeof(double));


    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)n)*n + 2*n*k)*sizeof(double) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || C == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*n*n*sizeof(double) << std::endl;  
      std::cout << "Allocation error." << std::endl;  
    }


    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A 
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*n*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }


    // fill matrices with random numbers
    // pthread
    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*n*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }


    // fill C
    stride = static_cast <size_t>(num_of_duplicate)*n*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = C;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }


    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      dsyr2k(n,k,1.0,&A[static_cast <size_t>(i)*n*k],&B[static_cast <size_t>(i)*n*k],0.0,&C[static_cast <size_t>(i)*n*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);
    free(C);

    t_5 = std::chrono::high_resolution_clock::now(); 


  } else { //FLOAT

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    float *A = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*k*sizeof(float)); //strided array of n matrix
    float *B = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*k*sizeof(float));
    float *C = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*n*n*sizeof(float));

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)n)*n + 2*n*k)*sizeof(float) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || C == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*n*n*sizeof(float) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A 
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*n*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    // fill matrices with random numbers
    // pthread
    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*n*k / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*k;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }



    // fill C
    stride = static_cast <size_t>(num_of_duplicate)*n*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = C;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      ssyr2k(n,k,1.0,&A[static_cast <size_t>(i)*n*k],&B[static_cast <size_t>(i)*n*k],0.0,&C[static_cast <size_t>(i)*n*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);
    free(C);

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


  return time_vect;
}



// std::vector<long long> test_syrk(int n, int k, int nt, bool isDouble, int num_of_duplicate);
// std::vector<long long> test_syr2k(int n, int k, int nt, bool isDouble, int num_of_duplicate);
// std::vector<long long> test_trmm(int m, int n, int nt, bool isDouble, int num_of_duplicate);
// std::vector<long long> test_trsm(int m, int n, int nt, bool isDouble, int num_of_duplicate);

std::vector<long long> test_class::test_trmm(int m, int n, int nt, bool isDouble, int num_of_duplicate) {

  // B = alpha * A * B
  // A is m x m ?
  // B is m x n

  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;

  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    double *A = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*m*sizeof(double)); //strided array of n matrix
    double *B = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(double));


    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)m)*m + m*n)*sizeof(double) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || B == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*m*sizeof(double) << std::endl;  
      std::cout << "Allocation error." << std::endl;  
    }


    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A (triangular)
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*m*m / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*m;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
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
    stride = static_cast <size_t>(num_of_duplicate)*m*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      dtrmm(m,n,1.0,&A[static_cast <size_t>(i)*m*m],&B[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);

    t_5 = std::chrono::high_resolution_clock::now(); 


  } else { //FLOAT

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    float *A = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*m*sizeof(float)); //strided array of n matrix
    float *B = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(float));

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)m)*m + m*n)*sizeof(float) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || B == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*m*sizeof(float) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A (triangular)
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*m*m / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*m;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
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
    stride = static_cast <size_t>(num_of_duplicate)*m*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f';
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      strmm(m,n,1.0,&A[static_cast <size_t>(i)*m*m],&B[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);

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


  return time_vect;
}

// std::vector<long long> test_syrk(int n, int k, int nt, bool isDouble, int num_of_duplicate);
// std::vector<long long> test_syr2k(int n, int k, int nt, bool isDouble, int num_of_duplicate);
// std::vector<long long> test_trmm(int m, int n, int nt, bool isDouble, int num_of_duplicate);
// std::vector<long long> test_trsm(int m, int n, int nt, bool isDouble, int num_of_duplicate);

std::vector<long long> test_class::test_trsm(int m, int n, int nt, bool isDouble, int num_of_duplicate) {

  // B = alpha * A * B
  // A is m x m ?
  // B is m x n

  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;

  std::vector<long long> time_vect;//vector to store iterations' time value
  time_vect.reserve(num_of_duplicate);

  if (isDouble) {// DOUBLE

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    double *A = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*m*sizeof(double)); //strided array of n matrix
    double *B = (double*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(double));


    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)m)*m + m*n)*sizeof(double) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || B == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*m*sizeof(double) << std::endl;  
      std::cout << "Allocation error." << std::endl;  
    }


    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A (triangular)
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*m*m / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*m;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
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
    
    // check whether A is symmetric
    bool is_symmetric = true;
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < row; col++) {
            if (A[row*m + col] != A[col*m + row]) {
                is_symmetric = false;
                break;
            }
        }
        if (!is_symmetric) break;
    }
#endif

    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*m*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }

    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      dtrsm(m,n,1.0,&A[static_cast <size_t>(i)*m*m],&B[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);

    t_5 = std::chrono::high_resolution_clock::now(); 


  } else { //FLOAT

    t_1 = std::chrono::high_resolution_clock::now(); 

    //assign space
    float *A = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*m*sizeof(float)); //strided array of n matrix
    float *B = (float*)memalign(128, static_cast <size_t>(num_of_duplicate)*m*n*sizeof(float));

    std::cout << "Trying to alloc GB:" << std::endl;  
    std::cout << (((unsigned long long)m)*m + m*n)*sizeof(float) * num_of_duplicate / 1e9 << std::endl;
    if (A == 0 || B == 0) {
      std::cout << "Trying to alloc:" << std::endl;  
      std::cout << static_cast <size_t>(num_of_duplicate)*m*m*sizeof(float) << std::endl;
      std::cout << "Allocation error." << std::endl;  
    }

    t_2 = std::chrono::high_resolution_clock::now(); 

    // fill matrices with random numbers
    // pthread
    // fill A (triangular)
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*m*m / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*m;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
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
     

    // check if A is symmetric
    bool is_symmetric = true;
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < row; col++) {
            if (A[row*m + col] != A[col*m + row]) {
                is_symmetric = false;
                break;
            }
        }
        if (!is_symmetric) break;
    }
    // print 
    std::cout << "A is symmetric: " << is_symmetric << std::endl;
#endif

    // fill B
    stride = static_cast <size_t>(num_of_duplicate)*m*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = B;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f';
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*m*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }    
    
    t_3 = std::chrono::high_resolution_clock::now(); 
  

    // this->set_num_threads(nt);
    // omp_set_num_threads(nt);  


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now(); 

      strsm(m,n,1.0,&A[static_cast <size_t>(i)*m*m],&B[static_cast <size_t>(i)*m*n]);//strided matrix array input

      t2 = std::chrono::high_resolution_clock::now();
      time = std::chrono::duration_cast<std::chrono::microseconds>((t2-t1)).count();
      time_vect.push_back(time);
    }

    t_4 = std::chrono::high_resolution_clock::now(); 

    free(A);
    free(B);

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


  return time_vect;
}

/**
 * This function performs a test of the dgemv function using double or float data types.
 * It allocates memory for matrices A, x, and y using memalign and fills them with random numbers using pthread.
 * It then performs dgemv operation on the matrices and records the time taken for each iteration.
 * The function returns a vector containing the time taken for each iteration.
 *
 * @param m The number of rows in matrix A.
 * @param n The number of columns in matrix A and the length of vector x.
 * @param nt The number of threads to use for filling the matrices with random numbers.
 * @param isDouble A boolean value indicating whether to use double or float data types.
 * @param num_of_duplicate The number of times to perform the dgemv operation.
 * @return A vector containing the time taken for each iteration.
 */
std::vector<long long> test_class::test_gemv(int m, int n, int nt, bool isDouble, int num_of_duplicate) {
  
  std::chrono::high_resolution_clock::time_point t1,t2;
  int64_t time;
  std::chrono::high_resolution_clock::time_point t_1,t_2,t_3,t_4,t_5;
  int64_t time1,time2,time3,time4;

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
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast<size_t>(num_of_duplicate) * m * n / NUM_THREADS;

    for (i = 0; i < NUM_THREADS; i++) {
        t_arg *arg_ptr = new t_arg;
        arg_ptr->M = A; // matrix start address
        arg_ptr->start = i * stride; // thread start address
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate) * m * n;
        else arg_ptr->end = (i + 1) * stride; // thread end address
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, static_cast<void*>(arg_ptr));

        if (rc) {
            std::cout << "Error: unable to create thread, " << rc << std::endl;
            exit(-1);
        }

        // delete arg_ptr;
    }

    for (i = 0; i < NUM_THREADS; i++) {
        rc = pthread_join(threads[i], NULL);
    }

    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    

    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // this->set_num_threads(nt);


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: estimate the array sizes and finish the code
      dgemv(m,n,1.0,&A[static_cast <size_t>(i)*m*n],&x[static_cast <size_t>(i)*n],0.0,&y[static_cast <size_t>(i)*m]);//strided matrix array input

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
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast<size_t>(num_of_duplicate) * m * n / NUM_THREADS;

    for (i = 0; i < NUM_THREADS; i++) {
        t_arg *arg_ptr = new t_arg;
        arg_ptr->M = A; // matrix start address
        arg_ptr->start = i * stride; // thread start address
        arg_ptr->type = 'f'; // float
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast<size_t>(num_of_duplicate) * m * n;
        else arg_ptr->end = (i + 1) * stride; // thread end address
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, static_cast<void*>(arg_ptr));

        if (rc) {
            std::cout << "Error: unable to create thread, " << rc << std::endl;
            exit(-1);
        }

        // delete arg_ptr;
    }

    for (i = 0; i < NUM_THREADS; i++) {
        rc = pthread_join(threads[i], NULL);
    }

    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    
    
    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
  

    // this->set_num_threads(nt);

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      sgemv(m, n, 1.0, &A[static_cast<size_t>(i) * m * n], &x[static_cast<size_t>(i) * n], 0.0, &y[static_cast<size_t>(i) * m]);//strided matrix array input

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


  return time_vect;
}


/**
 * Calculates the Syr function for a given input size and number of threads.
 * 
 * @param n The input size.
 * @param nt The number of threads to use.
 * @param isDouble Whether to use double precision or not.
 * @param num_of_duplicate The number of times to duplicate the input.
 * @return A vector containing the results of the Syr function.
 */
std::vector<long long> test_class::test_syr(int n, int nt, bool isDouble, int num_of_duplicate) {
  
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


    for (int i = 0; i < num_of_duplicate; i++) {
        t1 = std::chrono::high_resolution_clock::now();

        // TODO:
        dsyr(n, 1.0, &x[static_cast<size_t>(i) * n], &A[static_cast<size_t>(i) * n * n]); // Strided matrix array input

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


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      ssyr(n, 1.0, &x[static_cast<size_t>(i) * n], &A[static_cast<size_t>(i) * n * n]);//strided matrix array input

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


  return time_vect;
}

std::vector<long long> test_class::test_trsv(int n, int nt, bool isDouble, int num_of_duplicate) {
  

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
    // fill A (triangular)
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*n*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'd'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }
    // make A upper triangular
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < row; col++) {
            A[row*n + col] = 0.0;
        }
    }


    for (size_t i = 0; i < (static_cast <size_t>(num_of_duplicate)*n); i++)//Make x non-zero
    {
      x[i] = 1.79769e+10 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
    

    t_3 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

    // this->set_num_threads(nt);


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: estimate the array sizes and finish the code
      dtrsv(n, &A[static_cast <size_t>(i)*n*n], &x[static_cast <size_t>(i)*n]);//strided matrix array input

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
    // fill A (triangular)
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    size_t stride = static_cast <size_t>(num_of_duplicate)*n*n / NUM_THREADS;
    for( i = 0; i < NUM_THREADS; i++ ) {
        struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
        arg_ptr->M = A;
        arg_ptr->start = i * stride;
        arg_ptr->type = 'f'; // double
        if (i == NUM_THREADS - 1) arg_ptr->end = static_cast <size_t>(num_of_duplicate)*n*n;
        else arg_ptr->end = (i+1) * stride;
        arg_ptr->seed = i;
        rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
        
        if (rc) {
          std::cout << "Error:unable to create thread," << rc << std::endl;
          exit(-1);
        }
    }
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], NULL);
    }
    // make A upper triangular
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < row; col++) {
            A[row*n + col] = 0.0;
        }
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

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      strsv(n, &A[static_cast<size_t>(i) * n * n], &x[static_cast<size_t>(i) * n]);//strided matrix array input

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


  return time_vect;
}



std::vector<long long> test_class::test_dot(int n, int nt, bool isDouble, int num_of_duplicate) {
  
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

      for(int i=0; i < num_of_duplicate; i++){
        t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
        
        ddot(n, &x[static_cast<size_t>(i) * n], &y[static_cast<size_t>(i) * n]);

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


    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      sdot(n, &x[static_cast<size_t>(i) * n], &y[static_cast<size_t>(i) * n]);

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


  return time_vect;

}


std::vector<long long> test_class::test_axpy(int n, int nt, bool isDouble, int num_of_duplicate) {
  
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

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      daxpy(n, 1.0, &x[static_cast<size_t>(i) * n], &y[static_cast<size_t>(i) * n]);

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

    for(int i=0; i < num_of_duplicate; i++){
      t1 = std::chrono::high_resolution_clock::now();//https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

      // TODO: 
      saxpy(n, 1.0, &x[static_cast<size_t>(i) * n], &y[static_cast<size_t>(i) * n]);

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


  return time_vect;
}