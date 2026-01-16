#include <gtest/gtest.h>
#include "blas_exceptions.h"

namespace {

// Test BLASException base class
TEST(ExceptionTest, BLASExceptionBasic) {
    try {
        throw blas_exceptions::BLASException("Test error message");
        FAIL() << "Expected BLASException to be thrown";
    } catch (const blas_exceptions::BLASException& e) {
        EXPECT_NE(std::string(e.what()).find("BLAS Error"), std::string::npos);
        EXPECT_NE(std::string(e.what()).find("Test error message"), std::string::npos);
    } catch (...) {
        FAIL() << "Expected BLASException, got different exception";
    }
}

// Test ThreadCreationException
TEST(ExceptionTest, ThreadCreationException) {
    try {
        throw blas_exceptions::ThreadCreationException(11);
        FAIL() << "Expected ThreadCreationException to be thrown";
    } catch (const blas_exceptions::ThreadCreationException& e) {
        EXPECT_NE(std::string(e.what()).find("thread"), std::string::npos);
        EXPECT_NE(std::string(e.what()).find("11"), std::string::npos);
    } catch (const blas_exceptions::BLASException& e) {
        // Also acceptable - ThreadCreationException inherits from BLASException
        SUCCEED();
    } catch (...) {
        FAIL() << "Expected ThreadCreationException or BLASException";
    }
}

// Test MemoryAllocationException
TEST(ExceptionTest, MemoryAllocationException) {
    try {
        throw blas_exceptions::MemoryAllocationException("Failed to allocate 1GB");
        FAIL() << "Expected MemoryAllocationException to be thrown";
    } catch (const blas_exceptions::MemoryAllocationException& e) {
        EXPECT_NE(std::string(e.what()).find("Memory allocation"), std::string::npos);
        EXPECT_NE(std::string(e.what()).find("Failed to allocate 1GB"), std::string::npos);
    } catch (...) {
        FAIL() << "Expected MemoryAllocationException";
    }
}

// Test InvalidArgumentException
TEST(ExceptionTest, InvalidArgumentException) {
    try {
        throw blas_exceptions::InvalidArgumentException("num_threads", "must be positive");
        FAIL() << "Expected InvalidArgumentException to be thrown";
    } catch (const blas_exceptions::InvalidArgumentException& e) {
        EXPECT_NE(std::string(e.what()).find("num_threads"), std::string::npos);
        EXPECT_NE(std::string(e.what()).find("must be positive"), std::string::npos);
    } catch (...) {
        FAIL() << "Expected InvalidArgumentException";
    }
}

// Test CSVException
TEST(ExceptionTest, CSVException) {
    try {
        throw blas_exceptions::CSVException("read", "file not found");
        FAIL() << "Expected CSVException to be thrown";
    } catch (const blas_exceptions::CSVException& e) {
        EXPECT_NE(std::string(e.what()).find("CSV"), std::string::npos);
        EXPECT_NE(std::string(e.what()).find("read"), std::string::npos);
        EXPECT_NE(std::string(e.what()).find("file not found"), std::string::npos);
    } catch (...) {
        FAIL() << "Expected CSVException";
    }
}

// Test ConfigurationException
TEST(ExceptionTest, ConfigurationException) {
    try {
        throw blas_exceptions::ConfigurationException("Invalid YAML syntax");
        FAIL() << "Expected ConfigurationException to be thrown";
    } catch (const blas_exceptions::ConfigurationException& e) {
        EXPECT_NE(std::string(e.what()).find("Configuration"), std::string::npos);
        EXPECT_NE(std::string(e.what()).find("Invalid YAML syntax"), std::string::npos);
    } catch (...) {
        FAIL() << "Expected ConfigurationException";
    }
}

// Test exception inheritance hierarchy
TEST(ExceptionTest, ExceptionInheritance) {
    blas_exceptions::ThreadCreationException e(5);
    
    // Should be catchable as BLASException
    try {
        throw e;
    } catch (const blas_exceptions::BLASException&) {
        SUCCEED();
    } catch (...) {
        FAIL() << "ThreadCreationException should inherit from BLASException";
    }
    
    // Should be catchable as std::runtime_error
    try {
        throw e;
    } catch (const std::runtime_error&) {
        SUCCEED();
    } catch (...) {
        FAIL() << "BLASException should inherit from std::runtime_error";
    }
}

} // namespace

