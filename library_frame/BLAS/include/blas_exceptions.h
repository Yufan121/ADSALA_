#ifndef __BLAS_EXCEPTIONS_H_INCLUDED__
#define __BLAS_EXCEPTIONS_H_INCLUDED__

#include <stdexcept>
#include <string>

namespace blas_exceptions {

/**
 * @brief Base exception class for BLAS-related errors
 */
class BLASException : public std::runtime_error {
public:
    explicit BLASException(const std::string& message) 
        : std::runtime_error("BLAS Error: " + message) {}
};

/**
 * @brief Exception for thread creation failures
 */
class ThreadCreationException : public BLASException {
public:
    explicit ThreadCreationException(int error_code)
        : BLASException("Failed to create thread (error code: " + std::to_string(error_code) + ")") {}
};

/**
 * @brief Exception for memory allocation failures
 */
class MemoryAllocationException : public BLASException {
public:
    explicit MemoryAllocationException(const std::string& details)
        : BLASException("Memory allocation failed: " + details) {}
};

/**
 * @brief Exception for invalid input parameters
 */
class InvalidArgumentException : public BLASException {
public:
    explicit InvalidArgumentException(const std::string& param_name, const std::string& reason = "")
        : BLASException("Invalid argument '" + param_name + "'" + (reason.empty() ? "" : ": " + reason)) {}
};

/**
 * @brief Exception for CSV file operations
 */
class CSVException : public BLASException {
public:
    explicit CSVException(const std::string& operation, const std::string& details)
        : BLASException("CSV " + operation + " failed: " + details) {}
};

/**
 * @brief Exception for configuration errors
 */
class ConfigurationException : public BLASException {
public:
    explicit ConfigurationException(const std::string& details)
        : BLASException("Configuration error: " + details) {}
};

} // namespace blas_exceptions

#endif // __BLAS_EXCEPTIONS_H_INCLUDED__


