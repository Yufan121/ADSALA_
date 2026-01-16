# Code Duplication Analysis

## Executive Summary

Analysis of `src/data_collect` and `src/library_frame` reveals **~3500+ lines of duplicated code** that can be eliminated through refactoring, resulting in **~2000 lines net reduction** (~30% reduction in `interfaces-ml.cpp`).

## 1. Context Classes Duplication (`src/data_collect/`)

### Problem
Five BLAS context classes (BLAS-context, BLIS-context, MKL-context, OpenBLAS-context, cuBLAS-context) with nearly identical structure.

### Duplication Details
- **Files affected**: 5 header files + 5 implementation files
- **Duplicated declarations**: ~48 method signatures per class (dgemm, sgemm, dsymm, ssymm, dsyrk, ssyrk, etc.)
- **Duplicated patterns**: 
  - Constructor/destructor patterns
  - Similar cblas_* call patterns
  - Thread management code
- **Total duplication**: ~500 lines

### Example
```cpp
// BLIS-context.h, MKL-context.h - IDENTICAL method signatures
void dgemm(int m, int n, int k, double alpha, double *A, double *B, double beta, double *C);
void sgemm(int m, int n, int k, float alpha, float *A, float *B, float beta, float *C);
void dsymm(int m, int n, double alpha, double *A, double *B, double beta, double *C);
// ... 45+ more identical declarations
```

### Solution
Create base template class `BaseBLASContext<T>` using CRTP pattern:
- Shared method declarations in base class
- Type-specific implementations via template specialization
- Library-specific initialization in derived classes

### Impact
- **Before**: ~500 lines of duplicated code
- **After**: ~100 lines of shared code + ~200 lines of implementations
- **Reduction**: ~200 lines (~40% reduction)

---

## 2. Matrix Filling Patterns (`src/library_frame/BLAS/src/interfaces-ml.cpp`)

### Problem
Repeated pthread creation/join patterns for matrix initialization appear **43+ times**.

### Duplication Details
- **Pattern repeated**: 43+ times across template functions
- **Duplicated code per instance**: ~50 lines
- **Total duplication**: ~2000+ lines

### Duplicated Elements
1. **Thread array allocation**:
   ```cpp
   int num_threads = blas_config::Config::getInstance().getNumThreads();
   if (num_threads > 1024) num_threads = 1024;
   pthread_t threads[1024];
   ```

2. **Thread creation loop**:
   ```cpp
   size_t stride = static_cast<size_t>(num_of_duplicate)*m*k / num_threads;
   for (i = 0; i < num_threads; i++) {
       struct t_arg *arg_ptr = (struct t_arg *)malloc(sizeof(struct t_arg));
       arg_ptr->M = A.data();
       arg_ptr->start = i * stride;
       arg_ptr->type = get_type_char<T>();
       if (i == num_threads - 1) arg_ptr->end = ...;
       else arg_ptr->end = (i+1) * stride;
       arg_ptr->seed = i;
       rc = pthread_create(&threads[i], NULL, fill_with_rand, (void *)arg_ptr);
       if (rc) throw blas_exceptions::ThreadCreationException(rc);
   }
   ```

3. **Thread join loop**:
   ```cpp
   for (i = 0; i < num_threads; i++) {
       rc = pthread_join(threads[i], NULL);
   }
   ```

### Solution
Extract helper functions:
```cpp
template<typename T>
void fill_matrix_parallel(std::vector<T>& matrix, size_t total_elements, 
                         size_t num_of_duplicate, char type_char);

void create_pthreads(pthread_t* threads, int num_threads, 
                     void* matrix_ptr, size_t start, size_t end, 
                     char type_char, int seed);

void join_pthreads(pthread_t* threads, int num_threads);
```

### Impact
- **Before**: ~2000 lines of duplicated pthread code
- **After**: ~100 lines of helper functions
- **Reduction**: ~1900 lines (~95% reduction)

---

## 3. Timing Code Duplication (`src/library_frame/BLAS/src/interfaces-ml.cpp`)

