#include "../interfaces/TestFramework.h"
#include <iostream>
#include <string>

using namespace GameEngine::Testing;

/**
 * Test configuration management utility
 */
class TestConfigManager {
private:
    ITestFramework& m_framework;
    
public:
    TestConfigManager() : m_framework(GetTestFramework()) {}
    
    /**
     * Show current configuration
     */
    void ShowConfig() {
        std::string configPath = "projects/Tests/config/test_config.json";
        if (!m_framework.LoadConfig(configPath)) {
            std::cout << "[WARNING] Could not load test config, showing defaults" << std::endl;
        }
        
        const auto& config = m_framework.GetConfig();
        
        std::cout << "========================================" << std::endl;
        std::cout << " Test Framework Configuration" << std::endl;
        std::cout << "========================================" << std::endl;
        
        std::cout << std::endl << "Test Categories:" << std::endl;
        for (const auto& [category, enabled] : config.enabledCategories) {
            std::cout << "  " << category << ": " << (enabled ? "enabled" : "disabled") << std::endl;
        }
        
        std::cout << std::endl << "Test Execution Settings:" << std::endl;
        std::cout << "  Unit Tests: " << (config.enableUnitTests ? "enabled" : "disabled") << std::endl;
        std::cout << "  Integration Tests: " << (config.enableIntegrationTests ? "enabled" : "disabled") << std::endl;
        std::cout << "  Performance Tests: " << (config.enablePerformanceTests ? "enabled" : "disabled") << std::endl;
        
        std::cout << std::endl << "Output Settings:" << std::endl;
        std::cout << "  Verbose Output: " << (config.verboseOutput ? "enabled" : "disabled") << std::endl;
        std::cout << "  Show Timings: " << (config.showTimings ? "enabled" : "disabled") << std::endl;
        std::cout << "  Output Format: " << config.outputFormat << std::endl;
        
        std::cout << std::endl << "Performance Settings:" << std::endl;
        std::cout << "  Timeout: " << config.performanceTimeoutMs << "ms" << std::endl;
        std::cout << "  Iterations: " << config.performanceIterations << std::endl;
        
        std::cout << std::endl << "Test Directories:" << std::endl;
        for (const auto& dir : config.testDirectories) {
            std::cout << "  " << dir << std::endl;
        }
        
        if (!config.excludePatterns.empty()) {
            std::cout << std::endl << "Exclude Patterns:" << std::endl;
            for (const auto& pattern : config.excludePatterns) {
                std::cout << "  " << pattern << std::endl;
            }
        }
        
        std::cout << "========================================" << std::endl;
    }
    
    /**
     * Enable/disable a test category
     */
    void SetCategoryEnabled(const std::string& category, bool enabled) {
        std::string configPath = "projects/Tests/config/test_config.json";
        if (!m_framework.LoadConfig(configPath)) {
            std::cout << "[WARNING] Could not load test config, using defaults" << std::endl;
        }
        
        TestConfig config = m_framework.GetConfig();
        config.enabledCategories[category] = enabled;
        
        // Also update individual flags
        if (category == "unit") {
            config.enableUnitTests = enabled;
        } else if (category == "integration") {
            config.enableIntegrationTests = enabled;
        } else if (category == "performance") {
            config.enablePerformanceTests = enabled;
        }
        
        m_framework.SetConfig(config);
        
        if (m_framework.SaveConfig(configPath)) {
            std::cout << "[SUCCESS] " << category << " tests " 
                     << (enabled ? "enabled" : "disabled") << std::endl;
        } else {
            std::cout << "[ERROR] Failed to save configuration" << std::endl;
        }
    }
    
    /**
     * Set output format
     */
    void SetOutputFormat(const std::string& format) {
        if (format != "standard" && format != "json" && format != "xml") {
            std::cout << "[ERROR] Invalid output format. Use: standard, json, xml" << std::endl;
            return;
        }
        
        std::string configPath = "projects/Tests/config/test_config.json";
        if (!m_framework.LoadConfig(configPath)) {
            std::cout << "[WARNING] Could not load test config, using defaults" << std::endl;
        }
        
        TestConfig config = m_framework.GetConfig();
        config.outputFormat = format;
        m_framework.SetConfig(config);
        
        if (m_framework.SaveConfig(configPath)) {
            std::cout << "[SUCCESS] Output format set to: " << format << std::endl;
        } else {
            std::cout << "[ERROR] Failed to save configuration" << std::endl;
        }
    }
    
    /**
     * Set verbose output
     */
    void SetVerboseOutput(bool verbose) {
        std::string configPath = "projects/Tests/config/test_config.json";
        if (!m_framework.LoadConfig(configPath)) {
            std::cout << "[WARNING] Could not load test config, using defaults" << std::endl;
        }
        
        TestConfig config = m_framework.GetConfig();
        config.verboseOutput = verbose;
        m_framework.SetConfig(config);
        
        if (m_framework.SaveConfig(configPath)) {
            std::cout << "[SUCCESS] Verbose output " 
                     << (verbose ? "enabled" : "disabled") << std::endl;
        } else {
            std::cout << "[ERROR] Failed to save configuration" << std::endl;
        }
    }
    
    /**
     * Reset configuration to defaults
     */
    void ResetConfig() {
        TestConfig defaultConfig;
        m_framework.SetConfig(defaultConfig);
        
        std::string configPath = "projects/Tests/config/test_config.json";
        if (m_framework.SaveConfig(configPath)) {
            std::cout << "[SUCCESS] Configuration reset to defaults" << std::endl;
        } else {
            std::cout << "[ERROR] Failed to save configuration" << std::endl;
        }
    }
    
    /**
     * Show help information
     */
    void ShowHelp() {
        std::cout << "Test Configuration Manager - Usage:" << std::endl;
        std::cout << "  --show                   Show current configuration" << std::endl;
        std::cout << "  --enable <category>      Enable test category (unit/integration/performance)" << std::endl;
        std::cout << "  --disable <category>     Disable test category (unit/integration/performance)" << std::endl;
        std::cout << "  --format <format>        Set output format (standard/json/xml)" << std::endl;
        std::cout << "  --verbose <on|off>       Enable/disable verbose output" << std::endl;
        std::cout << "  --reset                  Reset configuration to defaults" << std::endl;
        std::cout << "  --help                   Show this help message" << std::endl;
        std::cout << std::endl;
        std::cout << "Configuration file: projects/Tests/config/test_config.json" << std::endl;
    }
};

/**
 * Main entry point for test configuration manager
 */
int main(int argc, char* argv[]) {
    TestConfigManager manager;
    
    if (argc < 2) {
        manager.ShowConfig();
        return 0;
    }
    
    std::string command = argv[1];
    
    if (command == "--show") {
        manager.ShowConfig();
    } else if (command == "--enable" && argc >= 3) {
        manager.SetCategoryEnabled(argv[2], true);
    } else if (command == "--disable" && argc >= 3) {
        manager.SetCategoryEnabled(argv[2], false);
    } else if (command == "--format" && argc >= 3) {
        manager.SetOutputFormat(argv[2]);
    } else if (command == "--verbose" && argc >= 3) {
        std::string value = argv[2];
        bool verbose = (value == "on" || value == "true" || value == "1");
        manager.SetVerboseOutput(verbose);
    } else if (command == "--reset") {
        manager.ResetConfig();
    } else if (command == "--help") {
        manager.ShowHelp();
    } else {
        std::cerr << "[ERROR] Unknown command or missing arguments" << std::endl;
        manager.ShowHelp();
        return 1;
    }
    
    return 0;
}