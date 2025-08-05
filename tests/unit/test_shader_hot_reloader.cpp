#include "../TestUtils.h"
#include "Graphics/ShaderHotReloader.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestShaderHotReloaderInitialization() {
    TestOutput::PrintTestStart("shader hot reloader initialization");

    ShaderHotReloader reloader;
    
    // Test initialization
    EXPECT_TRUE(reloader.Initialize());
    EXPECT_FALSE(reloader.IsEnabled()); // Should be disabled by default
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), 0);
    
    // Test shutdown
    reloader.Shutdown();

    TestOutput::PrintTestPass("shader hot reloader initialization");
    return true;
}

bool TestShaderHotReloaderConfiguration() {
    TestOutput::PrintTestStart("shader hot reloader configuration");

    ShaderHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Test enable/disable
    reloader.SetEnabled(true);
    EXPECT_TRUE(reloader.IsEnabled());
    
    reloader.SetEnabled(false);
    EXPECT_FALSE(reloader.IsEnabled());

    // Test check interval
    reloader.SetCheckInterval(1.0f);
    EXPECT_NEARLY_EQUAL(reloader.GetCheckInterval(), 1.0f);

    reloader.SetCheckInterval(-1.0f); // Should use default
    EXPECT_NEARLY_EQUAL(reloader.GetCheckInterval(), 0.5f);

    reloader.Shutdown();

    TestOutput::PrintTestPass("shader hot reloader configuration");
    return true;
}

bool TestShaderHotReloaderFileWatching() {
    TestOutput::PrintTestStart("shader hot reloader file watching");

    ShaderHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Create a temporary shader file
    std::string tempDir = "temp_shader_test";
    std::filesystem::create_directory(tempDir);
    
    std::string testShaderPath = tempDir + "/test_shader.glsl";
    {
        std::ofstream file(testShaderPath);
        file << "#version 330 core\nvoid main() { gl_Position = vec4(0.0); }\n";
    }

    // Watch the file
    reloader.WatchShaderFile(testShaderPath);
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), 1);
    EXPECT_TRUE(reloader.IsFileWatched(testShaderPath));

    // Get watched files list
    auto watchedFiles = reloader.GetWatchedFiles();
    EXPECT_EQUAL(watchedFiles.size(), 1);

    // Unwatch the file
    reloader.UnwatchShaderFile(testShaderPath);
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), 0);
    EXPECT_FALSE(reloader.IsFileWatched(testShaderPath));

    // Cleanup
    std::filesystem::remove_all(tempDir);
    reloader.Shutdown();

    TestOutput::PrintTestPass("shader hot reloader file watching");
    return true;
}

bool TestShaderHotReloaderDirectoryWatching() {
    TestOutput::PrintTestStart("shader hot reloader directory watching");

    ShaderHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Create a temporary directory with shader files
    std::string tempDir = "temp_shader_dir_test";
    std::filesystem::create_directory(tempDir);
    
    // Create multiple shader files
    std::vector<std::string> shaderFiles = {
        tempDir + "/vertex.vert",
        tempDir + "/fragment.frag",
        tempDir + "/compute.comp"
    };

    for (const std::string& shaderPath : shaderFiles) {
        std::ofstream file(shaderPath);
        file << "#version 330 core\nvoid main() {}\n";
    }

    // Watch the directory
    reloader.WatchShaderDirectory(tempDir);
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), shaderFiles.size());

    // Verify all files are being watched
    for (const std::string& shaderPath : shaderFiles) {
        EXPECT_TRUE(reloader.IsFileWatched(shaderPath));
    }

    // Cleanup
    std::filesystem::remove_all(tempDir);
    reloader.Shutdown();

    TestOutput::PrintTestPass("shader hot reloader directory watching");
    return true;
}

