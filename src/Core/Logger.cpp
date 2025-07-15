#include "Core/Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>

namespace GameEngine {
    Logger& Logger::GetInstance() {
        static Logger instance;
        return instance;
    }

    void Logger::Initialize(const std::string& filename) {
        m_logFile = std::make_unique<std::ofstream>(filename, std::ios::app);
        if (!m_logFile->is_open()) {
            std::cerr << "Warning: Could not open log file: " << filename << std::endl;
        }
    }

    void Logger::Log(LogLevel level, const std::string& message) {
        if (level < m_minLogLevel) {
            return;
        }

        std::string timestamp = GetTimestamp();
        std::string levelStr = GetLogLevelString(level);
        std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;

        // Output to console
        std::cout << logMessage << std::endl;

        // Output to file if available
        if (m_logFile && m_logFile->is_open()) {
            *m_logFile << logMessage << std::endl;
            m_logFile->flush();
        }
    }

    std::string Logger::GetLogLevelString(LogLevel level) {
        switch (level) {
            case LogLevel::Debug:    return "DEBUG";
            case LogLevel::Info:     return "INFO";
            case LogLevel::Warning:  return "WARNING";
            case LogLevel::Error:    return "ERROR";
            case LogLevel::Critical: return "CRITICAL";
            default:                 return "UNKNOWN";
        }
    }

    std::string Logger::GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
}