### Problem
Repeated timing measurement and reporting code appears **24+ times**.

### Duplication Details
- **Pattern repeated**: 24+ times
- **Duplicated code per instance**: ~20 lines
- **Total duplication**: ~500 lines

### Duplicated Elements
1. **Time calculation**:
   ```cpp
   time1 = std::chrono::duration_cast<std::chrono::microseconds>((t_2-t_1)).count();
   time2 = std::chrono::duration_cast<std::chrono::microseconds>((t_3-t_2)).count();
   time3 = std::chrono::duration_cast<std::chrono::microseconds>((t_4-t_3)).count();
   time4 = std::chrono::duration_cast<std::chrono::microseconds>((t_5-t_4)).count();
   ```

2. **Time printing**:
   ```cpp
   std::cout << "time1~time4:" << std::endl;
   std::cout << time1 << std::endl;
   std::cout << time2 << std::endl;
   std::cout << time3 << std::endl;
   std::cout << time4 << std::endl;
   std::cout << "End: time1~time4:" << std::endl;
   ```

3. **Average calculation**:
   ```cpp
   long long sum = 0;
   for (size_t i = 0; i < time_vect.size(); i++) {
       sum += time_vect[i];
   }
   return sum / time_vect.size();
   ```

### Solution
Create `TimingHelper` class:
```cpp
class TimingHelper {
public:
    struct TimingPhases {
        int64_t allocation_time;
        int64_t initialization_time;
        int64_t execution_time;
        int64_t cleanup_time;
    };
    
    static TimingPhases calculate_phases(/* time points */);
    static void print_phases(const TimingPhases& phases);
    static long long calculate_average(const std::vector<long long>& times);
};
```

### Impact
- **Before**: ~500 lines of duplicated timing code
- **After**: ~50 lines of helper class
- **Reduction**: ~450 lines (~90% reduction)

---

## 4. Predictor Initialization (`src/library_frame/BLAS/src/interfaces-ml.cpp`)

### Problem
Repeated predictor initialization and thread setting code appears **18+ times**.

### Duplication Details
- **Pattern repeated**: 18+ times
- **Duplicated code per instance**: ~15 lines
- **Total duplication**: ~300 lines

### Duplicated Elements
```cpp
Predictor predictor("xgb", routine_name);
int nt;
if (useML) {
    nt = predictor.model->predict_num_cores(m, k, n, routine_name);
    omp_set_num_threads(nt);
} else {
    nt = blas_config::Config::getInstance().getPredictorMaxThreads();
    omp_set_num_threads(nt);
}
std::cout << "Number of threads: " << nt << std::endl;
```

### Solution
Create `ThreadManager` class:
```cpp
class ThreadManager {
public:
    static int get_optimal_threads(const std::string& routine_name, 
                                    int m, int k, int n, bool useML);
    static void set_threads(int num_threads);
};
```

### Impact
- **Before**: ~300 lines of duplicated predictor code
- **After**: ~50 lines of helper class
- **Reduction**: ~250 lines (~83% reduction)

---

## 5. Level 1/2 Routines Memory Management (`src/library_frame/BLAS/src/interfaces-ml.cpp`)

### Problem
Level 1 (dot, axpy) and Level 2 (gemv, syr, trsv) routines still use unsafe `malloc`/`free`/`memalign`.

### Duplication Details
- **Routines affected**: 5 routines (dot, axpy, gemv, syr, trsv)
- **Duplicated pattern**: Memory allocation, initialization, cleanup
- **Total duplication**: ~200 lines
- **Risk**: Memory leaks, inconsistent with Level 3 routines

### Example Pattern
```cpp
// Repeated in dot, axpy, gemv, syr, trsv
float *x = (float*)memalign(128, static_cast<size_t>(num_of_duplicate)*n*sizeof(float));
float *y = (float*)memalign(128, static_cast<size_t>(num_of_duplicate)*n*sizeof(float));

// ... initialization loops ...

free(x);
free(y);
```

