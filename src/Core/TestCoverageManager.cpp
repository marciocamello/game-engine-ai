#include "Core/TestCoverageManager.h"
#include "Core/Logger.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace GameEngine {
namespace Core {

bool TestCoverageManager::GenerateCoverageReport(const std::string& outputPath, const CoverageConfig& config) {
    Logger::GetInstance().Info("Starting coverage report generation...");
    
    // Check if coverage tool is available
    if (!IsCoverageToolAvailable()) {
        Logger::GetInstance().Error("Coverage analysis tool not available!");
        return false;
    }
    
    // Create output directory
    std::filesystem::path outputDir(outputPath);
    try {
        std::filesystem::create_directories(outputDir);
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to create output directory: " + std::string(e.what()));
        return false;
    }
    
    // Find test executables
    auto testExecutables = FindTestExecutables();
    if (testExecutables.empty()) {
        Logger::GetInstance().Error("No test executables found for coverage analysis!");
        return false;
    }
    
    Logger::GetInstance().Info("Found " + std::to_string(testExecutables.size()) + " test executables");
    
    // Generate timestamp for unique report naming
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    std::string timestamp = ss.str();
    
    std::filesystem::path reportDir = outputDir / ("coverage_" + timestamp);
    std::filesystem::create_directories(reportDir);
    
    // Build coverage command
    std::string coverageTool = GetCoverageToolPath();
    std::stringstream command;
    
#ifdef _WIN32
    // OpenCppCoverage command for Windows
    command << "\"" << coverageTool << "\" --verbose";
    
    // Add source paths
    for (const auto& sourcePath : config.sourcePaths) {
        command << " --sources \"" << sourcePath << "\"";
    }
    
    // Add excluded paths
    for (const auto& excludePath : config.excludePaths) {
        command << " --excluded_sources \"" << excludePath << "\"";
    }
    
    // Add export options
    if (config.generateHtml) {
        command << " --export_type html:\"" << reportDir.string() << "\"";
    }
    if (config.generateXml) {
        command << " --export_type cobertura:\"" << (reportDir / "coverage.xml").string() << "\"";
    }
    
    // Add modules (test executables)
    for (const auto& testExe : testExecutables) {
        command << " --modules \"" << testExe << "\"";
    }
    
    // Create a batch file to run all tests
    std::filesystem::path testRunner = reportDir / "run_tests.bat";
    std::ofstream batchFile(testRunner);
    batchFile << "@echo off\n";
    batchFile << "echo Running test suite for coverage analysis...\n";
    
    for (const auto& testExe : testExecutables) {
        std::filesystem::path exePath(testExe);
        batchFile << "echo Running " << exePath.filename().string() << "...\n";
        batchFile << "\"" << testExe << "\"\n";
        batchFile << "if %ERRORLEVEL% neq 0 echo WARNING: " << exePath.filename().string() << " failed but continuing...\n";
    }
    
    batchFile << "echo All tests completed.\n";
    batchFile.close();
    
    command << " -- \"" << testRunner.string() << "\"";
    
#else
    // For Linux/macOS, use gcov/lcov
    Logger::GetInstance().Error("Coverage analysis not yet implemented for non-Windows platforms");
    return false;
#endif
    
    // Execute coverage command
    Logger::GetInstance().Info("Executing coverage analysis command...");
    Logger::GetInstance().Debug("Command: " + command.str());
    
    bool success = ExecuteCoverageCommand(command.str());
    
    if (success) {
        Logger::GetInstance().Info("Coverage report generated successfully at: " + reportDir.string());
        
        // Generate JSON summary if requested
        if (config.generateJson) {
            std::filesystem::path xmlReport = reportDir / "coverage.xml";
            if (std::filesystem::exists(xmlReport)) {
                auto report = AnalyzeCoverage(xmlReport.string());
                ExportToJson(report, (reportDir / "coverage.json").string());
            }
        }
    } else {
        Logger::GetInstance().Error("Coverage analysis failed!");
    }
    
    return success;
}

TestCoverageManager::CoverageReport TestCoverageManager::AnalyzeCoverage(const std::string& reportPath) {
    CoverageReport report;
    
    if (!std::filesystem::exists(reportPath)) {
        Logger::GetInstance().Error("Coverage report file not found: " + reportPath);
        return report;
    }
    
    try {
        report = ParseXmlReport(reportPath);
        Logger::GetInstance().Info("Coverage analysis completed: " + 
                    std::to_string(report.linesCovered) + "% lines, " +
                    std::to_string(report.branchesCovered) + "% branches");
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to analyze coverage report: " + std::string(e.what()));
    }
    
    return report;
}

bool TestCoverageManager::MeetsCoverageThreshold(const CoverageReport& report, 
                                               float lineThreshold, 
                                               float branchThreshold) {
    bool linesMet = report.linesCovered >= lineThreshold;
    bool branchesMet = report.branchesCovered >= branchThreshold;
    
    Logger::GetInstance().Info("Coverage threshold check - Lines: " + 
                std::to_string(report.linesCovered) + "% (required: " + 
                std::to_string(lineThreshold) + "%) " + 
                (linesMet ? "PASSED" : "FAILED"));
    
    Logger::GetInstance().Info("Coverage threshold check - Branches: " + 
                std::to_string(report.branchesCovered) + "% (required: " + 
                std::to_string(branchThreshold) + "%) " + 
                (branchesMet ? "PASSED" : "FAILED"));
    
    return linesMet && branchesMet;
}

std::string TestCoverageManager::GetCoverageToolPath() {
#ifdef _WIN32
    // Try to find OpenCppCoverage in common locations
    std::vector<std::string> possiblePaths = {
        "OpenCppCoverage.exe",  // In PATH
        "C:\\Program Files\\OpenCppCoverage\\OpenCppCoverage.exe",
        "C:\\Program Files (x86)\\OpenCppCoverage\\OpenCppCoverage.exe",
        "C:\\Tools\\OpenCppCoverage\\OpenCppCoverage.exe"
    };
    
    for (const auto& path : possiblePaths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
        
        // Try to execute it to see if it's in PATH
        if (path == "OpenCppCoverage.exe") {
            STARTUPINFOA si = {};
            PROCESS_INFORMATION pi = {};
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
            
            std::string testCommand = path + " --help";
            if (CreateProcessA(nullptr, const_cast<char*>(testCommand.c_str()), 
                             nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
                WaitForSingleObject(pi.hProcess, 1000); // Wait up to 1 second
                DWORD exitCode;
                if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                    if (exitCode != STILL_ACTIVE) {
                        return path;
                    }
                }
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        }
    }
#else
    // For Linux/macOS, look for gcov/lcov
    std::vector<std::string> possiblePaths = {
        "/usr/bin/gcov",
        "/usr/local/bin/gcov",
        "gcov"
    };
    
    for (const auto& path : possiblePaths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
#endif
    
    return "";
}

bool TestCoverageManager::IsCoverageToolAvailable() {
    std::string toolPath = GetCoverageToolPath();
    bool available = !toolPath.empty();
    
    if (available) {
        Logger::GetInstance().Info("Coverage tool found: " + toolPath);
    } else {
        Logger::GetInstance().Warning("Coverage tool not found. Please install OpenCppCoverage on Windows or gcov/lcov on Linux/macOS");
    }
    
    return available;
}

bool TestCoverageManager::CreateBaseline(const CoverageReport& report, const std::string& baselinePath) {
    try {
        return ExportToJson(report, baselinePath);
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to create coverage baseline: " + std::string(e.what()));
        return false;
    }
}

TestCoverageManager::CoverageReport TestCoverageManager::CompareWithBaseline(
    const CoverageReport& currentReport, const std::string& baselinePath) {
    
    CoverageReport diff;
    
    try {
        auto baseline = LoadFromJson(baselinePath);
        
        diff.linesCovered = currentReport.linesCovered - baseline.linesCovered;
        diff.branchesCovered = currentReport.branchesCovered - baseline.branchesCovered;
        diff.coveredLines = currentReport.coveredLines - baseline.coveredLines;
        diff.totalLines = currentReport.totalLines - baseline.totalLines;
        diff.coveredBranches = currentReport.coveredBranches - baseline.coveredBranches;
        diff.totalBranches = currentReport.totalBranches - baseline.totalBranches;
        
        Logger::GetInstance().Info("Coverage comparison with baseline:");
        Logger::GetInstance().Info("  Line coverage change: " + std::to_string(diff.linesCovered) + "%");
        Logger::GetInstance().Info("  Branch coverage change: " + std::to_string(diff.branchesCovered) + "%");
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to compare with baseline: " + std::string(e.what()));
    }
    
    return diff;
}

std::string TestCoverageManager::GenerateSummary(const CoverageReport& report) {
    std::stringstream summary;
    
    summary << "Coverage Analysis Summary:\n";
    summary << "  Line Coverage: " << std::fixed << std::setprecision(1) 
            << report.linesCovered << "% (" << report.coveredLines 
            << "/" << report.totalLines << " lines)\n";
    summary << "  Branch Coverage: " << std::fixed << std::setprecision(1) 
            << report.branchesCovered << "% (" << report.coveredBranches 
            << "/" << report.totalBranches << " branches)\n";
    
    if (!report.timestamp.empty()) {
        summary << "  Generated: " << report.timestamp << "\n";
    }
    
    if (!report.reportPath.empty()) {
        summary << "  Report: " << report.reportPath << "\n";
    }
    
    if (!report.uncoveredFiles.empty()) {
        summary << "  Files with incomplete coverage: " << report.uncoveredFiles.size() << "\n";
    }
    
    return summary.str();
}

bool TestCoverageManager::ExportToJson(const CoverageReport& report, const std::string& jsonPath) {
    try {
        std::ofstream jsonFile(jsonPath);
        if (!jsonFile.is_open()) {
            Logger::GetInstance().Error("Failed to open JSON file for writing: " + jsonPath);
            return false;
        }
        
        jsonFile << "{\n";
        jsonFile << "  \"linesCovered\": " << report.linesCovered << ",\n";
        jsonFile << "  \"branchesCovered\": " << report.branchesCovered << ",\n";
        jsonFile << "  \"totalLines\": " << report.totalLines << ",\n";
        jsonFile << "  \"coveredLines\": " << report.coveredLines << ",\n";
        jsonFile << "  \"totalBranches\": " << report.totalBranches << ",\n";
        jsonFile << "  \"coveredBranches\": " << report.coveredBranches << ",\n";
        jsonFile << "  \"timestamp\": \"" << report.timestamp << "\",\n";
        jsonFile << "  \"reportPath\": \"" << report.reportPath << "\",\n";
        
        jsonFile << "  \"uncoveredFiles\": [\n";
        for (size_t i = 0; i < report.uncoveredFiles.size(); ++i) {
            jsonFile << "    \"" << report.uncoveredFiles[i] << "\"";
            if (i < report.uncoveredFiles.size() - 1) jsonFile << ",";
            jsonFile << "\n";
        }
        jsonFile << "  ],\n";
        
        jsonFile << "  \"uncoveredFunctions\": [\n";
        for (size_t i = 0; i < report.uncoveredFunctions.size(); ++i) {
            jsonFile << "    \"" << report.uncoveredFunctions[i] << "\"";
            if (i < report.uncoveredFunctions.size() - 1) jsonFile << ",";
            jsonFile << "\n";
        }
        jsonFile << "  ]\n";
        jsonFile << "}\n";
        
        jsonFile.close();
        Logger::GetInstance().Info("Coverage report exported to JSON: " + jsonPath);
        return true;
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to export coverage report to JSON: " + std::string(e.what()));
        return false;
    }
}

TestCoverageManager::CoverageReport TestCoverageManager::LoadFromJson(const std::string& jsonPath) {
    CoverageReport report;
    
    try {
        std::ifstream jsonFile(jsonPath);
        if (!jsonFile.is_open()) {
            Logger::GetInstance().Error("Failed to open JSON file for reading: " + jsonPath);
            return report;
        }
        
        // Simple JSON parsing - read the entire file and extract values
        std::string content((std::istreambuf_iterator<char>(jsonFile)),
                           std::istreambuf_iterator<char>());
        jsonFile.close();
        
        // Extract numeric values using simple string parsing
        auto extractFloat = [&content](const std::string& key) -> float {
            std::string searchKey = "\"" + key + "\": ";
            size_t pos = content.find(searchKey);
            if (pos != std::string::npos) {
                pos += searchKey.length();
                size_t endPos = content.find_first_of(",\n}", pos);
                if (endPos != std::string::npos) {
                    std::string value = content.substr(pos, endPos - pos);
                    return std::stof(value);
                }
            }
            return 0.0f;
        };
        
        auto extractInt = [&content](const std::string& key) -> int {
            std::string searchKey = "\"" + key + "\": ";
            size_t pos = content.find(searchKey);
            if (pos != std::string::npos) {
                pos += searchKey.length();
                size_t endPos = content.find_first_of(",\n}", pos);
                if (endPos != std::string::npos) {
                    std::string value = content.substr(pos, endPos - pos);
                    return std::stoi(value);
                }
            }
            return 0;
        };
        
        auto extractString = [&content](const std::string& key) -> std::string {
            std::string searchKey = "\"" + key + "\": \"";
            size_t pos = content.find(searchKey);
            if (pos != std::string::npos) {
                pos += searchKey.length();
                size_t endPos = content.find("\"", pos);
                if (endPos != std::string::npos) {
                    return content.substr(pos, endPos - pos);
                }
            }
            return "";
        };
        
        report.linesCovered = extractFloat("linesCovered");
        report.branchesCovered = extractFloat("branchesCovered");
        report.totalLines = extractInt("totalLines");
        report.coveredLines = extractInt("coveredLines");
        report.totalBranches = extractInt("totalBranches");
        report.coveredBranches = extractInt("coveredBranches");
        report.timestamp = extractString("timestamp");
        report.reportPath = extractString("reportPath");
        
        Logger::GetInstance().Info("Coverage report loaded from JSON: " + jsonPath);
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to load coverage report from JSON: " + std::string(e.what()));
    }
    
    return report;
}

TestCoverageManager::CoverageReport TestCoverageManager::ParseXmlReport(const std::string& xmlPath) {
    CoverageReport report;
    
    try {
        std::ifstream xmlFile(xmlPath);
        if (!xmlFile.is_open()) {
            Logger::GetInstance().Error("Failed to open XML coverage report: " + xmlPath);
            return report;
        }
        
        std::string content((std::istreambuf_iterator<char>(xmlFile)),
                           std::istreambuf_iterator<char>());
        xmlFile.close();
        
        // Simple XML parsing using string search (basic approach)
        auto extractAttribute = [&content](const std::string& tag, const std::string& attribute) -> std::string {
            std::string searchPattern = "<" + tag;
            size_t pos = content.find(searchPattern);
            if (pos != std::string::npos) {
                size_t endPos = content.find(">", pos);
                if (endPos != std::string::npos) {
                    std::string tagContent = content.substr(pos, endPos - pos);
                    std::string attrPattern = attribute + "=\"";
                    size_t attrPos = tagContent.find(attrPattern);
                    if (attrPos != std::string::npos) {
                        attrPos += attrPattern.length();
                        size_t attrEndPos = tagContent.find("\"", attrPos);
                        if (attrEndPos != std::string::npos) {
                            return tagContent.substr(attrPos, attrEndPos - attrPos);
                        }
                    }
                }
            }
            return "";
        };
        
        // Extract coverage metrics from the coverage tag
        std::string lineRate = extractAttribute("coverage", "line-rate");
        std::string branchRate = extractAttribute("coverage", "branch-rate");
        std::string linesCovered = extractAttribute("coverage", "lines-covered");
        std::string linesValid = extractAttribute("coverage", "lines-valid");
        std::string branchesCovered = extractAttribute("coverage", "branches-covered");
        std::string branchesValid = extractAttribute("coverage", "branches-valid");
        
        if (!lineRate.empty()) {
            report.linesCovered = std::stof(lineRate) * 100.0f;
        }
        if (!branchRate.empty()) {
            report.branchesCovered = std::stof(branchRate) * 100.0f;
        }
        if (!linesCovered.empty()) {
            report.coveredLines = std::stoi(linesCovered);
        }
        if (!linesValid.empty()) {
            report.totalLines = std::stoi(linesValid);
        }
        if (!branchesCovered.empty()) {
            report.coveredBranches = std::stoi(branchesCovered);
        }
        if (!branchesValid.empty()) {
            report.totalBranches = std::stoi(branchesValid);
        }
        
        // Generate timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        report.timestamp = ss.str();
        
        // Set report path
        std::filesystem::path xmlPathObj(xmlPath);
        std::filesystem::path htmlPath = xmlPathObj.parent_path() / "index.html";
        if (std::filesystem::exists(htmlPath)) {
            report.reportPath = htmlPath.string();
        }
        
        Logger::GetInstance().Info("Parsed XML coverage report: " + xmlPath);
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to parse XML coverage report: " + std::string(e.what()));
    }
    
    return report;
}

std::vector<std::string> TestCoverageManager::FindTestExecutables(const std::string& buildPath) {
    std::vector<std::string> testExecutables;
    
    try {
        std::filesystem::path buildDir(buildPath);
        if (!std::filesystem::exists(buildDir)) {
            Logger::GetInstance().Warning("Build directory not found: " + buildPath);
            return testExecutables;
        }
        
        // List of expected test executables
        std::vector<std::string> testNames = {
            "BulletUtilsTest.exe",
            "CollisionShapeFactoryTest.exe",
            "BulletIntegrationTest.exe",
            "BulletConversionTest.exe",
            "BulletUtilsSimpleTest.exe",
            "CollisionShapeFactorySimpleTest.exe",
            "PhysicsQueriesTest.exe",
            "PhysicsConfigurationTest.exe",
            "MovementComponentComparisonTest.exe",
            "CharacterBehaviorSimpleTest.exe",
            "PhysicsDebugDrawerTest.exe"
        };
        
        for (const auto& testName : testNames) {
            std::filesystem::path testPath = buildDir / testName;
            if (std::filesystem::exists(testPath)) {
                testExecutables.push_back(testPath.string());
                Logger::GetInstance().Debug("Found test executable: " + testPath.string());
            }
        }
        
        Logger::GetInstance().Info("Found " + std::to_string(testExecutables.size()) + " test executables");
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to find test executables: " + std::string(e.what()));
    }
    
    return testExecutables;
}

bool TestCoverageManager::ExecuteCoverageCommand(const std::string& command, const std::string& workingDir) {
    Logger::GetInstance().Info("Executing coverage command in directory: " + workingDir);
    Logger::GetInstance().Debug("Command: " + command);
    
#ifdef _WIN32
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    
    // Create process
    BOOL success = CreateProcessA(
        nullptr,                                    // Application name
        const_cast<char*>(command.c_str()),        // Command line
        nullptr,                                    // Process security attributes
        nullptr,                                    // Thread security attributes
        FALSE,                                      // Inherit handles
        0,                                          // Creation flags
        nullptr,                                    // Environment
        workingDir.c_str(),                        // Current directory
        &si,                                        // Startup info
        &pi                                         // Process info
    );
    
    if (!success) {
        Logger::GetInstance().Error("Failed to create coverage process");
        return false;
    }
    
    // Wait for process to complete
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 300000); // 5 minute timeout
    
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    if (waitResult == WAIT_TIMEOUT) {
        Logger::GetInstance().Error("Coverage analysis timed out");
        return false;
    }
    
    if (exitCode != 0) {
        Logger::GetInstance().Error("Coverage analysis failed with exit code: " + std::to_string(exitCode));
        return false;
    }
    
    Logger::GetInstance().Info("Coverage analysis completed successfully");
    return true;
    
#else
    // For Linux/macOS, use system() call
    int result = std::system(command.c_str());
    if (result != 0) {
        Logger::GetInstance().Error("Coverage analysis failed with exit code: " + std::to_string(result));
        return false;
    }
    
    Logger::GetInstance().Info("Coverage analysis completed successfully");
    return true;
#endif
}

} // namespace Core
} // namespace GameEngine