bool TestShaderHotReloaderCallbacks() {
    TestOutput::PrintTestStart("shader hot reloader callbacks");

    ShaderHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    bool reloadCallbackCalled = false;
    bool errorCallbackCalled = false;
    std::string reloadedFile;
    std::string errorFile;
    std::string errorMessage;

    // Set callbacks
    reloader.SetReloadCallback([&](const std::string& filepath) {
        reloadCallbackCalled = true;
        reloadedFile = filepath;
    });

    reloader.SetErrorCallback([&](const std::string& filepath, const std::string& error) {
        errorCallbackCalled = true;
        errorFile = filepath;
        errorMessage = error;
    });

    // Create a temporary shader file
    std::string tempDir = "temp_callback_test";
    std::filesystem::create_directory(tempDir);
    
    std::string testShaderPath = tempDir + "/callback_test.glsl";
    {
        std::ofstream file(testShaderPath);
        file << "#version 330 core\nvoid main() { gl_Position = vec4(0.0); }\n";
    }

    // Test manual reload (should trigger callback)
    reloader.ReloadShader(testShaderPath);
    EXPECT_TRUE(reloadCallbackCalled);
    EXPECT_EQUAL(reloadedFile, std::filesystem::absolute(testShaderPath).string());

    // Test error callback with non-existent file
    reloader.WatchShaderFile("non_existent_file.glsl");
    EXPECT_TRUE(errorCallbackCalled);

    // Cleanup
    std::filesystem::remove_all(tempDir);
    reloader.Shutdown();

    TestOutput::PrintTestPass("shader hot reloader callbacks");
    return true;
}

bool TestShaderHotReloaderFileExtensions() {
    TestOutput::PrintTestStart("shader hot reloader file extensions");

    ShaderHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Create temporary directory
    std::string tempDir = "temp_extension_test";
    std::filesystem::create_directory(tempDir);

    // Create files with different extensions
    std::vector<std::pair<std::string, bool>> testFiles = {
        {tempDir + "/shader.glsl", true},   // Should be recognized
        {tempDir + "/vertex.vert", true},   // Should be recognized
        {tempDir + "/fragment.frag", true}, // Should be recognized
        {tempDir + "/compute.comp", true},  // Should be recognized
        {tempDir + "/geometry.geom", true}, // Should be recognized
        {tempDir + "/vertex.vs", true},     // Should be recognized
        {tempDir + "/fragment.fs", true},   // Should be recognized
        {tempDir + "/text.txt", false},     // Should NOT be recognized
        {tempDir + "/code.cpp", false},     // Should NOT be recognized
        {tempDir + "/header.h", false}      // Should NOT be recognized
    };

    // Create all test files
    for (const auto& pair : testFiles) {
        std::ofstream file(pair.first);
        file << "// Test file\n";
    }

    // Watch the directory
    reloader.WatchShaderDirectory(tempDir);

    // Count expected shader files
    size_t expectedShaderFiles = 0;
    for (const auto& pair : testFiles) {
        if (pair.second) {
            expectedShaderFiles++;
        }
    }

    EXPECT_EQUAL(reloader.GetWatchedFileCount(), expectedShaderFiles);

    // Verify only shader files are being watched
    for (const auto& pair : testFiles) {
        if (pair.second) {
            EXPECT_TRUE(reloader.IsFileWatched(pair.first));
        } else {
            EXPECT_FALSE(reloader.IsFileWatched(pair.first));
        }
    }

    // Cleanup
    std::filesystem::remove_all(tempDir);
    reloader.Shutdown();

    TestOutput::PrintTestPass("shader hot reloader file extensions");
    return true;
}

bool TestShaderHotReloaderLogicOnly() {
    TestOutput::PrintTestStart("shader hot reloader logic only");

    ShaderHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Test that no files are watched initially
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), 0);
    EXPECT_FALSE(reloader.IsFileWatched("nonexistent.glsl"));

    // Test watched files list is empty
    auto watchedFiles = reloader.GetWatchedFiles();
    EXPECT_EQUAL(watchedFiles.size(), 0);

    // Test update method doesn't crash when no files are watched
    reloader.Update();

    // Test reload all when no files are watched
    reloader.ReloadAllShaders();

    // Test unwatching non-existent file doesn't crash
    reloader.UnwatchShaderFile("nonexistent.glsl");

    reloader.Shutdown();

    TestOutput::PrintTestPass("shader hot reloader logic only");
    return true;
}

