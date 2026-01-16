# Codebase Refactoring and Cleanup Plan

## Overview
This plan addresses structural issues, redundancies, and organization problems in the ADSALA-AMD platform codebase to create a cleaner, more maintainable structure.

## Current Issues Identified

### 1. Directory Structure Problems
- **Scattered experimental directories**: `real 500M new/`, `real 500M new Gadi/`, `run_gadi_dot/`, `run_gadi_dot_new/`
- **Build artifacts in source**: `data_collect/lib/`, `data_collect/real 500M new Gadi/CMakeFiles/`
- **Legacy code mixed with active**: `legacy/`, `resources_old/`
- **Duplicate test directories**: `library_frame/test/` and `tests/`
- **OS artifacts**: `__MACOSX/` directory
- **Spaces in directory names**: Makes scripting and automation difficult

### 2. File Redundancies
- Backup files: `CMakeLists.txt.orig`
- Build artifacts: `CMakeCache.txt`, `CMakeFiles/`, `.o` files in source directories
- Compiled Python: `*.pyc` files, `__pycache__/` directories
- Duplicate files: `diagram.svg` in root and `images/`
- Multiple model/config files: `xgb_*.ini`, `xgb_*.model` scattered in `model_training/`
- Job output files: `*.sh.o*` files

### 3. Naming Inconsistencies
- Spaces in directory names (`real 500M new`)
- Mixed naming conventions (camelCase vs snake_case vs kebab-case)
- Inconsistent file naming patterns

### 4. Code Organization Issues
- Scripts scattered across root and subdirectories
- No clear separation: source/build/test/data
- Configuration files mixed with source code
- Test code in multiple locations

### 5. Code Duplication Issues

#### 5.1: Context Classes Duplication (`src/data_collect/`)
- **Problem**: Multiple BLAS context classes (BLAS-context, BLIS-context, MKL-context, OpenBLAS-context, cuBLAS-context) with nearly identical structure
- **Duplicated Elements**:
  - Identical method signatures across all classes (dgemm, sgemm, dsymm, ssymm, etc.)
  - Similar constructor/destructor patterns
  - Similar implementation patterns (cblas_* calls with same parameters)
  - Duplicated header declarations (48+ method declarations per class)
- **Impact**: 
  - Maintenance burden: changes must be replicated across 5+ files
  - Inconsistency risk: implementations may diverge
  - Code bloat: ~500+ lines of duplicated declarations/implementations
- **Solution**: Create base template class or use CRTP pattern to eliminate duplication

#### 5.2: Matrix Filling Patterns (`src/library_frame/BLAS/src/interfaces-ml.cpp`)
- **Problem**: Repeated pthread creation/join patterns for matrix initialization
- **Duplicated Elements**:
  - pthread_t array allocation (repeated 43+ times)
  - Thread creation loop pattern (repeated 43+ times)
  - Thread join loop pattern (repeated 43+ times)
  - Stride calculation logic (repeated 43+ times)
  - Error handling for pthread_create (repeated 43+ times)
- **Impact**: 
  - ~2000+ lines of duplicated pthread code
  - Error-prone: easy to miss error handling in one location
  - Hard to maintain: changes require updates in many places
- **Solution**: Extract helper functions: `fill_matrix_parallel()`, `fill_symmetric_matrix_parallel()`
- **Status**: ✅ COMPLETED - Created helper functions and replaced all 43+ instances

#### 5.3: Timing Code Duplication (`src/library_frame/BLAS/src/interfaces-ml.cpp`)
- **Problem**: Repeated timing measurement and reporting code
- **Duplicated Elements**:
  - time1~time4 calculation (repeated 24+ times)
  - time1~time4 printing (repeated 24+ times)
  - Average calculation loop (repeated 24+ times)
- **Impact**: 
  - ~500+ lines of duplicated timing code
  - Inconsistent timing reporting
- **Solution**: Extract `TimingHelper` class or helper functions

#### 5.4: Predictor Initialization (`src/library_frame/BLAS/src/interfaces-ml.cpp`)
- **Problem**: Repeated predictor initialization and thread setting code
- **Duplicated Elements**:
  - Predictor object creation (repeated 18+ times)
  - ML vs non-ML branch logic (repeated 18+ times)
  - omp_set_num_threads calls (repeated 18+ times)
  - "Number of threads" print statement (repeated 18+ times)
