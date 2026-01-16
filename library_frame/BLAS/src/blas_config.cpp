#include "blas_config.h"
#include "blas_exceptions.h"
#include <fstream>
#include <sstream>

#ifdef HAVE_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif

namespace blas_config {

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

Config::Config() {
    loadDefaults();
}

void Config::loadDefaults() {
    num_threads_ = 24;
    num_of_duplicate_ = 10;
    default_config_path_ = "config.yaml";
}

void Config::reset() {
    loadDefaults();
}

void Config::loadFromFile(const std::string& config_path) {
#ifdef HAVE_YAML_CPP
    std::ifstream file(config_path);
    if (!file.is_open()) {
        throw blas_exceptions::ConfigurationException(
            "Cannot open configuration file: " + config_path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    try {
        parseYAML(buffer.str());
        default_config_path_ = config_path;
    } catch (const YAML::Exception& e) {
        throw blas_exceptions::ConfigurationException(
            "YAML parsing error in " + config_path + ": " + e.what());
    } catch (const std::exception& e) {
        throw blas_exceptions::ConfigurationException(
            "Error loading configuration from " + config_path + ": " + e.what());
    }
#else
    // If yaml-cpp is not available, just use defaults
    default_config_path_ = config_path;
#endif
}

void Config::loadFromString(const std::string& yaml_content) {
#ifdef HAVE_YAML_CPP
    try {
        parseYAML(yaml_content);
    } catch (const YAML::Exception& e) {
        throw blas_exceptions::ConfigurationException(
            "YAML parsing error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw blas_exceptions::ConfigurationException(
            "Error parsing YAML configuration: " + std::string(e.what()));
    }
#else
    // If yaml-cpp is not available, ignore the string and use defaults
    (void)yaml_content;
#endif
}

void Config::parseYAML(const std::string& yaml_content) {
#ifdef HAVE_YAML_CPP
    YAML::Node config = YAML::Load(yaml_content);

    // Parse num_threads
    if (config["num_threads"]) {
        try {
            int threads = config["num_threads"].as<int>();
            if (threads < 1 || threads > 1024) {
                throw blas_exceptions::ConfigurationException(
                    "num_threads must be between 1 and 1024");
            }
            num_threads_ = threads;
        } catch (const YAML::BadConversion& e) {
            throw blas_exceptions::ConfigurationException(
                "Invalid num_threads value: must be an integer");
        }
    }

    // Parse num_of_duplicate
    if (config["num_of_duplicate"]) {
        try {
            int duplicates = config["num_of_duplicate"].as<int>();
            if (duplicates < 1 || duplicates > 1000) {
                throw blas_exceptions::ConfigurationException(
                    "num_of_duplicate must be between 1 and 1000");
            }
            num_of_duplicate_ = duplicates;
        } catch (const YAML::BadConversion& e) {
            throw blas_exceptions::ConfigurationException(
                "Invalid num_of_duplicate value: must be an integer");
        }
    }
#else
    (void)yaml_content;
#endif
}

} // namespace blas_config

