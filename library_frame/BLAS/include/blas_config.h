#ifndef __BLAS_CONFIG_H_INCLUDED__
#define __BLAS_CONFIG_H_INCLUDED__

#include <string>
#include <memory>

namespace blas_config {

/**
 * @brief Configuration class for BLAS benchmarking system
 * 
 * Loads configuration from YAML files and provides access to configuration values.
 * Supports command-line overrides and default values.
 */
class Config {
public:
    /**
     * @brief Get the singleton instance of Config
     * @return Reference to the Config instance
     */
    static Config& getInstance();

    /**
     * @brief Load configuration from a YAML file
     * @param config_path Path to the YAML configuration file
     * @throws ConfigurationException if the file cannot be loaded or parsed
     */
    void loadFromFile(const std::string& config_path);

    /**
     * @brief Load configuration from a YAML string (for testing)
     * @param yaml_content YAML content as a string
     * @throws ConfigurationException if the YAML cannot be parsed
     */
    void loadFromString(const std::string& yaml_content);

    /**
     * @brief Get the number of threads for pthread operations
     * @return Number of threads (default: 24)
     */
    int getNumThreads() const { return num_threads_; }

    /**
     * @brief Set the number of threads (command-line override)
     * @param num_threads Number of threads
     */
    void setNumThreads(int num_threads) { num_threads_ = num_threads; }

    /**
     * @brief Get the number of duplicate runs for benchmarking
     * @return Number of duplicates (default: 10)
     */
    int getNumOfDuplicate() const { return num_of_duplicate_; }

    /**
     * @brief Set the number of duplicate runs (command-line override)
     * @param num_of_duplicate Number of duplicates
     */
    void setNumOfDuplicate(int num_of_duplicate) { num_of_duplicate_ = num_of_duplicate; }

    /**
     * @brief Get the default configuration file path
     * @return Path to default config file
     */
    std::string getDefaultConfigPath() const { return default_config_path_; }

    /**
     * @brief Set the default configuration file path
     * @param path Path to default config file
     */
    void setDefaultConfigPath(const std::string& path) { default_config_path_ = path; }

    /**
     * @brief Reset configuration to defaults
     */
    void reset();

private:
    Config();
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    void loadDefaults();
    void parseYAML(const std::string& yaml_content);

    int num_threads_;
    int num_of_duplicate_;
    std::string default_config_path_;
};

} // namespace blas_config

#endif // __BLAS_CONFIG_H_INCLUDED__

