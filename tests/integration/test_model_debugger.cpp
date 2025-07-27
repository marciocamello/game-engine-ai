#include "../TestUtils.h"
#include "Resource/ModelDebugger.h"
#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include <memory>
#include <string>
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestModelDebuggerBasicAnalysis() {
    TestOutput::PrintTestStart("model debugger basic analysis");

    try {
        // Create a test model
        auto model = std::make_shared<Model>("test_model.obj");
        TestOutput::PrintInfo("Model created, calling CreateDefault...");
        
        model->CreateDefault(); // Creates a default cube mesh
        TestOutput::PrintInfo("CreateDefault completed");

        // Create debugger
        ModelDebugger debugger;
        TestOutput::PrintInfo("ModelDebugger created");
        
        debugger.EnableVerboseLogging(true);
        TestOutput::PrintInfo("Verbose logging enabled");

        // Analyze the model
        TestOutput::PrintInfo("Starting model analysis...");
        auto stats = debugger.AnalyzeModel(model);
        TestOutput::PrintInfo("Model analysis completed");

        // Verify basic statistics
        TestOutput::PrintInfo("Mesh count: " + std::to_string(stats.meshCount));
        TestOutput::PrintInfo("Total vertices: " + std::to_string(stats.totalVertices));
        TestOutput::PrintInfo("Total triangles: " + std::to_string(stats.totalTriangles));
        TestOutput::PrintInfo("Node count: " + std::to_string(stats.nodeCount));

        if (stats.meshCount == 0) {
            TestOutput::PrintTestFail("model debugger basic analysis", "meshCount > 0", "meshCount = 0");
            return false;
        }

        // Generate reports
        std::string statisticsReport = debugger.GenerateStatisticsReport(stats);
        if (statisticsReport.empty()) {
            TestOutput::PrintTestFail("model debugger basic analysis", "non-empty statistics report", "empty report");
            return false;
        }

        TestOutput::PrintInfo("Statistics Report Preview:");
        TestOutput::PrintInfo(statisticsReport.substr(0, std::min(size_t(200), statisticsReport.length())) + "...");

        TestOutput::PrintTestPass("model debugger basic analysis");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("model debugger basic analysis", "no exception", e.what());
        return false;
    }
}

bool TestModelDebuggerMeshAnalysis() {
    TestOutput::PrintTestStart("model debugger mesh analysis");

    try {
        // Create a test model with default mesh
        auto model = std::make_shared<Model>("test_model.obj");
        model->CreateDefault();

        // Create debugger
        ModelDebugger debugger;
        debugger.EnableDetailedMeshAnalysis(true);

        // Analyze meshes
        auto meshAnalyses = debugger.AnalyzeMeshes(model);

        // Verify mesh analysis
        EXPECT_TRUE(!meshAnalyses.empty());
        
        for (const auto& analysis : meshAnalyses) {
            EXPECT_TRUE(analysis.vertexCount > 0);
            EXPECT_TRUE(analysis.triangleCount > 0);
            EXPECT_TRUE(!analysis.name.empty());
        }

        // Generate mesh analysis report
        std::string meshReport = debugger.GenerateMeshAnalysisReport(meshAnalyses);
        EXPECT_TRUE(!meshReport.empty());

        TestOutput::PrintInfo("Mesh Analysis Report Preview:");
        TestOutput::PrintInfo(meshReport.substr(0, 300) + "...");

        TestOutput::PrintTestPass("model debugger mesh analysis");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("model debugger mesh analysis", "no exception", e.what());
        return false;
    }
}

