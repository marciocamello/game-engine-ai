#include "ProjectTestUtils.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <random>

namespace GameEngine {
namespace Testing {
namespace Project {

// Static variables for managing temporary resources
static std::vector<std::string> s_tempAssets;
static std::string s_tempDirectory;

// ProjectTestUtils implementation
bool ProjectTestUtils::InitializeMockGameEnvironment() {
    try {
        // Create temporary directory for test environment
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        
        s_tempDirectory = "test_temp/mock_game_" + std::to_string(timestamp);
        std::filesystem::create_directories(s_tempDirectory);
        
        // Create basic directory structure
        std::filesystem::create_directories(s_tempDirectory + "/assets");
        std::filesystem::create_directories(s_tempDirectory + "/config");
        std::filesystem::create_directories(s_tempDirectory + "/saves");
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to initialize mock game environment: " << e.what() << std::endl;
        return false;
    }
}

void ProjectTestUtils::CleanupMockGameEnvironment() {
    try {
        if (!s_tempDirectory.empty() && std::filesystem::exists(s_tempDirectory)) {
            std::filesystem::remove_all(s_tempDirectory);
            s_tempDirectory.clear();
        }
        CleanupTempTestAssets();
    } catch (const std::exception& e) {
        std::cerr << "[WARNING] Failed to cleanup mock game environment: " << e.what() << std::endl;
    }
}

std::string ProjectTestUtils::CreateTempTestAsset(const std::string& assetType, const std::string& content) {
    try {
        if (s_tempDirectory.empty()) {
            InitializeMockGameEnvironment();
        }
        
        // Generate unique filename
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);
        
        std::string filename = "test_" + assetType + "_" + std::to_string(dis(gen));
        std::string extension = ".txt"; // Default extension
        
        if (assetType == "texture") extension = ".png";
        else if (assetType == "model") extension = ".obj";
        else if (assetType == "audio") extension = ".wav";
        else if (assetType == "config") extension = ".json";
        
        std::string fullPath = s_tempDirectory + "/assets/" + filename + extension;
        
        // Create the file with content
        std::ofstream file(fullPath);
        if (file.is_open()) {
            file << content;
            file.close();
            
            s_tempAssets.push_back(fullPath);
            return fullPath;
        }
        
        return "";
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to create temp test asset: " << e.what() << std::endl;
        return "";
    }
}

void ProjectTestUtils::CleanupTempTestAssets() {
    for (const auto& asset : s_tempAssets) {
        try {
            if (std::filesystem::exists(asset)) {
                std::filesystem::remove(asset);
            }
        } catch (const std::exception& e) {
            std::cerr << "[WARNING] Failed to remove temp asset " << asset << ": " << e.what() << std::endl;
        }
    }
    s_tempAssets.clear();
}

bool ProjectTestUtils::ValidateProjectStructure(const std::string& projectPath) {
    try {
        // Check required directories
        std::vector<std::string> requiredDirs = {
            "src", "assets", "config"
        };
        
        for (const auto& dir : requiredDirs) {
            std::string fullPath = projectPath + "/" + dir;
            if (!std::filesystem::exists(fullPath)) {
                std::cerr << "[ERROR] Missing required directory: " << fullPath << std::endl;
                return false;
            }
        }
        
        // Check required files
        std::vector<std::string> requiredFiles = {
            "CMakeLists.txt", "README.md"
        };
        
        for (const auto& file : requiredFiles) {
            std::string fullPath = projectPath + "/" + file;
            if (!std::filesystem::exists(fullPath)) {
                std::cerr << "[ERROR] Missing required file: " << fullPath << std::endl;
                return false;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to validate project structure: " << e.what() << std::endl;
        return false;
    }
}

std::string ProjectTestUtils::CreateMockGameConfig(const std::string& projectName) {
    std::string configContent = R"({
    "projectName": ")" + projectName + R"(",
    "version": "1.0.0",
    "requiredModules": [
        "graphics",
        "physics",
        "audio"
    ],
    "optionalModules": [
        "scripting"
    ],
    "settings": {
        "windowWidth": 1280,
        "windowHeight": 720,
        "fullscreen": false,
        "vsync": true
    }
})";
    
    return CreateTempTestAsset("config", configContent);
}

// GameTestFixture implementation
GameTestFixture::GameTestFixture() = default;

GameTestFixture::~GameTestFixture() {
    if (m_initialized) {
        Cleanup();
    }
}

bool GameTestFixture::Setup() {
    if (m_initialized) {
        return true;
    }
    
    if (!ProjectTestUtils::InitializeMockGameEnvironment()) {
        return false;
    }
    
    m_tempDir = s_tempDirectory;
    m_initialized = true;
    return true;
}

void GameTestFixture::Cleanup() {
    if (m_initialized) {
        ProjectTestUtils::CleanupMockGameEnvironment();
        m_initialized = false;
        m_tempDir.clear();
    }
}

// GameAssetTest implementation
bool GameAssetTest::ValidateAssetLoadingTime(const std::string& assetPath, double maxLoadTimeMs) {
    if (!std::filesystem::exists(assetPath)) {
        TestOutput::PrintError("Asset file does not exist: " + assetPath);
        return false;
    }
    
    TestTimer timer;
    
    // Simulate asset loading by reading the file
    try {
        std::ifstream file(assetPath, std::ios::binary);
        if (!file.is_open()) {
            TestOutput::PrintError("Failed to open asset file: " + assetPath);
            return false;
        }
        
        // Read entire file to simulate loading
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<char> buffer(fileSize);
        file.read(buffer.data(), fileSize);
        
        double loadTime = timer.ElapsedMs();
        
        if (loadTime <= maxLoadTimeMs) {
            TestOutput::PrintInfo("Asset loaded in " + TestFrameworkUtils::FormatExecutionTime(loadTime));
            return true;
        } else {
            TestOutput::PrintError("Asset loading too slow: " + 
                                 TestFrameworkUtils::FormatExecutionTime(loadTime) + 
                                 " > " + TestFrameworkUtils::FormatExecutionTime(maxLoadTimeMs));
            return false;
        }
    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception during asset loading: " + std::string(e.what()));
        return false;
    }
}

bool GameAssetTest::ValidateAssetMemoryUsage(const std::string& assetPath, size_t maxMemoryBytes) {
    if (!std::filesystem::exists(assetPath)) {
        TestOutput::PrintError("Asset file does not exist: " + assetPath);
        return false;
    }
    
    try {
        size_t fileSize = std::filesystem::file_size(assetPath);
        
        if (fileSize <= maxMemoryBytes) {
            TestOutput::PrintInfo("Asset memory usage: " + std::to_string(fileSize) + " bytes");
            return true;
        } else {
            TestOutput::PrintError("Asset memory usage too high: " + 
                                 std::to_string(fileSize) + " > " + std::to_string(maxMemoryBytes) + " bytes");
            return false;
        }
    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception during memory validation: " + std::string(e.what()));
        return false;
    }
}

bool GameAssetTest::ValidateAssetIntegrity(const std::string& assetPath) {
    if (!std::filesystem::exists(assetPath)) {
        TestOutput::PrintError("Asset file does not exist: " + assetPath);
        return false;
    }
    
    try {
        std::ifstream file(assetPath, std::ios::binary);
        if (!file.is_open()) {
            TestOutput::PrintError("Failed to open asset file: " + assetPath);
            return false;
        }
        
        // Basic integrity check - ensure file is readable and not empty
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        
        if (fileSize == 0) {
            TestOutput::PrintError("Asset file is empty: " + assetPath);
            return false;
        }
        
        // Try to read first few bytes
        file.seekg(0, std::ios::beg);
        char buffer[16];
        file.read(buffer, std::min(fileSize, size_t(16)));
        
        if (file.gcount() == 0) {
            TestOutput::PrintError("Failed to read asset file: " + assetPath);
            return false;
        }
        
        TestOutput::PrintInfo("Asset integrity validated: " + std::to_string(fileSize) + " bytes");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception during integrity validation: " + std::string(e.what()));
        return false;
    }
}

} // namespace Project
} // namespace Testing
} // namespace GameEngine