#include <gtest/gtest.h>
#include "blas_config.h"
#include "blas_exceptions.h"
#include <fstream>
#include <cstdio>

namespace {

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset config to defaults before each test
        blas_config::Config::getInstance().reset();
    }
    
    void TearDown() override {
        // Clean up any test config files
        std::remove("test_config.yaml");
    }
    
    void createTestConfigFile(const std::string& content) {
        std::ofstream file("test_config.yaml");
        file << content;
        file.close();
    }
};

// Test default values
TEST_F(ConfigTest, DefaultValues) {
    auto& config = blas_config::Config::getInstance();
    EXPECT_EQ(config.getNumThreads(), 24);
    EXPECT_EQ(config.getNumOfDuplicate(), 10);
}

// Test loading from valid YAML file
TEST_F(ConfigTest, LoadValidYAML) {
#ifdef HAVE_YAML_CPP
    createTestConfigFile(
        "num_threads: 48\n"
        "num_of_duplicate: 20\n"
    );
    
    auto& config = blas_config::Config::getInstance();
    EXPECT_NO_THROW(config.loadFromFile("test_config.yaml"));
    EXPECT_EQ(config.getNumThreads(), 48);
    EXPECT_EQ(config.getNumOfDuplicate(), 20);
#else
    GTEST_SKIP() << "yaml-cpp not available, skipping YAML parsing tests";
#endif
}

// Test loading from invalid file path
TEST_F(ConfigTest, LoadInvalidPath) {
    auto& config = blas_config::Config::getInstance();
    EXPECT_THROW(
        config.loadFromFile("nonexistent_config.yaml"),
        blas_exceptions::ConfigurationException
    );
}

// Test loading from invalid YAML content
TEST_F(ConfigTest, LoadInvalidYAML) {
#ifdef HAVE_YAML_CPP
    createTestConfigFile(
        "num_threads: invalid_value\n"
        "num_of_duplicate: 20\n"
    );
    
    auto& config = blas_config::Config::getInstance();
    EXPECT_THROW(
        config.loadFromFile("test_config.yaml"),
        blas_exceptions::ConfigurationException
    );
#else
    GTEST_SKIP() << "yaml-cpp not available";
#endif
}

// Test validation: num_threads out of range
TEST_F(ConfigTest, NumThreadsOutOfRange) {
#ifdef HAVE_YAML_CPP
    createTestConfigFile("num_threads: 2000\n");
    
    auto& config = blas_config::Config::getInstance();
    EXPECT_THROW(
        config.loadFromFile("test_config.yaml"),
        blas_exceptions::ConfigurationException
    );
#else
    GTEST_SKIP() << "yaml-cpp not available";
#endif
}

// Test validation: num_of_duplicate out of range
TEST_F(ConfigTest, NumOfDuplicateOutOfRange) {
#ifdef HAVE_YAML_CPP
    createTestConfigFile("num_of_duplicate: 2000\n");
    
    auto& config = blas_config::Config::getInstance();
    EXPECT_THROW(
        config.loadFromFile("test_config.yaml"),
        blas_exceptions::ConfigurationException
    );
#else
    GTEST_SKIP() << "yaml-cpp not available";
#endif
}

// Test command-line override
TEST_F(ConfigTest, CommandLineOverride) {
    auto& config = blas_config::Config::getInstance();
    
    config.setNumThreads(64);
    EXPECT_EQ(config.getNumThreads(), 64);
    EXPECT_EQ(config.getNumOfDuplicate(), 10); // Should remain default
    
    config.setNumOfDuplicate(15);
    EXPECT_EQ(config.getNumThreads(), 64);
    EXPECT_EQ(config.getNumOfDuplicate(), 15);
}

// Test singleton pattern
TEST_F(ConfigTest, SingletonPattern) {
    auto& config1 = blas_config::Config::getInstance();
    auto& config2 = blas_config::Config::getInstance();
    
    // Should be the same instance
    EXPECT_EQ(&config1, &config2);
    
    config1.setNumThreads(32);
    EXPECT_EQ(config2.getNumThreads(), 32);
}

// Test loading from string
TEST_F(ConfigTest, LoadFromString) {
#ifdef HAVE_YAML_CPP
    auto& config = blas_config::Config::getInstance();
    
    std::string yaml_content = 
        "num_threads: 16\n"
        "num_of_duplicate: 5\n";
    
    EXPECT_NO_THROW(config.loadFromString(yaml_content));
    EXPECT_EQ(config.getNumThreads(), 16);
    EXPECT_EQ(config.getNumOfDuplicate(), 5);
#else
    GTEST_SKIP() << "yaml-cpp not available";
#endif
}

// Test reset functionality
TEST_F(ConfigTest, Reset) {
    auto& config = blas_config::Config::getInstance();
    
    config.setNumThreads(100);
    config.setNumOfDuplicate(50);
    
    config.reset();
    
    EXPECT_EQ(config.getNumThreads(), 24);
    EXPECT_EQ(config.getNumOfDuplicate(), 10);
}

} // namespace

