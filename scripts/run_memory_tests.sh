#!/bin/bash

# Memory Safety Test Runner
# This script runs memory safety tests using Valgrind and AddressSanitizer

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=======================================${NC}"
echo -e "${BLUE}  Memory Safety Test Runner${NC}"
echo -e "${BLUE}=======================================${NC}"

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: Must be run from project root directory${NC}"
    exit 1
fi

# Build the project if needed
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Build directory not found. Creating and building...${NC}"
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    cd ..
else
    echo -e "${YELLOW}Rebuilding project...${NC}"
    cd build
    make -j$(nproc)
    cd ..
fi

echo -e "${GREEN}Build completed successfully${NC}"

# Function to run tests with Valgrind
run_valgrind_tests() {
    echo -e "${BLUE}Running tests with Valgrind...${NC}"

    if ! command -v valgrind &> /dev/null; then
        echo -e "${RED}Valgrind not found. Please install valgrind to run memory leak detection.${NC}"
        return 1
    fi

    cd build

    # Run memory safety tests with Valgrind
    echo -e "${YELLOW}Running MemorySafetyTest with Valgrind...${NC}"
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
             --verbose --log-file=valgrind_memory_safety.log \
             ./tests/integration/test_memory_safety

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Valgrind memory safety tests passed${NC}"
    else
        echo -e "${RED}✗ Valgrind memory safety tests failed${NC}"
        echo -e "${YELLOW}Check valgrind_memory_safety.log for details${NC}"
        return 1
    fi

    cd ..
    return 0
}

# Function to run tests with AddressSanitizer
run_asan_tests() {
    echo -e "${BLUE}Running tests with AddressSanitizer...${NC}"

    # Check if ASAN is available (need to rebuild with ASAN flags)
    if [ ! -d "build_asan" ]; then
        echo -e "${YELLOW}Building with AddressSanitizer...${NC}"
        mkdir -p build_asan
        cd build_asan
        cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
                 -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
        make -j$(nproc)
        cd ..
    fi

    cd build_asan

    echo -e "${YELLOW}Running MemorySafetyTest with AddressSanitizer...${NC}"
    ./tests/integration/test_memory_safety

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ AddressSanitizer memory safety tests passed${NC}"
    else
        echo -e "${RED}✗ AddressSanitizer memory safety tests failed${NC}"
        return 1
    fi

    cd ..
    return 0
}

# Function to run regular tests for comparison
run_regular_tests() {
    echo -e "${BLUE}Running regular tests for comparison...${NC}"

    cd build

    echo -e "${YELLOW}Running MemorySafetyTest without memory checking...${NC}"
    ./tests/integration/test_memory_safety

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Regular memory safety tests passed${NC}"
    else
        echo -e "${RED}✗ Regular memory safety tests failed${NC}"
        return 1
    fi

    cd ..
    return 0
}

# Main execution
echo -e "${BLUE}Available test modes:${NC}"
echo "1. Regular tests (no memory checking)"
echo "2. Valgrind memory leak detection"
echo "3. AddressSanitizer memory safety"
echo "4. All tests"

if [ $# -eq 0 ]; then
    echo -e "${YELLOW}No mode specified. Running regular tests...${NC}"
    MODE="regular"
else
    MODE="$1"
fi

case $MODE in
    "regular"|"1")
        run_regular_tests
        ;;
    "valgrind"|"2")
        run_valgrind_tests
        ;;
    "asan"|"address"|"3")
        run_asan_tests
        ;;
    "all"|"4")
        echo -e "${BLUE}Running all memory safety tests...${NC}"
        run_regular_tests
        echo ""
        run_valgrind_tests
        echo ""
        run_asan_tests
        ;;
    *)
        echo -e "${RED}Invalid mode: $MODE${NC}"
        echo "Usage: $0 [regular|valgrind|asan|all]"
        exit 1
        ;;
esac

echo -e "${BLUE}=======================================${NC}"
echo -e "${GREEN}Memory safety testing completed${NC}"
echo -e "${BLUE}=======================================${NC}"
