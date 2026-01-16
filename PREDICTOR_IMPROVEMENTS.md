# Predictor Module Improvement Plan

## Overview
The `predictor.c` and `predictor.hpp` files contain hardcoded configuration values and use outdated memory management patterns. This plan outlines improvements to integrate with the unified YAML configuration system and modernize the codebase.

## Issues Identified

### 1. Hardcoded Configuration Values
- **Location**: `predictor.c` lines 34-37
- **Problem**: 
  - `max_num_threads = 48*2` (hardcoded default)
  - `max_feature = 17` (hardcoded default)
  - `feature_num = 5` (hardcoded default)
- **Impact**: Values are read from INI files but have hardcoded defaults that conflict with the main config system
- **Solution**: Integrate with `blas_config::Config` singleton, remove hardcoded defaults

### 2. Config File Format Mismatch
- **Location**: `predictor.c` line 312 (`load_config()`)
- **Problem**: Uses INI file format via `config-read.h`, while main system uses YAML
- **Impact**: Two different config systems, inconsistent configuration management
- **Solution**: 
  - Option A: Migrate predictor config to YAML (preferred)
  - Option B: Keep INI for model-specific config, but read `max_num_threads` from YAML config

### 3. Memory Management Issues
- **Location**: Multiple locations in `predictor.c` and `predictor.hpp`
- **Problems**:
  - `malloc`/`free` in C code (lines 223, 253, 286, 315-319, 365, 528, 562, 596)
  - `new`/`delete` in C++ code (`predictor.hpp` lines 149-152, 164-165, etc.)
  - No RAII patterns
  - Potential memory leaks if exceptions occur
- **Solution**: 
  - C code: Keep `malloc`/`free` but add proper error handling
  - C++ code: Replace `new`/`delete` with smart pointers (`std::unique_ptr`, `std::vector`)

### 4. Global Variables
- **Location**: `predictor.c` lines 69-73
- **Problem**: Static global variables (`booster`, `ConfigInfo`, `list_feature_index`, `memory`)
- **Impact**: Difficult to test, not thread-safe, poor encapsulation
- **Solution**: Encapsulate in a context structure or class

### 5. Error Handling
- **Location**: `predictor.c` line 22-28 (`safe_xgboost` macro)
- **Problem**: Uses `exit(1)` which terminates the program
- **Impact**: Cannot recover from errors, difficult to test
- **Solution**: Replace with exception handling (C++) or error return codes (C)

### 6. Variable Name Conflicts
- **Location**: `interfaces-ml.cpp` uses `max_num_threads` from predictor.c
- **Problem**: `max_num_threads` in predictor.c conflicts with `max_num_threads` in config system
- **Impact**: Confusion, potential bugs
- **Solution**: Rename predictor's `max_num_threads` to `predictor_max_threads` or use config system directly

## Proposed Improvements

### Phase 1: Configuration Integration
1. **Add predictor config to YAML**:
   ```yaml
   predictor:
     max_num_threads: 96  # Maximum threads for prediction
     max_feature: 17       # Maximum number of features
     feature_num: 5        # Number of features used
   ```

2. **Create C++ wrapper for config access**:
   - Add `getPredictorMaxThreads()`, `getMaxFeature()`, `getFeatureNum()` to `blas_config::Config`
   - Provide C interface function to access these values

3. **Update `load_config()` function**:
   - Read `max_num_threads` from YAML config (via C interface)
   - Keep INI reading for model-specific parameters (data_mean_, data_std_, etc.)
   - Remove hardcoded defaults

### Phase 2: Memory Management Modernization
1. **C++ code (`predictor.hpp`)**:
   - Replace `char* model_name` and `char* config_name` with `std::string`
   - Use `std::unique_ptr` for model pointers
   - Use `std::vector` for arrays

2. **C code (`predictor.c`)**:
   - Add proper error checking for all `malloc` calls
   - Return error codes instead of using `exit()`
   - Document memory ownership

### Phase 3: Error Handling
1. **Replace `safe_xgboost` macro**:
   - Create function that returns error code
   - Let caller handle errors (C++ can throw exceptions)

2. **Add error propagation**:
   - Return error codes from C functions
   - Throw exceptions from C++ wrappers

### Phase 4: Encapsulation
1. **Create predictor context structure**:
   ```c
   typedef struct {
       BoosterHandle booster;
       PreprocessInfo config_info;
       int* list_feature_index;
       MemoryStruct memory;
   } PredictorContext;
   ```

2. **Pass context to functions**:
   - Remove global variables
   - Make functions thread-safe
   - Enable multiple predictor instances

## Implementation Priority

### High Priority (Critical)
1. ✅ Remove hardcoded `max_num_threads` and integrate with config system
2. ✅ Fix `max_num_threads` conflict between predictor and main config
3. ✅ Replace `exit(1)` with proper error handling

### Medium Priority (Important)
4. Replace `new`/`delete` with smart pointers in C++ code
5. Add proper error checking for memory allocation
6. Migrate predictor config to YAML (or at least `max_num_threads`)

### Low Priority (Nice to have)
7. Encapsulate global variables in context structure
8. Add comprehensive error handling throughout
9. Improve code documentation

## Testing Requirements
- Unit tests for config integration
- Memory leak tests (valgrind/AddressSanitizer)
- Error handling tests
- Integration tests with config system

## Backward Compatibility
- Keep INI file reading for model-specific parameters (data_mean_, data_std_, etc.)
- Maintain existing function signatures where possible
- Add new functions rather than breaking existing ones