- **Impact**: 
  - ~300+ lines of duplicated code
  - Inconsistent thread management
- **Solution**: Extract `ThreadManager` or `PredictorHelper` class

#### 5.5: Level 1/2 Routines Still Use malloc/free (`src/library_frame/BLAS/src/interfaces-ml.cpp`)
- **Problem**: Level 1 (dot, axpy) and Level 2 (gemv, syr, trsv) routines still use `malloc`/`free`/`memalign`
- **Duplicated Elements**:
  - Memory allocation with memalign (repeated 5+ times)
  - Manual memory initialization loops (repeated 5+ times)
  - free() calls (repeated 5+ times)
- **Impact**: 
  - Memory leak risk
  - Inconsistent with Level 3 routines (which use std::vector)
  - ~200+ lines of unsafe memory management
- **Solution**: Replace malloc/free with std::vector, consistent with Level 3 routines
- **Status**: ✅ COMPLETED - Replaced all malloc/free with std::vector in Level 1/2 routines

#### 5.6: Level 1/2 Routines Float/Double Duplication (`src/library_frame/BLAS/src/interfaces-ml.cpp`)
- **Problem**: Level 1/2 routines have separate float and double implementations (similar to Level 3 before refactoring).
- **Duplicated code**: ~400 lines
- **Pattern**: Nearly identical code with only type differences
- **Solution**: Create template implementations (same pattern as Level 3)

## Proposed Directory Structure

```
ADSALA-AMD_platform/
├── CMakeLists.txt
├── README.md
├── config.yaml
├── requirements.txt
├── .gitignore
│
├── src/                          # All source code
│   ├── data_collect/             # Data collection module
│   │   ├── CMakeLists.txt
│   │   ├── include/              # Headers
│   │   │   ├── BLAS-context.h
│   │   │   ├── BLIS-context.h
│   │   │   ├── interfaces.h
│   │   │   ├── library-manager.h
│   │   │   └── ...
│   │   └── src/                  # Implementation
│   │       ├── BLAS-context.cpp
│   │       ├── main.cpp
│   │       └── ...
│   │
│   ├── library_frame/            # Library implementation
│   │   ├── CMakeLists.txt
│   │   ├── BLAS/
│   │   │   ├── include/
│   │   │   └── src/
│   │   ├── predictor/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── predictor.c
│   │   │   ├── predictor.hpp
│   │   │   └── config-read.h
│   │   └── README.md
│   │
│   └── model_training/           # Python training code
│       ├── main.py
│       ├── README.md
│       └── resources/
│           └── load_data.py
│
├── tests/                        # All test code (unified)
│   ├── CMakeLists.txt
│   ├── unit/                     # Unit tests
│   ├── integration/              # Integration tests
│   └── data/                     # Test data
│
├── scripts/                      # All scripts organized
│   ├── build/                    # Build scripts
│   │   ├── setup-build.sh
│   │   └── setup-build-Gadi.sh
│   ├── run/                      # Runtime scripts
│   │   ├── setup-run.sh
│   │   └── batch.sh
│   └── data/                     # Data processing scripts
│       └── combine_domains.sh
│
├── data/                         # Data directories
│   ├── experiments/              # Experimental data
│   │   ├── gadi_500M/
│   │   ├── setonix_500M/
│   │   └── dot_experiments/
│   ├── models/                   # Trained models
│   │   ├── xgb_dgemm.model
│   │   ├── xgb_sgemm.model
│   │   └── ...
│   └── configs/                  # Model configs
│       ├── xgb_dgemm.ini
│       └── ...
│
├── build/                        # Build directory (gitignored)
│   └── ...
│
├── docs/                         # Documentation
│   ├── architecture.md
│   ├── installation.md
│   └── api/
│
└── legacy/                       # Archived/legacy code
    ├── old_experiments/
    └── deprecated_code/
```

## Refactoring Tasks

### Phase 1: Cleanup and Removal (Low Risk) ✅ COMPLETE

