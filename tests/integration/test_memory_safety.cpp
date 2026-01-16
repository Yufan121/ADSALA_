#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <memory>
#include <cstdlib>
#include <cstring>
#include "library-manager.h"
#include "blas_config.h"

// Helper function to allocate aligned memory (simulating the matrix allocation in the code)
double* allocate_matrix(size_t size) {
    void* ptr = nullptr;
    int ret = posix_memalign(&ptr, 64, size * sizeof(double)); // 64-byte alignment
    if (ret != 0) {
        throw std::bad_alloc();
    }
    return static_cast<double*>(ptr);
}

// Helper function to free aligned memory
void free_matrix(double* ptr) {
    free(ptr);
}

// Test fixture for memory safety tests
class MemorySafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset config for clean tests
        blas_config::Config::getInstance().reset();
    }

    void TearDown() override {
        // Cleanup if needed
    }

    // Helper to create a simple test matrix
    void create_test_matrix(double*& matrix, size_t rows, size_t cols, double value = 1.0) {
        size_t size = rows * cols;
        matrix = allocate_matrix(size);
        for (size_t i = 0; i < size; ++i) {
            matrix[i] = value;
        }
    }

    // Helper to verify matrix contents
    void verify_matrix(const double* matrix, size_t rows, size_t cols, double expected_value) {
        size_t size = rows * cols;
        for (size_t i = 0; i < size; ++i) {
            EXPECT_DOUBLE_EQ(matrix[i], expected_value);
        }
    }
};

// Test: Memory allocation and deallocation
TEST_F(MemorySafetyTest, MatrixAllocationDeallocation) {
    const size_t rows = 100;
    const size_t cols = 200;
    double* matrix = nullptr;

    // Test allocation
    EXPECT_NO_THROW({
        matrix = allocate_matrix(rows * cols);
        ASSERT_NE(matrix, nullptr);
    });

    // Test deallocation
    EXPECT_NO_THROW({
        free_matrix(matrix);
        matrix = nullptr;
    });
}

// Test: Large matrix operations (stress test for memory)
TEST_F(MemorySafetyTest, LargeMatrixOperations) {
    const size_t size = 1000; // 1000x1000 matrix
    double* A = nullptr;
    double* B = nullptr;
    double* C = nullptr;

    // Allocate large matrices
    EXPECT_NO_THROW({
        A = allocate_matrix(size * size);
        B = allocate_matrix(size * size);
        C = allocate_matrix(size * size);
    });

    // Initialize matrices
    memset(A, 0, size * size * sizeof(double));
    memset(B, 0, size * size * sizeof(double));
    memset(C, 0, size * size * sizeof(double));

    // Fill with test data
    for (size_t i = 0; i < size * size; ++i) {
        A[i] = 1.0;
        B[i] = 2.0;
        C[i] = 0.0;
    }

    // Cleanup
    free_matrix(A);
    free_matrix(B);
    free_matrix(C);
}

// Test: BLAS context memory safety with actual operations
TEST_F(MemorySafetyTest, BLASContextMemorySafety) {
    // Get a BLAS library (preferably BLIS or MKL which are fully implemented)
    library_manager lm;
    std::vector<test_class*> libraries = lm.get_libs();

    ASSERT_FALSE(libraries.empty());

    // Find a fully implemented library (BLIS or MKL)
    test_class* lib = nullptr;
    for (auto* test_lib : libraries) {
        if (test_lib->name == "blis" || test_lib->name == "mkl") {
            lib = test_lib;
            break;
        }
    }

    if (lib == nullptr) {
        GTEST_SKIP() << "No fully implemented BLAS library found for memory testing";
        return;
    }

    // Test matrix dimensions
    const int m = 50, n = 60, k = 70;

    // Allocate matrices
    double* A = allocate_matrix(m * k);
    double* B = allocate_matrix(k * n);
    double* C = allocate_matrix(m * n);

    // Initialize matrices
    for (int i = 0; i < m * k; ++i) A[i] = 1.0;
    for (int i = 0; i < k * n; ++i) B[i] = 2.0;
    for (int i = 0; i < m * n; ++i) C[i] = 0.0;

    // Perform DGEMM operation
    EXPECT_NO_THROW({
        lib->dgemm(m, n, k, 1.0, A, B, 0.0, C);
    });

    // Verify result (C should be 2.0 * k = 140.0)
    for (int i = 0; i < m * n; ++i) {
        EXPECT_DOUBLE_EQ(C[i], 140.0);
    }

    // Cleanup
    free_matrix(A);
    free_matrix(B);
    free_matrix(C);
}

