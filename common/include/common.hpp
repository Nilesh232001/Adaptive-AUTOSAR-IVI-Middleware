#ifndef COMMON_HPP
#define COMMON_HPP

#include <string>
#include <vector>

// Common utility functions and types used across the project

namespace common {

    // Function to trim whitespace from both ends of a string
    std::string trim(const std::string& str);

    // Function to split a string by a delimiter
    std::vector<std::string> split(const std::string& str, char delimiter);

    // Function to convert a string to lowercase
    std::string toLower(const std::string& str);

    // Function to check if a string starts with a given prefix
    bool startsWith(const std::string& str, const std::string& prefix);

    // Function to check if a string ends with a given suffix
    bool endsWith(const std::string& str, const std::string& suffix);

} // namespace common

#endif // COMMON_HPP