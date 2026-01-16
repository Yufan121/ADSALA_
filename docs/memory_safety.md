# Memory Safety Testing

This document describes the memory safety testing infrastructure implemented for the ADSALA-AMD platform.

## Overview

Memory safety is critical for high-performance computing applications. The memory safety tests verify that:

- No memory leaks occur during BLAS operations
- Memory allocations and deallocations are properly handled
- Edge cases don't cause memory corruption
- Memory alignment requirements are met

## Test Structure

### Integration Tests (`tests/integration/test_memory_safety.cpp`)

The memory safety tests are located in `tests/integration/test_memory_safety.cpp` and include:

#### Test Cases

1. **MatrixAllocationDeallocation**: Tests basic memory allocation/deallocation
2. **LargeMatrixOperations**: Stress tests with large matrices (1000x1000)
3. **BLASContextMemorySafety**: Tests actual BLAS operations (DGEMM) for memory leaks
4. **Level1OperationsMemorySafety**: Tests Level 1 BLAS operations (DDOT, DAXPY)
5. **EdgeCaseAllocations**: Tests edge cases like zero-sized allocations
6. **MemoryAlignment**: Verifies 64-byte memory alignment requirements
7. **MultipleAllocations**: Stress test with many allocations/deallocations

#### Test Features

- **Aligned Memory**: Uses `posix_memalign` for 64-byte aligned allocations (matching the actual code)
- **BLAS Integration**: Tests with real BLAS libraries (BLIS, MKL when available)
- **Comprehensive Coverage**: Tests all major memory operations in the codebase
- **Stress Testing**: Large matrices and multiple allocation patterns

## Running Memory Safety Tests

### Prerequisites

- **Valgrind**: For memory leak detection
  ```bash
  # Ubuntu/Debian
  sudo apt install valgrind

  # macOS
  brew install valgrind
  ```

- **AddressSanitizer**: Built into modern compilers (GCC 4.8+, Clang 3.1+)

### Test Runner Script

Use the provided script for easy testing:

```bash
# Run regular tests (no memory checking)
./scripts/run_memory_tests.sh regular

# Run with Valgrind memory leak detection
./scripts/run_memory_tests.sh valgrind

# Run with AddressSanitizer
./scripts/run_memory_tests.sh asan

# Run all memory safety tests
./scripts/run_memory_tests.sh all
```

### Manual Testing

#### Valgrind
```bash
cd build
valgrind --leak-check=full --show-leak-kinds=all \
         --track-origins=yes --verbose \
         ./tests/integration/test_memory_safety
```

#### AddressSanitizer
```bash
# Build with ASAN
mkdir build_asan
cd build_asan
cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
         -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
make

# Run tests
./tests/integration/test_memory_safety
```

## Memory Safety Features Tested

### Memory Allocation
- Proper use of `posix_memalign` for aligned allocations
- Correct alignment to 64-byte boundaries
- Proper cleanup of allocated memory

### BLAS Operations
- Memory safety during matrix operations
- Proper handling of input/output buffers
- No buffer overflows or underflows

### Edge Cases
- Zero-sized allocations
- Very large matrices
- Multiple concurrent allocations

### Stress Testing
- High allocation/deallocation frequency
- Large memory footprints
- Memory fragmentation scenarios

## Interpreting Results

### Valgrind Output
- **No leaks reported**: All memory properly freed
- **Definitely lost**: Memory allocated but never freed
- **Indirectly lost**: Memory lost through pointers
- **Possibly lost**: Memory that might be leaked

### AddressSanitizer Output
- **Heap buffer overflow/underflow**: Accessing memory outside allocated bounds
- **Use after free**: Accessing freed memory
- **Memory leaks**: Unfreed allocations

## Best Practices

1. **Always run memory tests** before major releases
2. **Fix all memory issues** before merging code changes
3. **Monitor memory usage** in production environments
4. **Use the test script** for consistent testing across environments

## Integration with CI/CD

The memory safety tests can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions
- name: Memory Safety Tests
  run: |
    ./scripts/run_memory_tests.sh valgrind
    ./scripts/run_memory_tests.sh asan
```

## Troubleshooting

### Common Issues

1. **Valgrind not available**: Install valgrind package
2. **ASAN false positives**: Some libraries may report false leaks
3. **Performance impact**: Memory checking tools slow down execution significantly
4. **Large memory requirements**: Tests may require substantial RAM for large matrices

### Debugging Memory Issues

1. **Use Valgrind with `--track-origins=yes`** to find allocation sources
2. **Enable ASAN with `--asan-stack=1`** for stack traces
3. **Check allocation/deallocation balance** in test code
4. **Use heap profilers** for detailed memory analysis

## Future Enhancements

- **Thread safety testing**: Memory safety in multi-threaded environments
- **GPU memory testing**: Memory safety for CUDA operations
- **Long-running tests**: Memory leak detection over extended periods
- **Performance regression testing**: Memory usage impact analysis
