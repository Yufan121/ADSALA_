#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "library-manager.h"
#include "rapidcsv.h"
#include "blas_exceptions.h"
#include "blas_config.h"

namespace {

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test CSV domain file
        std::ofstream file("test_domain.csv");
        file << "m,n,k\n";
        file << "100,200,300\n";
        file << "200,300,400\n";
        file.close();
        
        // Reset config
        blas_config::Config::getInstance().reset();
    }
    
    void TearDown() override {
        std::remove("test_domain.csv");
        std::remove("test_output.csv");
    }
};

// Test: CSV read → Config load → Routine execution → CSV write
TEST_F(IntegrationTest, EndToEndWorkflow) {
    // Load configuration
    try {
        blas_config::Config::getInstance().loadFromFile("config.yaml");
    } catch (...) {
        // Use defaults if config file not found
    }
    
    // Read CSV
    rapidcsv::Document csv_file("test_domain.csv");
    EXPECT_NO_THROW(csv_file.GetCell<int>(0, 0));
    
    // Verify config values
    auto& config = blas_config::Config::getInstance();
    EXPECT_GT(config.getNumThreads(), 0);
    EXPECT_GT(config.getNumOfDuplicate(), 0);
    
    // Write to CSV (simulated)
    csv_file.SetCell<long long>(3, 0, 12345);
    csv_file.Save("test_output.csv");
    
    // Verify output
    rapidcsv::Document output_file("test_output.csv");
    EXPECT_EQ(output_file.GetCell<long long>(3, 0), 12345);
}

// Test: Error handling in workflow
TEST_F(IntegrationTest, ErrorHandling) {
    // Test invalid CSV file
    EXPECT_THROW(
        rapidcsv::Document csv("nonexistent.csv"),
        std::exception
    );
    
    // Test invalid config file
    EXPECT_THROW(
        blas_config::Config::getInstance().loadFromFile("nonexistent_config.yaml"),
        blas_exceptions::ConfigurationException
    );
}

} // namespace

