#include "../TestUtils.h"
#include "Resource/ModelLoadingException.h"
#include "Resource/ModelValidator.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestModelLoadingExceptionCreation() {
    TestOutput::PrintTestStart("ModelLoadingException creation");

    try {
        // Test basic exception creation
        ModelLoadingException exception(
            ModelLoadingException::ErrorType::FileNotFound,
            "Test file not found",
            "test.obj"
        );

        EXPECT_TRUE(exception.GetErrorType() == ModelLoadingException::ErrorType::FileNotFound);
        EXPECT_TRUE(exception.GetFilePath() == "test.obj");
        EXPECT_TRUE(exception.GetErrorTypeString() == "File Not Found");
        EXPECT_FALSE(exception.IsRecoverable());

        TestOutput::PrintTestPass("ModelLoadingException creation");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelLoadingException creation", "no exception", e.what());
        return false;
    }
}

bool TestModelValidationException() {
    TestOutput::PrintTestStart("ModelValidationException functionality");

    try {
        std::vector<ModelValidationException::ValidationError> errors;
        
        ModelValidationException::ValidationError error1;
        error1.component = "Mesh";
        error1.description = "Invalid vertex data";
        error1.suggestion = "Check vertex buffer";
        error1.isCritical = true;
        errors.push_back(error1);

        ModelValidationException::ValidationError error2;
        error2.component = "Material";
        error2.description = "Missing texture";
        error2.suggestion = "Provide default texture";
        error2.isCritical = false;
        errors.push_back(error2);

        ModelValidationException exception("Validation failed", "test.fbx", errors);

        EXPECT_TRUE(exception.GetValidationErrors().size() == 2);
        EXPECT_TRUE(exception.HasCriticalErrors());

        std::string summary = exception.GetValidationSummary();
        EXPECT_TRUE(summary.find("2 error(s)") != std::string::npos);
        EXPECT_TRUE(summary.find("CRITICAL") != std::string::npos);

        TestOutput::PrintTestPass("ModelValidationException functionality");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelValidationException functionality", "no exception", e.what());
        return false;
    }
}

bool TestModelCorruptionException() {
    TestOutput::PrintTestStart("ModelCorruptionException functionality");

    try {
        ModelCorruptionException exception(
            "File header is invalid",
            "corrupted.obj",
            ModelCorruptionException::CorruptionType::InvalidHeader,
            0
        );

        EXPECT_TRUE(exception.GetCorruptionType() == ModelCorruptionException::CorruptionType::InvalidHeader);
        EXPECT_TRUE(exception.GetCorruptionOffset() == 0);
        EXPECT_TRUE(exception.GetCorruptionTypeString() == "Invalid Header");

        std::string advice = exception.GetRecoveryAdvice();
        EXPECT_FALSE(advice.empty());
        EXPECT_TRUE(advice.find("re-exporting") != std::string::npos);

        TestOutput::PrintTestPass("ModelCorruptionException functionality");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelCorruptionException functionality", "no exception", e.what());
        return false;
    }
}

bool TestModelExceptionFactory() {
    TestOutput::PrintTestStart("ModelExceptionFactory functionality");

    try {
        // Test file not found error creation
        auto fileNotFoundEx = ModelExceptionFactory::CreateFileNotFoundError("nonexistent.obj");
        EXPECT_TRUE(fileNotFoundEx.GetErrorType() == ModelLoadingException::ErrorType::FileNotFound);
        EXPECT_TRUE(fileNotFoundEx.GetFilePath() == "nonexistent.obj");

        // Test unsupported format error
        auto unsupportedEx = ModelExceptionFactory::CreateUnsupportedFormatError("test.xyz", "xyz");
        EXPECT_TRUE(unsupportedEx.GetErrorType() == ModelLoadingException::ErrorType::UnsupportedFormat);
        EXPECT_TRUE(unsupportedEx.GetContext().formatHint == "xyz");

        // Test out of memory error
        auto memoryEx = ModelExceptionFactory::CreateOutOfMemoryError("large.fbx", 1024 * 1024 * 1024);
        EXPECT_TRUE(memoryEx.GetErrorType() == ModelLoadingException::ErrorType::OutOfMemory);
        EXPECT_TRUE(memoryEx.GetSeverity() == ModelLoadingException::Severity::Critical);

        // Test importer error
        auto importerEx = ModelExceptionFactory::CreateImporterError("test.obj", "Assimp failed");
        EXPECT_TRUE(importerEx.GetErrorType() == ModelLoadingException::ErrorType::ImporterError);

        TestOutput::PrintTestPass("ModelExceptionFactory functionality");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelExceptionFactory functionality", "no exception", e.what());
        return false;
    }
}