#### Task 1.1: Remove Build Artifacts ✅
- [x] Remove `data_collect/lib/` (build artifacts)
- [x] Remove `data_collect/real 500M new Gadi/CMakeFiles/`
- [x] Remove `data_collect/real 500M new Gadi/CMakeCache.txt`
- [x] Remove all `*.o` files from source directories
- [x] Remove `CMakeLists.txt.orig`
- [x] Remove `__MACOSX/` directory
- [x] Update `.gitignore` to prevent future commits

#### Task 1.2: Remove Compiled Python Files ✅
- [x] Remove all `*.pyc` files
- [x] Remove all `__pycache__/` directories
- [x] Ensure `.gitignore` includes these patterns

#### Task 1.3: Remove Job Output Files ✅
- [x] Remove `*.sh.o*` files (SLURM job outputs)
- [x] Add `*.sh.o*` to `.gitignore`

#### Task 1.4: Remove Duplicate Files ✅
- [x] Remove duplicate `diagram.svg` (keep one in `images/` or `docs/`)
- [x] Consolidate duplicate scripts

### Phase 2: Directory Reorganization (Medium Risk) ✅ COMPLETE

#### Task 2.1: Consolidate Experimental Directories ✅
- [x] Move `real 500M new/` → `data/experiments/gadi_500M_v1/`
- [x] Move `real 500M new Gadi/` → `data/experiments/gadi_500M_v2/`
- [x] Move `run_gadi_dot/` → `data/experiments/gadi_dot_v1/`
- [x] Move `run_gadi_dot_new/` → `data/experiments/gadi_dot_v2/`
- [x] Update all references in scripts and documentation

#### Task 2.2: Organize Scripts ✅
- [x] Create `scripts/` directory structure
- [x] Move `setup-build*.sh` → `scripts/build/`
- [x] Move `setup-run.sh` → `scripts/run/`
- [x] Move `tar-cmd.sh`, `move_files.sh` → `scripts/utils/`
- [x] Move `combine_domains*.sh` → `scripts/data/`
- [x] Move `script_creater.sh` → `scripts/utils/`
- [x] Update all script paths in documentation

#### Task 2.3: Organize Model Files ✅
- [x] Create `data/models/` directory
- [x] Move all `*.model` files → `data/models/`
- [x] Create `data/configs/` directory
- [x] Move all `*.ini` files → `data/configs/`
- [x] Update code references to model/config paths

#### Task 2.4: Consolidate Test Directories ✅
- [x] Move `library_frame/test/` → `tests/legacy/` (archive)
- [x] Keep unified `tests/` directory structure
- [x] Update CMakeLists.txt references

#### Task 2.5: Move Legacy Code ✅
- [x] Move `data_collect/legacy/` → `legacy/deprecated_code/`
- [x] Move `model_selection/resources_old/` → `legacy/deprecated_code/`
- [x] Add README in legacy explaining what's archived

### Phase 3: Source Code Reorganization (Higher Risk) ✅ COMPLETE

#### Task 3.1: Reorganize Source Structure ✅
- [x] Create `src/` directory
- [x] Move `data_collect/` → `src/data_collect/`
- [x] Move `library_frame/` → `src/library_frame/`
- [x] Move `model_training/` → `src/model_training/`
- [x] Update all CMakeLists.txt paths
- [x] Update include paths in source files

#### Task 3.2: Separate Headers and Sources
- [ ] Create `data_collect/include/` for headers
- [ ] Move `*.h` files → `include/`
- [ ] Move `*.cpp` files → `src/`
- [ ] Update include paths

#### Task 3.3: Create Documentation Structure
- [ ] Create `docs/` directory
- [ ] Move `README.md` content → `docs/` (keep root README as overview)
- [ ] Create `docs/architecture.md`
- [ ] Create `docs/installation.md`
- [ ] Create `docs/api/` for API documentation

### Phase 4: Code Duplication Elimination (High Priority)

#### Task 4.1: Refactor Context Classes (`src/data_collect/`)
- [ ] Create base template class `BaseBLASContext<T>` for common functionality
- [ ] Use CRTP (Curiously Recurring Template Pattern) for type-specific implementations
- [ ] Extract common BLAS call patterns into helper functions
- [ ] Reduce ~500 lines of duplication to ~100 lines of shared code
- [ ] Update `library-manager.cpp` to work with new structure
- **Estimated Impact**: Reduce code by ~400 lines, improve maintainability