// Test: Level 1 operations memory safety
TEST_F(MemorySafetyTest, Level1OperationsMemorySafety) {
    library_manager lm;
    std::vector<test_class*> libraries = lm.get_libs();

    ASSERT_FALSE(libraries.empty());

    test_class* lib = nullptr;
    for (auto* test_lib : libraries) {
        if (test_lib->name == "blis" || test_lib->name == "mkl") {
            lib = test_lib;
            break;
        }
    }

    if (lib == nullptr) {
        GTEST_SKIP() << "No fully implemented BLAS library found for memory testing";
        return;
    }

    const int n = 100;

    // Allocate vectors
    double* x = allocate_matrix(n);
    double* y = allocate_matrix(n);

    // Initialize vectors
    for (int i = 0; i < n; ++i) {
        x[i] = 1.0;
        y[i] = 2.0;
    }

    // Test DDOT
    EXPECT_NO_THROW({
        lib->ddot(n, x, y);
    });

    // Test DAXPY
    EXPECT_NO_THROW({
        lib->daxpy(n, 2.0, x, y);
    });

    // Cleanup
    free_matrix(x);
    free_matrix(y);
}

// Test: Edge cases for memory allocation
TEST_F(MemorySafetyTest, EdgeCaseAllocations) {
    // Test zero-sized allocation (should not crash)
    EXPECT_NO_THROW({
        double* zero_matrix = allocate_matrix(0);
        free_matrix(zero_matrix);
    });

    // Test very small allocations
    EXPECT_NO_THROW({
        double* small_matrix = allocate_matrix(1);
        small_matrix[0] = 42.0;
        EXPECT_DOUBLE_EQ(small_matrix[0], 42.0);
        free_matrix(small_matrix);
    });
}

// Test: Memory alignment verification
TEST_F(MemorySafetyTest, MemoryAlignment) {
    const size_t alignment = 64; // 64-byte alignment requirement

    double* matrix = allocate_matrix(100);
    ASSERT_NE(matrix, nullptr);

    // Check alignment
    uintptr_t addr = reinterpret_cast<uintptr_t>(matrix);
    EXPECT_EQ(addr % alignment, 0U) << "Matrix not properly aligned to " << alignment << " bytes";

    free_matrix(matrix);
}

// Test: Multiple allocations and deallocations (stress test)
TEST_F(MemorySafetyTest, MultipleAllocations) {
    const int num_allocations = 100;
    const size_t matrix_size = 1000;

    std::vector<double*> matrices;

    // Allocate many matrices
    for (int i = 0; i < num_allocations; ++i) {
        double* matrix = allocate_matrix(matrix_size);
        ASSERT_NE(matrix, nullptr);
        matrices.push_back(matrix);
    }

    // Fill and verify each matrix
    for (int i = 0; i < num_allocations; ++i) {
        double* matrix = matrices[i];
        for (size_t j = 0; j < matrix_size; ++j) {
            matrix[j] = static_cast<double>(i + j);
        }
        EXPECT_DOUBLE_EQ(matrix[0], static_cast<double>(i));
    }

    // Deallocate all matrices
    for (double* matrix : matrices) {
        free_matrix(matrix);
    }

    matrices.clear();
}

} // namespace
