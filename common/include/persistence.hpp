#ifndef PERSISTENCE_HPP
#define PERSISTENCE_HPP

#include <string>
#include <nlohmann/json.hpp>

class Persistence {
public:
    // Load configuration from a JSON file
    static nlohmann::json loadConfig(const std::string& filename);

    // Save configuration to a JSON file
    static void saveConfig(const std::string& filename, const nlohmann::json& config);

    // Load the last known state from a JSON file
    static nlohmann::json loadState(const std::string& filename);

    // Save the current state to a JSON file
    static void saveState(const std::string& filename, const nlohmann::json& state);
};

// Logging function for persistence module
void log_error(const std::string& message);

#endif // PERSISTENCE_HPP