bool TestModelErrorRecovery() {
    TestOutput::PrintTestStart("ModelErrorRecovery functionality");

    try {
        // Test recovery strategies for different error types
        ModelLoadingException corruptedFileEx(
            ModelLoadingException::ErrorType::CorruptedFile,
            "File is corrupted",
            "corrupted.obj"
        );

        auto strategies = ModelErrorRecovery::GetRecoveryStrategies(corruptedFileEx);
        EXPECT_TRUE(strategies.size() > 0);
        EXPECT_TRUE(ModelErrorRecovery::IsRecoveryPossible(corruptedFileEx));

        // Test that file not found has limited recovery options
        ModelLoadingException fileNotFoundEx(
            ModelLoadingException::ErrorType::FileNotFound,
            "File not found",
            "missing.obj"
        );

        auto fileNotFoundStrategies = ModelErrorRecovery::GetRecoveryStrategies(fileNotFoundEx);
        bool foundFallback = false;
        for (const auto& strategy : fileNotFoundStrategies) {
            if (strategy == ModelErrorRecovery::RecoveryStrategy::FallbackToDefault) {
                foundFallback = true;
                break;
            }
        }
        EXPECT_TRUE(foundFallback);

        // Test recovery attempt
        auto result = ModelErrorRecovery::AttemptRecovery(corruptedFileEx, ModelErrorRecovery::RecoveryStrategy::FallbackToDefault);
        EXPECT_TRUE(result.strategyUsed == ModelErrorRecovery::RecoveryStrategy::FallbackToDefault);

        TestOutput::PrintTestPass("ModelErrorRecovery functionality");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelErrorRecovery functionality", "no exception", e.what());
        return false;
    }
}

bool TestModelValidator() {
    TestOutput::PrintTestStart("ModelValidator functionality");

    try {
        ModelValidator validator;

        // Test validation configuration
        validator.SetValidationLevel(ModelValidator::ValidationSeverity::Warning);
        validator.EnableValidationType(ModelValidator::ValidationType::Performance, true);
        validator.SetPerformanceThresholds(50000, 100000, 50.0f);

        // Test validation type string conversion
        std::string typeStr = ModelValidator::GetValidationTypeString(ModelValidator::ValidationType::GeometryData);
        EXPECT_TRUE(typeStr == "Geometry Data");

        std::string severityStr = ModelValidator::GetValidationSeverityString(ModelValidator::ValidationSeverity::Error);
        EXPECT_TRUE(severityStr == "Error");

        // Test severity parsing
        auto severity = ModelValidator::GetSeverityFromString("warning");
        EXPECT_TRUE(severity == ModelValidator::ValidationSeverity::Warning);

        TestOutput::PrintTestPass("ModelValidator functionality");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelValidator functionality", "no exception", e.what());
        return false;
    }
}

bool TestModelValidatorFileValidation() {
    TestOutput::PrintTestStart("ModelValidator file validation");

    try {
        ModelValidator validator;

        // Create a temporary test file
        std::string testFile = "test_validation.obj";
        std::ofstream file(testFile);
        file << "# Test OBJ file\n";
        file << "v 0.0 0.0 0.0\n";
        file << "v 1.0 0.0 0.0\n";
        file << "v 0.0 1.0 0.0\n";
        file << "f 1 2 3\n";
        file.close();

        // Validate the file
        auto report = validator.ValidateFile(testFile);
        
        EXPECT_TRUE(report.filepath == testFile);
        EXPECT_TRUE(report.format == "obj");
        EXPECT_TRUE(report.validationTime.count() >= 0);

        // Generate and check report
        std::string reportStr = validator.GenerateValidationReport(report);
        EXPECT_FALSE(reportStr.empty());
        EXPECT_TRUE(reportStr.find("Model Validation Report") != std::string::npos);

        // Clean up
        std::filesystem::remove(testFile);

        TestOutput::PrintTestPass("ModelValidator file validation");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelValidator file validation", "no exception", e.what());
        return false;
    }
}

bool TestModelDiagnosticLogger() {
    TestOutput::PrintTestStart("ModelDiagnosticLogger functionality");

    try {
        auto& logger = ModelDiagnosticLogger::GetInstance();

        // Test logging at different levels
        logger.SetLogLevel(ModelDiagnosticLogger::LogLevel::Debug);
        
        logger.LogDebug("Debug message", "TestComponent", "test.obj");
        logger.LogInfo("Info message", "TestComponent", "test.obj");
        logger.LogWarning("Warning message", "TestComponent", "test.obj");
        logger.LogError("Error message", "TestComponent", "test.obj");

        // Test recent entries retrieval
        auto entries = logger.GetRecentEntries(10);
        EXPECT_TRUE(entries.size() >= 3); // At least info, warning, error should be logged

        // Test log level string conversion
        std::string levelStr = ModelDiagnosticLogger::GetLogLevelString(ModelDiagnosticLogger::LogLevel::Warning);
        EXPECT_TRUE(levelStr == "WARN");

        TestOutput::PrintTestPass("ModelDiagnosticLogger functionality");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelDiagnosticLogger functionality", "no exception", e.what());
        return false;
    }
}

