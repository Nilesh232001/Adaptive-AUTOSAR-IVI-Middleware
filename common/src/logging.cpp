#include "logging.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios_base::app);
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm nowTm = *std::localtime(&nowTime);
    std::ostringstream oss;
    oss << std::put_time(&nowTm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (logFile.is_open()) {
        logFile << "[" << getCurrentTime() << "] " 
                << logLevelToString(level) << ": " 
                << message << std::endl;
        logFile.flush();
    }
}

// Global logging functions for convenience
static Logger* g_logger = nullptr;

void log_info(const std::string& message) {
    if (!g_logger) {
        g_logger = new Logger("app.log");
    }
    g_logger->log(Logger::LogLevel::INFO, message);
}

void log_warning(const std::string& message) {
    if (!g_logger) {
        g_logger = new Logger("app.log");
    }
    g_logger->log(Logger::LogLevel::WARNING, message);
}

void log_error(const std::string& message) {
    if (!g_logger) {
        g_logger = new Logger("app.log");
    }
    g_logger->log(Logger::LogLevel::ERROR, message);
}