bool TestModelDebuggerPipelineMonitoring() {
    TestOutput::PrintTestStart("model debugger pipeline monitoring");

    try {
        ModelDebugger debugger;
        debugger.EnableVerboseLogging(true);

        // Start pipeline monitoring
        std::string testFile = "test_pipeline.obj";
        debugger.StartPipelineMonitoring(testFile);

        // Simulate pipeline stages
        debugger.LogPipelineStage("FileLoading", "Loading model file from disk");
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
        debugger.LogPipelineStageComplete("FileLoading", true);

        debugger.LogPipelineStage("Parsing", "Parsing model data");
        debugger.LogPipelineMetadata("format", "obj");
        debugger.LogPipelineMetadata("file_size", "1024");
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Simulate work
        debugger.LogPipelineStageComplete("Parsing", true);

        debugger.LogPipelineStage("MeshProcessing", "Processing mesh geometry");
        std::this_thread::sleep_for(std::chrono::milliseconds(15)); // Simulate work
        debugger.LogPipelineStageComplete("MeshProcessing", true);

        debugger.LogPipelineStage("Optimization", "Optimizing mesh data");
        std::this_thread::sleep_for(std::chrono::milliseconds(8)); // Simulate work
        debugger.LogPipelineStageComplete("Optimization", true);

        // Finish monitoring
        auto pipelineReport = debugger.FinishPipelineMonitoring();

        // Verify pipeline report
        EXPECT_EQUAL(pipelineReport.filepath, testFile);
        EXPECT_TRUE(pipelineReport.overallSuccess);
        EXPECT_TRUE(pipelineReport.stages.size() == 4);
        EXPECT_TRUE(pipelineReport.totalDurationMs > 0);

        // Generate pipeline report
        std::string reportStr = debugger.GeneratePipelineReport(pipelineReport);
        EXPECT_TRUE(!reportStr.empty());

        TestOutput::PrintInfo("Pipeline Report Preview:");
        TestOutput::PrintInfo(reportStr.substr(0, 400) + "...");

        TestOutput::PrintTestPass("model debugger pipeline monitoring");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("model debugger pipeline monitoring", "no exception", e.what());
        return false;
    }
}

bool TestModelDebuggerIssueDetection() {
    TestOutput::PrintTestStart("model debugger issue detection");

    try {
        // Create a test model
        auto model = std::make_shared<Model>("test_model.obj");
        model->CreateDefault();

        ModelDebugger debugger;
        
        // Set low thresholds to trigger issues
        debugger.SetPerformanceThresholds(100, 50, 1.0f); // Very low thresholds
        debugger.SetQualityThresholds(1e-3f, 1.0f);

        // Analyze model
        auto stats = debugger.AnalyzeModel(model);

        // Detect issues
        auto performanceIssues = debugger.DetectPerformanceIssues(stats);
        auto qualityIssues = debugger.DetectQualityIssues(stats);
        auto optimizationSuggestions = debugger.GenerateOptimizationSuggestions(stats);
        auto compatibilitySuggestions = debugger.GenerateCompatibilitySuggestions(stats);

        // With low thresholds, we should detect some issues
        TestOutput::PrintInfo("Performance Issues: " + std::to_string(performanceIssues.size()));
        TestOutput::PrintInfo("Quality Issues: " + std::to_string(qualityIssues.size()));
        TestOutput::PrintInfo("Optimization Suggestions: " + std::to_string(optimizationSuggestions.size()));
        TestOutput::PrintInfo("Compatibility Suggestions: " + std::to_string(compatibilitySuggestions.size()));

        // Print some examples if found
        if (!performanceIssues.empty()) {
            TestOutput::PrintInfo("Example Performance Issue: " + performanceIssues[0]);
        }
        if (!optimizationSuggestions.empty()) {
            TestOutput::PrintInfo("Example Optimization Suggestion: " + optimizationSuggestions[0]);
        }

        TestOutput::PrintTestPass("model debugger issue detection");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("model debugger issue detection", "no exception", e.what());
        return false;
    }
}

bool TestModelDebuggerDiagnosticLogging() {
    TestOutput::PrintTestStart("model debugger diagnostic logging");

    try {
        // Test diagnostic logger
        auto& logger = ModelDiagnosticLogger::GetInstance();
        logger.SetLogLevel(ModelDiagnosticLogger::LogLevel::Debug);
        logger.EnableConsoleOutput(true);

        // Test different log levels
        logger.LogTrace("Trace message", "TestComponent", "test.obj");
        logger.LogDebug("Debug message", "TestComponent", "test.obj");
        logger.LogInfo("Info message", "TestComponent", "test.obj");
        logger.LogWarning("Warning message", "TestComponent", "test.obj");
        logger.LogError("Error message", "TestComponent", "test.obj");
        logger.LogCritical("Critical message", "TestComponent", "test.obj");

        // Test context logging
        std::unordered_map<std::string, std::string> context;
        context["key1"] = "value1";
        context["key2"] = "value2";
        logger.LogWithContext(ModelDiagnosticLogger::LogLevel::Info, "Context message", context);

        // Get recent entries
        auto recentEntries = logger.GetRecentEntries(5);
        EXPECT_TRUE(recentEntries.size() > 0);

        TestOutput::PrintInfo("Recent log entries: " + std::to_string(recentEntries.size()));

        TestOutput::PrintTestPass("model debugger diagnostic logging");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("model debugger diagnostic logging", "no exception", e.what());
        return false;
    }
}