bool TestDiagnosticInfoGeneration() {
    TestOutput::PrintTestStart("Diagnostic info generation");

    try {
        ModelValidator validator;

        // Create a test file for diagnostic info
        std::string testFile = "test_diagnostic.obj";
        std::ofstream file(testFile);
        file << "# Test file for diagnostics\n";
        file << "v 0.0 0.0 0.0\n";
        file.close();

        // Generate diagnostic info
        auto diagnosticInfo = validator.GenerateDiagnosticInfo(testFile, "Test error message");

        EXPECT_TRUE(diagnosticInfo.filepath == testFile);
        EXPECT_TRUE(diagnosticInfo.format == "obj");
        EXPECT_TRUE(diagnosticInfo.fileSize > 0);
        EXPECT_TRUE(diagnosticInfo.errorMessage == "Test error message");
        EXPECT_FALSE(diagnosticInfo.platform.empty());
        EXPECT_FALSE(diagnosticInfo.engineVersion.empty());

        // Generate diagnostic report
        std::string report = validator.GenerateDiagnosticReport(diagnosticInfo);
        EXPECT_FALSE(report.empty());
        EXPECT_TRUE(report.find("Model Loading Diagnostic Report") != std::string::npos);
        EXPECT_TRUE(report.find(testFile) != std::string::npos);

        // Clean up
        std::filesystem::remove(testFile);

        TestOutput::PrintTestPass("Diagnostic info generation");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Diagnostic info generation", "no exception", e.what());
        return false;
    }
}

bool TestExceptionDetailedMessages() {
    TestOutput::PrintTestStart("Exception detailed messages");

    try {
        ModelLoadingException::ErrorContext context;
        context.filepath = "test.fbx";
        context.formatHint = "fbx";
        context.fileSize = 1024 * 1024; // 1MB
        context.loadingTime = std::chrono::milliseconds(500);
        context.systemInfo = "Windows 10";
        context.additionalInfo.push_back("Additional context 1");
        context.additionalInfo.push_back("Additional context 2");

        ModelLoadingException exception(
            ModelLoadingException::ErrorType::ImporterError,
            "Importer failed to process file",
            context,
            ModelLoadingException::Severity::Error
        );

        std::string detailedMessage = exception.GetDetailedMessage();
        
        EXPECT_TRUE(detailedMessage.find("Error") != std::string::npos);
        EXPECT_TRUE(detailedMessage.find("Importer Error") != std::string::npos);
        EXPECT_TRUE(detailedMessage.find("test.fbx") != std::string::npos);
        EXPECT_TRUE(detailedMessage.find("fbx") != std::string::npos);
        EXPECT_TRUE(detailedMessage.find("1024") != std::string::npos); // File size
        EXPECT_TRUE(detailedMessage.find("500ms") != std::string::npos); // Loading time
        EXPECT_TRUE(detailedMessage.find("Windows 10") != std::string::npos);
        EXPECT_TRUE(detailedMessage.find("Additional context 1") != std::string::npos);

        TestOutput::PrintTestPass("Exception detailed messages");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Exception detailed messages", "no exception", e.what());
        return false;
    }
}

int main() {
    TestOutput::PrintHeader("Model Error Handling Tests");

    TestSuite suite("Model Error Handling");

    try {
        suite.RunTest("ModelLoadingException Creation", TestModelLoadingExceptionCreation);
        suite.RunTest("ModelValidationException", TestModelValidationException);
        suite.RunTest("ModelCorruptionException", TestModelCorruptionException);
        suite.RunTest("ModelExceptionFactory", TestModelExceptionFactory);
        suite.RunTest("ModelErrorRecovery", TestModelErrorRecovery);
        suite.RunTest("ModelValidator", TestModelValidator);
        suite.RunTest("ModelValidator File Validation", TestModelValidatorFileValidation);
        suite.RunTest("ModelDiagnosticLogger", TestModelDiagnosticLogger);
        suite.RunTest("Diagnostic Info Generation", TestDiagnosticInfoGeneration);
        suite.RunTest("Exception Detailed Messages", TestExceptionDetailedMessages);

        suite.PrintSummary();
        return suite.AllTestsPassed() ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    }
}