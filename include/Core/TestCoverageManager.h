#pragma once

#include <string>
#include <vector>

namespace GameEngine {
namespace Core {

/**
 * @brief Manages test coverage analysis and reporting
 * 
 * This class provides utilities for generating and analyzing test coverage reports,
 * integrating with OpenCppCoverage on Windows and other coverage tools on other platforms.
 */
class TestCoverageManager {
public:
    /**
     * @brief Structure containing coverage analysis results
     */
    struct CoverageReport {
        float linesCovered = 0.0f;          ///< Percentage of lines covered (0-100)
        float branchesCovered = 0.0f;       ///< Percentage of branches covered (0-100)
        std::vector<std::string> uncoveredFiles;     ///< List of files with incomplete coverage
        std::vector<std::string> uncoveredFunctions; ///< List of functions with incomplete coverage
        
        // Additional metrics
        int totalLines = 0;                 ///< Total number of executable lines
        int coveredLines = 0;               ///< Number of covered lines
        int totalBranches = 0;              ///< Total number of branches
        int coveredBranches = 0;            ///< Number of covered branches
        std::string reportPath;             ///< Path to the HTML coverage report
        std::string timestamp;              ///< When the report was generated
    };

    /**
     * @brief Configuration for coverage analysis
     */
    struct CoverageConfig {
        std::vector<std::string> sourcePaths = {"src", "include"};  ///< Paths to analyze
        std::vector<std::string> excludePaths = {"vcpkg", "build", "tests", "examples"}; ///< Paths to exclude
        std::string outputPath = "coverage_reports";               ///< Output directory
        float lineCoverageThreshold = 100.0f;                     ///< Required line coverage percentage
        float branchCoverageThreshold = 95.0f;                    ///< Required branch coverage percentage
        bool generateHtml = true;                                  ///< Generate HTML report
        bool generateXml = true;                                   ///< Generate XML report
        bool generateJson = true;                                  ///< Generate JSON summary
    };

    /**
     * @brief Generate a coverage report by running tests with coverage analysis
     * 
     * @param outputPath Path where the coverage report should be generated
     * @param config Coverage analysis configuration
     * @return true if coverage report was generated successfully, false otherwise
     */
    static bool GenerateCoverageReport(const std::string& outputPath, const CoverageConfig& config = CoverageConfig{});

    /**
     * @brief Analyze an existing coverage report
     * 
     * @param reportPath Path to the coverage report (XML format)
     * @return CoverageReport structure with analysis results
     */
    static CoverageReport AnalyzeCoverage(const std::string& reportPath);

    /**
     * @brief Check if coverage meets the specified threshold
     * 
     * @param report Coverage report to check
     * @param lineThreshold Minimum required line coverage percentage (default: 100.0)
     * @param branchThreshold Minimum required branch coverage percentage (default: 95.0)
     * @return true if coverage meets both thresholds, false otherwise
     */
    static bool MeetsCoverageThreshold(const CoverageReport& report, 
                                     float lineThreshold = 100.0f, 
                                     float branchThreshold = 95.0f);

    /**
     * @brief Get the path to the coverage analysis tool (OpenCppCoverage on Windows)
     * 
     * @return Path to the coverage tool executable, empty string if not found
     */
    static std::string GetCoverageToolPath();

    /**
     * @brief Check if coverage analysis tools are available
     * 
     * @return true if coverage tools are installed and available, false otherwise
     */
    static bool IsCoverageToolAvailable();

    /**
     * @brief Create a baseline coverage report for future comparisons
     * 
     * @param report Coverage report to use as baseline
     * @param baselinePath Path where baseline should be saved
     * @return true if baseline was created successfully, false otherwise
     */
    static bool CreateBaseline(const CoverageReport& report, const std::string& baselinePath);

    /**
     * @brief Compare current coverage with baseline
     * 
     * @param currentReport Current coverage report
     * @param baselinePath Path to baseline coverage report
     * @return Coverage difference (positive means improvement, negative means regression)
     */
    static CoverageReport CompareWithBaseline(const CoverageReport& currentReport, const std::string& baselinePath);

    /**
     * @brief Generate a summary string of the coverage report
     * 
     * @param report Coverage report to summarize
     * @return Human-readable summary string
     */
    static std::string GenerateSummary(const CoverageReport& report);

    /**
     * @brief Export coverage report to JSON format
     * 
     * @param report Coverage report to export
     * @param jsonPath Path where JSON file should be saved
     * @return true if export was successful, false otherwise
     */
    static bool ExportToJson(const CoverageReport& report, const std::string& jsonPath);

    /**
     * @brief Load coverage report from JSON format
     * 
     * @param jsonPath Path to JSON coverage report
     * @return Loaded coverage report, or empty report if loading failed
     */
    static CoverageReport LoadFromJson(const std::string& jsonPath);

private:
    /**
     * @brief Parse XML coverage report (Cobertura format)
     * 
     * @param xmlPath Path to XML coverage report
     * @return Parsed coverage report
     */
    static CoverageReport ParseXmlReport(const std::string& xmlPath);

    /**
     * @brief Find test executables for coverage analysis
     * 
     * @param buildPath Path to build directory
     * @return List of test executable paths
     */
    static std::vector<std::string> FindTestExecutables(const std::string& buildPath = "build/Release");

    /**
     * @brief Execute coverage analysis command
     * 
     * @param command Coverage analysis command to execute
     * @param workingDir Working directory for the command
     * @return true if command executed successfully, false otherwise
     */
    static bool ExecuteCoverageCommand(const std::string& command, const std::string& workingDir = ".");
};

} // namespace Core
} // namespace GameEngine