#### Task 4.2: Extract Matrix Filling Helpers (`src/library_frame/BLAS/src/interfaces-ml.cpp`) ✅ COMPLETE
- [x] Create `fill_matrix_parallel()` template function
- [x] Create `fill_symmetric_matrix_parallel()` helper function
- [x] Extract stride calculation into helper
- [x] Replace 43+ duplicated pthread blocks with function calls
- [x] Fix memory leaks by properly freeing malloc'd structures
- **Impact**: Reduced code by ~1850 lines (~92% reduction)

#### Task 4.3: Extract Timing Helper (`src/library_frame/BLAS/src/interfaces-ml.cpp`)
- [ ] Create `TimingHelper` class or namespace
- [ ] Extract time1~time4 calculation into `calculate_timing_phases()`
- [ ] Extract time1~time4 printing into `print_timing_phases()`
- [ ] Extract average calculation into `calculate_average_time()`
- [ ] Replace 24+ duplicated timing blocks
- **Estimated Impact**: Reduce code by ~450 lines, ensure consistent timing

#### Task 4.4: Extract Predictor/Thread Management (`src/library_frame/BLAS/src/interfaces-ml.cpp`)
- [ ] Create `ThreadManager` or `PredictorHelper` class
- [ ] Extract predictor initialization logic
- [ ] Extract ML vs non-ML branch into single function
- [ ] Extract thread setting logic
- [ ] Replace 18+ duplicated predictor blocks
- **Estimated Impact**: Reduce code by ~250 lines, centralize thread management

#### Task 4.5: Replace malloc/free in Level 1/2 Routines (`src/library_frame/BLAS/src/interfaces-ml.cpp`) ✅ COMPLETE
- [x] Replace `malloc`/`memalign` with `std::vector` in `run_compare_dot_`
- [x] Replace `malloc`/`memalign` with `std::vector` in `run_compare_axpy_`
- [x] Replace `malloc`/`memalign` with `std::vector` in `run_compare_gemv_`
- [x] Replace `malloc`/`memalign` with `std::vector` in `run_compare_syr_`
- [x] Replace `malloc`/`memalign` with `std::vector` in `run_compare_trsv_`
- [x] Remove all `free()` calls
- **Impact**: Eliminated memory leak risk, consistent with Level 3 routines

#### Task 4.6: Template Level 1/2 Routines (`src/library_frame/BLAS/src/interfaces-ml.cpp`)
- [ ] Create template implementations for Level 1 routines (dot, axpy)
- [ ] Create template implementations for Level 2 routines (gemv, syr, trsv)
- [ ] Eliminate float/double duplication (similar to Level 3)
- [ ] Use same template pattern as Level 3 routines
- **Estimated Impact**: Reduce code by ~200 lines, eliminate float/double duplication

### Phase 5: Naming Standardization and Code Quality (Medium Risk)

#### Task 5.1: Fix Directory Names
- [ ] Rename directories with spaces (e.g., `generate domain/` → `generate_domain/`)
- [ ] Standardize on snake_case for directories
- [ ] Update all references in scripts and code
- [ ] Update CMakeLists.txt paths

#### Task 5.2: Standardize File Naming
- [ ] Rename `*-context.*` files to `*_context.*` (snake_case)
- [ ] Rename `interfaces-ml.*` to `interfaces_ml.*`
- [ ] Use consistent naming: `snake_case` for C++ files
- [ ] Update all includes and references
- [ ] Update CMakeLists.txt file lists

