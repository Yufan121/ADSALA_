#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <omp.h>
#include "predictor.hpp"
#include "blas_config.h"

// Forward declaration of helper function
int setup_threads_with_predictor(Predictor& predictor, bool useML, int predicted_cores);

// Mock predictor class for testing
class MockPredictor : public Predictor {
public:
    MockPredictor() : Predictor("xgb", "test") {}

    // Override to return a predictable value
    int predict_num_cores(int m, int n, std::string routine) {
        return 8; // Return a fixed value for testing
    }

    int predict_num_cores(int m, int k, int n, std::string routine) {
        return 4; // Return a different fixed value for testing
    }

    int predict_num_cores(int n, std::string routine) {
        return 2; // Return another fixed value for testing
    }
};

namespace {

// Test fixture for predictor helper tests
class PredictorHelperTest : public ::testing::Test {
protected:
    MockPredictor mockPredictor;
    int original_num_threads;

    void SetUp() override {
        // Store original OpenMP thread count
        original_num_threads = omp_get_max_threads();

        // Reset config to known state
        blas_config::Config::getInstance().reset();
        blas_config::Config::getInstance().setNumThreads(16); // Set a known max threads value
    }

    void TearDown() override {
        // Restore original OpenMP thread count
        omp_set_num_threads(original_num_threads);
    }
};

// Test setup_threads_with_predictor with useML = true
TEST_F(PredictorHelperTest, SetupThreadsWithPredictor_UseML_True) {
    // Capture cout output
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    int result = setup_threads_with_predictor(mockPredictor, true, 8);

    std::cout.rdbuf(old); // Restore original buffer

    // Check return value
    EXPECT_EQ(result, 8);

    // Check that OpenMP threads were set correctly
    EXPECT_EQ(omp_get_max_threads(), 8);

    // Check output
    std::string output = buffer.str();
    EXPECT_NE(output.find("Number of threads: 8"), std::string::npos);
}

// Test setup_threads_with_predictor with useML = false
TEST_F(PredictorHelperTest, SetupThreadsWithPredictor_UseML_False) {
    // Set config max threads to a known value
    blas_config::Config::getInstance().setPredictorMaxThreads(12);

    // Capture cout output
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    int result = setup_threads_with_predictor(mockPredictor, false, 8); // predicted_cores ignored when useML=false

    std::cout.rdbuf(old); // Restore original buffer

    // Check return value (should be from config)
    EXPECT_EQ(result, 12);

    // Check that OpenMP threads were set correctly
    EXPECT_EQ(omp_get_max_threads(), 12);

    // Check output
    std::string output = buffer.str();
    EXPECT_NE(output.find("Number of threads: 12"), std::string::npos);
}

// Test setup_threads_with_predictor with different predicted cores values
TEST_F(PredictorHelperTest, SetupThreadsWithPredictor_DifferentValues) {
    std::vector<int> test_values = {1, 4, 8, 16, 32};

    for (int expected_threads : test_values) {
        std::stringstream buffer;
        std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

        int result = setup_threads_with_predictor(mockPredictor, true, expected_threads);

        std::cout.rdbuf(old);

        EXPECT_EQ(result, expected_threads);
        EXPECT_EQ(omp_get_max_threads(), expected_threads);

        std::string output = buffer.str();
        std::string expected_msg = "Number of threads: " + std::to_string(expected_threads);
        EXPECT_NE(output.find(expected_msg), std::string::npos);
    }
}

// Test setup_threads_with_predictor output format
TEST_F(PredictorHelperTest, SetupThreadsWithPredictor_OutputFormat) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    setup_threads_with_predictor(mockPredictor, true, 6);

    std::cout.rdbuf(old);

    std::string output = buffer.str();

    // Check that output contains expected format
    EXPECT_NE(output.find("Number of threads:"), std::string::npos);
    EXPECT_NE(output.find("6"), std::string::npos);

    // Check that it's a single line or properly formatted
    size_t pos = output.find("Number of threads:");
    EXPECT_NE(pos, std::string::npos);
}

// Test that the function doesn't crash with edge cases
TEST_F(PredictorHelperTest, SetupThreadsWithPredictor_EdgeCases) {
    // Test with 0 threads (though this might not be realistic)
    EXPECT_NO_THROW({
        setup_threads_with_predictor(mockPredictor, true, 0);
    });

    // Test with very high thread count
    EXPECT_NO_THROW({
        setup_threads_with_predictor(mockPredictor, true, 1024);
    });

    // Verify OpenMP handles these values appropriately
    // (OpenMP might clamp or handle these values)
}

// Test that config changes are respected when useML = false
TEST_F(PredictorHelperTest, SetupThreadsWithPredictor_ConfigChanges) {
    // Change config multiple times and verify it's respected
    blas_config::Config::getInstance().setPredictorMaxThreads(4);

    int result1 = setup_threads_with_predictor(mockPredictor, false, 999); // ignored
    EXPECT_EQ(result1, 4);
    EXPECT_EQ(omp_get_max_threads(), 4);

    blas_config::Config::getInstance().setPredictorMaxThreads(8);

    int result2 = setup_threads_with_predictor(mockPredictor, false, 999); // ignored
    EXPECT_EQ(result2, 8);
    EXPECT_EQ(omp_get_max_threads(), 8);
}

} // namespace