bool TestShaderHotReloaderMultipleFiles() {
    TestOutput::PrintTestStart("shader hot reloader multiple files");

    ShaderHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Create temporary directory with multiple shader files
    std::string tempDir = "temp_multiple_test";
    std::filesystem::create_directory(tempDir);

    std::vector<std::string> shaderFiles = {
        tempDir + "/shader1.glsl",
        tempDir + "/shader2.vert",
        tempDir + "/shader3.frag",
        tempDir + "/shader4.comp"
    };

    // Create all shader files
    for (const std::string& shaderPath : shaderFiles) {
        std::ofstream file(shaderPath);
        file << "#version 330 core\nvoid main() {}\n";
    }

    // Watch all files individually
    for (const std::string& shaderPath : shaderFiles) {
        reloader.WatchShaderFile(shaderPath);
    }

    // Test all files are watched
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), shaderFiles.size());
    
    for (const std::string& shaderPath : shaderFiles) {
        EXPECT_TRUE(reloader.IsFileWatched(shaderPath));
    }

    // Test watched files list
    auto watchedFiles = reloader.GetWatchedFiles();
    EXPECT_EQUAL(watchedFiles.size(), shaderFiles.size());

    // Unwatch one file
    reloader.UnwatchShaderFile(shaderFiles[0]);
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), shaderFiles.size() - 1);
    EXPECT_FALSE(reloader.IsFileWatched(shaderFiles[0]));

    // Other files should still be watched
    for (size_t i = 1; i < shaderFiles.size(); ++i) {
        EXPECT_TRUE(reloader.IsFileWatched(shaderFiles[i]));
    }

    // Cleanup
    std::filesystem::remove_all(tempDir);
    reloader.Shutdown();

    TestOutput::PrintTestPass("shader hot reloader multiple files");
    return true;
}

bool TestShaderHotReloaderErrorHandling() {
    TestOutput::PrintTestStart("shader hot reloader error handling");

    ShaderHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    bool errorCallbackCalled = false;
    std::string errorFile;
    std::string errorMessage;

    // Set error callback
    reloader.SetErrorCallback([&](const std::string& filepath, const std::string& error) {
        errorCallbackCalled = true;
        errorFile = filepath;
        errorMessage = error;
    });

    // Test watching non-existent file
    reloader.WatchShaderFile("completely_nonexistent_file.glsl");
    EXPECT_TRUE(errorCallbackCalled);

    // Reset callback state
    errorCallbackCalled = false;
    errorFile.clear();
    errorMessage.clear();

    // Test watching non-existent directory
    reloader.WatchShaderDirectory("completely_nonexistent_directory");
    EXPECT_TRUE(errorCallbackCalled);

    // Reset callback state
    errorCallbackCalled = false;

    // Test watching a file that's not a directory as a directory
    std::string tempFile = "temp_file_not_dir.txt";
    {
        std::ofstream file(tempFile);
        file << "This is not a directory\n";
    }

    reloader.WatchShaderDirectory(tempFile);
    EXPECT_TRUE(errorCallbackCalled);

    // Cleanup
    std::filesystem::remove(tempFile);
    reloader.Shutdown();

    TestOutput::PrintTestPass("shader hot reloader error handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("ShaderHotReloader");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ShaderHotReloader Tests");

        // Run all tests
        allPassed &= suite.RunTest("Shader Hot Reloader Initialization", TestShaderHotReloaderInitialization);
        allPassed &= suite.RunTest("Shader Hot Reloader Configuration", TestShaderHotReloaderConfiguration);
        allPassed &= suite.RunTest("Shader Hot Reloader Logic Only", TestShaderHotReloaderLogicOnly);
        allPassed &= suite.RunTest("Shader Hot Reloader File Watching", TestShaderHotReloaderFileWatching);
        allPassed &= suite.RunTest("Shader Hot Reloader Directory Watching", TestShaderHotReloaderDirectoryWatching);
        allPassed &= suite.RunTest("Shader Hot Reloader Multiple Files", TestShaderHotReloaderMultipleFiles);
        allPassed &= suite.RunTest("Shader Hot Reloader Callbacks", TestShaderHotReloaderCallbacks);
        allPassed &= suite.RunTest("Shader Hot Reloader File Extensions", TestShaderHotReloaderFileExtensions);
        allPassed &= suite.RunTest("Shader Hot Reloader Error Handling", TestShaderHotReloaderErrorHandling);

        // Print detailed summary
        suite.PrintSummary();

        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    } catch (...) {
        TestOutput::PrintError("UNKNOWN TEST ERROR!");
        return 1;
    }
}