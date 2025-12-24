#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

class Logger {
public:
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& filename);
    void log(LogLevel level, const std::string& message);
    
private:
    std::ofstream logFile;
    std::string getCurrentTime();
    std::string logLevelToString(LogLevel level);
};

#endif // LOGGING_HPP