### Solution
Replace with `std::vector` (consistent with Level 3):
```cpp
std::vector<float> x(static_cast<size_t>(num_of_duplicate) * n);
std::vector<float> y(static_cast<size_t>(num_of_duplicate) * n);
// Automatic cleanup, no free() needed
```

### Impact
- **Before**: ~200 lines using unsafe malloc/free
- **After**: ~200 lines using safe std::vector
- **Risk reduction**: Eliminates memory leak risk
- **Consistency**: Matches Level 3 routine patterns

---

## 6. Level 1/2 Routines Float/Double Duplication (`src/library_frame/BLAS/src/interfaces-ml.cpp`)

### Problem
Level 1/2 routines have separate float and double implementations (similar to Level 3 before refactoring).

### Duplication Details
- **Routines affected**: 5 routines × 2 types = 10 implementations
- **Duplicated code**: ~400 lines
- **Pattern**: Nearly identical code with only type differences

### Solution
Create template implementations (same pattern as Level 3):
```cpp
template<typename T>
long long run_compare_dot_impl(int n, bool useML, size_t num_of_duplicate, test_class* lib);

// Wrapper functions
long long run_compare_dot_(int n, bool isDouble, bool useML, size_t num_of_duplicate, test_class* lib) {
    if (isDouble) return run_compare_dot_impl<double>(n, useML, num_of_duplicate, lib);
    else return run_compare_dot_impl<float>(n, useML, num_of_duplicate, lib);
}
```

### Impact
- **Before**: ~400 lines of float/double duplication
- **After**: ~200 lines of template code
- **Reduction**: ~200 lines (~50% reduction)

---

## Summary Statistics

### Current State
| Category | Duplication | Instances | Lines |
|----------|-------------|-----------|-------|
| Context classes | Method declarations | 5 classes | ~500 |
| Matrix filling | Pthread patterns | 43+ | ~2000 |
| Timing code | Measurement/reporting | 24+ | ~500 |
| Predictor init | Initialization logic | 18+ | ~300 |
| Level 1/2 malloc | Memory management | 5 routines | ~200 |
| Level 1/2 types | Float/double | 5 routines | ~400 |
| **TOTAL** | | | **~3900** |

### After Refactoring
| Category | New Code | Reduction |
|----------|----------|-----------|
| Context classes | ~300 lines | ~200 lines |
| Matrix filling | ~100 lines | ~1900 lines |
| Timing code | ~50 lines | ~450 lines |
| Predictor init | ~50 lines | ~250 lines |
| Level 1/2 malloc | ~200 lines | 0 (safety improvement) |
| Level 1/2 types | ~200 lines | ~200 lines |
| **TOTAL** | **~900 lines** | **~3000 lines** |

### Net Impact
- **Code reduction**: ~3000 lines eliminated
- **New reusable code**: ~900 lines
- **Net reduction**: ~2100 lines (~30% reduction in interfaces-ml.cpp)
- **Maintainability**: Significantly improved
- **Consistency**: All routines use same patterns
- **Safety**: Eliminated memory leak risks

---

## Priority Recommendations

### High Priority (Immediate Impact)
1. **Extract matrix filling helpers** - Eliminates 1900 lines, reduces error risk
2. **Replace malloc/free in Level 1/2** - Eliminates memory leak risk
3. **Extract timing helpers** - Eliminates 450 lines, ensures consistency

### Medium Priority (Significant Impact)
4. **Extract predictor helpers** - Eliminates 250 lines, centralizes logic
5. **Template Level 1/2 routines** - Eliminates 200 lines, matches Level 3 pattern

### Lower Priority (Architectural Improvement)
6. **Refactor context classes** - Eliminates 200 lines, improves architecture

---

## Implementation Order

1. **Phase 4.5**: Replace malloc/free (safety first)
2. **Phase 4.2**: Extract matrix filling helpers (biggest impact)
3. **Phase 4.3**: Extract timing helpers (quick win)
4. **Phase 4.4**: Extract predictor helpers (centralize logic)
5. **Phase 4.6**: Template Level 1/2 routines (consistency)
6. **Phase 4.1**: Refactor context classes (architectural improvement)

