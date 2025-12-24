#include "logging.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace Logging {

void logInfo(const std::string& message) {
    std::ofstream logFile("log.txt", std::ios_base::app);
    if (logFile.is_open()) {
        logFile << "[" << currentDateTime() << "] INFO: " << message << std::endl;
    }
}

void logError(const std::string& message) {
    std::ofstream logFile("log.txt", std::ios_base::app);
    if (logFile.is_open()) {
        logFile << "[" << currentDateTime() << "] ERROR: " << message << std::endl;
    }
}

std::string currentDateTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm nowTm = *std::localtime(&nowTime);
    std::ostringstream oss;
    oss << std::put_time(&nowTm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace Logging