bool TestModelDebuggerReportGeneration() {
    TestOutput::PrintTestStart("model debugger report generation");

    try {
        // Create a test model
        auto model = std::make_shared<Model>("test_model.obj");
        model->CreateDefault();

        ModelDebugger debugger;
        
        // Analyze model
        auto stats = debugger.AnalyzeModel(model);

        // Test different report types
        std::string statisticsReport = debugger.GenerateStatisticsReport(stats);
        std::string detailedBreakdown = debugger.GenerateDetailedBreakdown(stats);
        
        auto meshAnalyses = debugger.AnalyzeMeshes(model);
        std::string meshReport = debugger.GenerateMeshAnalysisReport(meshAnalyses);

        // Verify reports are generated
        if (statisticsReport.empty()) {
            TestOutput::PrintTestFail("model debugger report generation", "non-empty statistics report", "empty report");
            return false;
        }

        TestOutput::PrintInfo("Statistics report length: " + std::to_string(statisticsReport.length()));
        TestOutput::PrintInfo("Detailed breakdown length: " + std::to_string(detailedBreakdown.length()));
        TestOutput::PrintInfo("Mesh report length: " + std::to_string(meshReport.length()));

        TestOutput::PrintTestPass("model debugger report generation");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("model debugger report generation", "no exception", e.what());
        return false;
    }
}

bool TestModelDebuggerPerformanceProfiling() {
    TestOutput::PrintTestStart("model debugger performance profiling");

    try {
        ModelDebugger debugger;
        debugger.EnableVerboseLogging(true);
        debugger.StartMemoryProfiling();

        // Test performance profiling
        std::string testFile = "test_performance.obj";
        auto profile = debugger.ProfileModelLoading(testFile);

        // Verify profile data
        if (profile.filepath != testFile) {
            TestOutput::PrintTestFail("model debugger performance profiling", "correct filepath", profile.filepath);
            return false;
        }

        if (profile.totalLoadingTimeMs <= 0) {
            TestOutput::PrintTestFail("model debugger performance profiling", "positive loading time", std::to_string(profile.totalLoadingTimeMs));
            return false;
        }

        // Generate performance report
        std::string performanceReport = debugger.GeneratePerformanceReport(profile);
        if (performanceReport.empty()) {
            TestOutput::PrintTestFail("model debugger performance profiling", "non-empty performance report", "empty report");
            return false;
        }

        TestOutput::PrintInfo("Performance Report Preview:");
        TestOutput::PrintInfo(performanceReport.substr(0, std::min(size_t(300), performanceReport.length())) + "...");

        // Test benchmark functionality
        std::vector<std::string> testFiles = {"test1.obj", "test2.obj", "test3.obj"};
        auto benchmark = debugger.BenchmarkModelLoading(testFiles, "Test Benchmark");

        if (benchmark.testName != "Test Benchmark") {
            TestOutput::PrintTestFail("model debugger performance profiling", "correct benchmark name", benchmark.testName);
            return false;
        }

        std::string benchmarkReport = debugger.GenerateBenchmarkReport(benchmark);
        if (benchmarkReport.empty()) {
            TestOutput::PrintTestFail("model debugger performance profiling", "non-empty benchmark report", "empty report");
            return false;
        }

        TestOutput::PrintInfo("Benchmark Report Preview:");
        TestOutput::PrintInfo(benchmarkReport.substr(0, std::min(size_t(200), benchmarkReport.length())) + "...");

        debugger.StopMemoryProfiling();

        TestOutput::PrintTestPass("model debugger performance profiling");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("model debugger performance profiling", "no exception", e.what());
        return false;
    }
}

int main() {
    TestOutput::PrintHeader("Model Debugger Test Suite");

    TestSuite suite("Model Debugger Tests");

    try {
        suite.RunTest("Basic Analysis", TestModelDebuggerBasicAnalysis);
        suite.RunTest("Mesh Analysis", TestModelDebuggerMeshAnalysis);
        suite.RunTest("Pipeline Monitoring", TestModelDebuggerPipelineMonitoring);
        suite.RunTest("Issue Detection", TestModelDebuggerIssueDetection);
        suite.RunTest("Diagnostic Logging", TestModelDebuggerDiagnosticLogging);
        suite.RunTest("Report Generation", TestModelDebuggerReportGeneration);
        suite.RunTest("Performance Profiling", TestModelDebuggerPerformanceProfiling);

        suite.PrintSummary();
        return suite.AllTestsPassed() ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception: " + std::string(e.what()));
        return 1;
    }
}