#### Task 5.3: Standardize Variable/Function Naming
- [ ] Document naming conventions in `docs/coding_standards.md`:
  - Classes: `PascalCase` (e.g., `BLASContext`)
  - Functions: `snake_case` (e.g., `fill_matrix_parallel`)
  - Variables: `snake_case` (e.g., `num_threads`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_THREADS`)
  - Namespaces: `snake_case` (e.g., `blas_config`)
- [ ] Refactor inconsistent names gradually:
  - `test_class` → `TestClass`
  - `test_class_ml` → `TestClassML`
  - `library_manager` → `LibraryManager`
- [ ] Update all references

#### Task 5.4: Remove Commented Code
- [ ] Remove commented-out code blocks from `interfaces-ml.cpp`
- [ ] Remove commented-out code from `BLIS-context.cpp` (lines 19-38)
- [ ] Remove commented-out code from `MKL-context.cpp`
- [ ] Remove commented-out code from CMakeLists.txt files
- [ ] Clean up TODO comments or convert to issues

#### Task 5.5: Improve Code Documentation
- [ ] Add function documentation comments (Doxygen style)
- [ ] Document template parameters
- [ ] Add class-level documentation
- [ ] Document complex algorithms

### Phase 6: Documentation and Final Cleanup (Low Risk)

#### Task 6.1: Create Documentation Structure
- [ ] Create `docs/` directory
- [ ] Create `docs/architecture.md`:
  - System overview
  - Component descriptions
  - Data flow diagrams
  - Module interactions
- [ ] Create `docs/installation.md`:
  - Prerequisites
  - Build instructions
  - Configuration setup
  - Troubleshooting
- [ ] Create `docs/api/` directory:
  - API reference for public interfaces
  - Class documentation
  - Function documentation
- [ ] Create `docs/coding_standards.md`:
  - Naming conventions
  - Code style guidelines
  - Best practices
  - Examples

#### Task 6.2: Update Main Documentation
- [ ] Update root `README.md`:
  - Reflect new directory structure
  - Update build instructions
  - Update usage examples
  - Add links to detailed docs
- [ ] Update `src/data_collect/readme.md`:
  - Update paths to reflect new structure
  - Update script references
- [ ] Update `src/library_frame/README.md`:
  - Update paths
  - Reflect new structure
- [ ] Update `src/model_training/README.md`:
  - Update data paths
  - Update model/config paths

#### Task 6.3: Add Code Documentation
- [ ] Add Doxygen comments to all public headers:
  - `src/data_collect/interfaces.h`
  - `src/data_collect/library-manager.h`
  - `src/library_frame/BLAS/include/*.h`
  - `src/library_frame/predictor/predictor.hpp`
- [ ] Add inline documentation for complex functions
- [ ] Document template specializations
- [ ] Add usage examples in documentation

#### Task 6.4: Create Developer Guide
- [ ] Create `docs/developer_guide.md`:
  - How to add new BLAS routines
  - How to add new BLAS library support
  - How to extend the predictor
  - Testing guidelines
  - Contribution guidelines

#### Task 6.5: Final Cleanup
- [ ] Remove all commented-out code blocks
- [ ] Update outdated comments
- [ ] Fix inconsistent indentation
- [ ] Ensure consistent line endings
- [ ] Add missing error handling documentation

## Code Duplication Statistics

### Current State
- **Context classes**: ~500 lines of duplicated declarations/implementations
- **Matrix filling**: ✅ ~2000 lines eliminated (was duplicated 43+ times)
- **Timing code**: ~500 lines of duplicated timing logic (24+ instances)
- **Predictor code**: ~300 lines of duplicated initialization (18+ instances)
- **Level 1/2 routines**: ✅ ~200 lines using unsafe malloc/free → now safe std::vector
- **Total duplication**: ~3500+ lines that can be eliminated

### Expected After Refactoring
- **Context classes**: ~100 lines of shared code + ~200 lines of implementations
- **Matrix filling**: ✅ ~150 lines of helper functions replacing 2000 lines
- **Timing code**: ~50 lines of helper functions replacing 500 lines
- **Predictor code**: ~50 lines of helper class replacing 300 lines
- **Level 1/2 routines**: ✅ ~200 lines using safe std::vector
- **Total reduction**: ~2500+ lines eliminated, ~500 lines of reusable code added
- **Net reduction**: ~2000 lines (~30% reduction in interfaces-ml.cpp)

### Progress So Far
- ✅ **Phase 1**: Cleanup and Removal - COMPLETE
- ✅ **Phase 2**: Directory Reorganization - COMPLETE
- ✅ **Phase 3**: Source Code Reorganization - COMPLETE (except headers/sources separation)
- 🔄 **Phase 4**: Code Duplication Elimination - IN PROGRESS
  - ✅ Task 4.2: Extract matrix filling helpers - COMPLETE
  - ✅ Task 4.5: Replace malloc/free in Level 1/2 - COMPLETE
  - ⏳ Task 4.3: Extract timing helpers - PENDING
  - ⏳ Task 4.4: Extract predictor helpers - PENDING
  - ⏳ Task 4.6: Template Level 1/2 routines - PENDING
  - ⏳ Task 4.1: Refactor context classes - PENDING
- ⏳ **Phase 5**: Naming Standardization - PENDING
- ⏳ **Phase 6**: Documentation and Cleanup - PENDING

## Implementation Strategy

### Priority Order
1. **Phase 1** (Cleanup) - ✅ COMPLETE
2. **Phase 2** (Directory reorganization) - ✅ COMPLETE
3. **Phase 3** (Source reorganization) - ✅ COMPLETE
4. **Phase 4** (Code duplication elimination) - 🔄 IN PROGRESS
   - ✅ 4.5: Replace malloc/free (safety first) - COMPLETE
   - ✅ 4.2: Extract matrix filling helpers (biggest impact) - COMPLETE
   - ⏳ 4.3: Extract timing helpers (quick win) - NEXT
   - ⏳ 4.4: Extract predictor helpers (centralize logic) - PENDING
   - ⏳ 4.6: Template Level 1/2 routines (consistency) - PENDING
   - ⏳ 4.1: Refactor context classes (architectural improvement) - PENDING
5. **Phase 5** (Naming and code quality) - PENDING
6. **Phase 6** (Documentation) - PENDING

### Risk Mitigation
- Create backup branch before major changes
- Test after each phase
- Update `.gitignore` first
- Use git mv for renames to preserve history
- Update paths incrementally
- Test build system after each change

## Success Criteria
- [x] No build artifacts in source directories
- [x] Clear separation: src/tests/data/scripts/docs
- [ ] Consistent naming conventions
- [ ] All tests pass
- [ ] Build system works correctly
- [ ] Documentation updated
- [ ] No broken references
- [x] Code duplication significantly reduced (~2000 lines eliminated so far)

## Estimated Effort
- Phase 1: 2-4 hours ✅ COMPLETE
- Phase 2: 4-8 hours ✅ COMPLETE
- Phase 3: 8-16 hours ✅ COMPLETE
- Phase 4: 16-24 hours (Code duplication elimination - HIGH VALUE)
  - ✅ 4.2: Extract matrix filling helpers - COMPLETE (~4 hours)
  - ✅ 4.5: Replace malloc/free - COMPLETE (~2 hours)
  - ⏳ 4.3: Extract timing helpers - ~2 hours
  - ⏳ 4.4: Extract predictor helpers - ~2 hours
  - ⏳ 4.6: Template Level 1/2 routines - ~4 hours
  - ⏳ 4.1: Refactor context classes - ~6 hours
- Phase 5: 8-12 hours (Naming and code quality)
- Phase 6: 8-12 hours (Documentation)
- **Total Remaining: 24-36 hours**

## Notes

### Completed Work
1. **Phase 1-3**: Successfully reorganized directory structure, moved source code into `src/`, consolidated experimental data and scripts
2. **Matrix Filling Helpers**: Created `fill_matrix_parallel()` and `fill_symmetric_matrix_parallel()` helper functions, eliminating ~1850 lines of duplicated pthread code
3. **Memory Safety**: Replaced all `malloc`/`free`/`memalign` calls in Level 1/2 routines with `std::vector`, eliminating memory leak risks
4. **Exception Handling**: Added comprehensive exception handling to Level 3 handler functions in `main.cpp`

### Next Steps
1. Continue Phase 4: Extract timing helpers and predictor helpers
2. Template Level 1/2 routines to eliminate float/double duplication
3. Refactor context classes to eliminate duplication
4. Standardize naming conventions
5. Create comprehensive documentation

### Known Issues
- Some commented-out code remains in various files
- Duplicate `run_compare_gemv_` function definition (needs investigation)
- Inconsistent naming conventions across codebase
- Missing Doxygen documentation for public APIs
