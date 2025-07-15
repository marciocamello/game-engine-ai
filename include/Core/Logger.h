#pragma once

#include <string>
#include <fstream>
#include <memory>

namespace GameEngine {
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    class Logger {
    public:
        static Logger& GetInstance();
        
        void Initialize(const std::string& filename = "engine.log");
        void Log(LogLevel level, const std::string& message);
        void SetLogLevel(LogLevel level) { m_minLogLevel = level; }

        // Convenience methods
        void Debug(const std::string& message) { Log(LogLevel::Debug, message); }
        void Info(const std::string& message) { Log(LogLevel::Info, message); }
        void Warning(const std::string& message) { Log(LogLevel::Warning, message); }
        void Error(const std::string& message) { Log(LogLevel::Error, message); }
        void Critical(const std::string& message) { Log(LogLevel::Critical, message); }

    private:
        Logger() = default;
        std::string GetLogLevelString(LogLevel level);
        std::string GetTimestamp();

        std::unique_ptr<std::ofstream> m_logFile;
        LogLevel m_minLogLevel = LogLevel::Info;
    };

    // Convenience macros
    #define LOG_DEBUG(msg) GameEngine::Logger::GetInstance().Debug(msg)
    #define LOG_INFO(msg) GameEngine::Logger::GetInstance().Info(msg)
    #define LOG_WARNING(msg) GameEngine::Logger::GetInstance().Warning(msg)
    #define LOG_ERROR(msg) GameEngine::Logger::GetInstance().Error(msg)
    #define LOG_CRITICAL(msg) GameEngine::Logger::GetInstance().Critical(msg)
}