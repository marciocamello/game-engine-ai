#pragma once

#include "../../tests/TestUtils.h"
#include <string>
#include <memory>

namespace GameEngine {
namespace Testing {
namespace Project {

/**
 * Project-specific test utilities that extend the base TestUtils
 */
class ProjectTestUtils {
public:
    /**
     * Initialize a mock game environment for testing
     */
    static bool InitializeMockGameEnvironment();
    
    /**
     * Cleanup mock game environment
     */
    static void CleanupMockGameEnvironment();
    
    /**
     * Create a temporary test asset file
     */
    static std::string CreateTempTestAsset(const std::string& assetType, const std::string& content);
    
    /**
     * Remove temporary test assets
     */
    static void CleanupTempTestAssets();
    
    /**
     * Validate game project structure
     */
    static bool ValidateProjectStructure(const std::string& projectPath);
    
    /**
     * Mock game configuration for testing
     */
    static std::string CreateMockGameConfig(const std::string& projectName);
};

/**
 * Game-specific test fixtures for common testing scenarios
 */
class GameTestFixture {
private:
    bool m_initialized = false;
    std::string m_tempDir;
    
public:
    GameTestFixture();
    ~GameTestFixture();
    
    /**
     * Setup test environment
     */
    bool Setup();
    
    /**
     * Cleanup test environment
     */
    void Cleanup();
    
    /**
     * Get temporary directory for test files
     */
    const std::string& GetTempDirectory() const { return m_tempDir; }
    
    /**
     * Check if fixture is properly initialized
     */
    bool IsInitialized() const { return m_initialized; }
};

/**
 * Performance testing utilities for game projects
 */
class GamePerformanceTest {
public:
    /**
     * Measure game loop performance
     */
    template<typename GameLoopFunc>
    static double MeasureGameLoopPerformance(GameLoopFunc&& gameLoop, int iterations = 100) {
        TestTimer timer;
        
        for (int i = 0; i < iterations; ++i) {
            gameLoop();
        }
        
        return timer.ElapsedMs() / iterations;
    }
    
    /**
     * Validate game performance meets requirements
     */
    template<typename GameLoopFunc>
    static bool ValidateGamePerformance(const std::string& testName, GameLoopFunc&& gameLoop,
                                       double maxFrameTimeMs, int iterations = 100) {
        TestOutput::PrintTestStart(testName);
        
        double avgFrameTime = MeasureGameLoopPerformance(std::forward<GameLoopFunc>(gameLoop), iterations);
        
        TestOutput::PrintTiming(testName, avgFrameTime * iterations, iterations);
        
        if (avgFrameTime <= maxFrameTimeMs) {
            TestOutput::PrintTestPass(testName);
            return true;
        } else {
            std::ostringstream expected, actual;
            expected << "<= " << maxFrameTimeMs << "ms per frame";
            actual << avgFrameTime << "ms per frame";
            TestOutput::PrintTestFail(testName, expected.str(), actual.str());
            return false;
        }
    }
};

/**
 * Asset testing utilities for game projects
 */
class GameAssetTest {
public:
    /**
     * Validate asset loading performance
     */
    static bool ValidateAssetLoadingTime(const std::string& assetPath, double maxLoadTimeMs);
    
    /**
     * Test asset memory usage
     */
    static bool ValidateAssetMemoryUsage(const std::string& assetPath, size_t maxMemoryBytes);
    
    /**
     * Validate asset file integrity
     */
    static bool ValidateAssetIntegrity(const std::string& assetPath);
};

} // namespace Project
} // namespace Testing
} // namespace GameEngine

// Convenience macros for project testing

/**
 * Setup and cleanup game test fixture
 */
#define GAME_TEST_FIXTURE_SETUP() \
    GameEngine::Testing::Project::GameTestFixture fixture; \
    if (!fixture.Setup()) { \
        GameEngine::Testing::TestOutput::PrintError("Failed to setup game test fixture"); \
        return false; \
    }

#define GAME_TEST_FIXTURE_CLEANUP() \
    fixture.Cleanup();

/**
 * Validate game performance with automatic fixture management
 */
#define EXPECT_GAME_PERFORMANCE(testName, gameLoop, maxFrameTimeMs) \
    do { \
        GAME_TEST_FIXTURE_SETUP(); \
        bool result = GameEngine::Testing::Project::GamePerformanceTest::ValidateGamePerformance( \
            testName, gameLoop, maxFrameTimeMs); \
        GAME_TEST_FIXTURE_CLEANUP(); \
        if (!result) return false; \
    } while(0)

/**
 * Validate asset loading with automatic cleanup
 */
#define EXPECT_ASSET_LOADS_FAST(assetPath, maxTimeMs) \
    do { \
        if (!GameEngine::Testing::Project::GameAssetTest::ValidateAssetLoadingTime(assetPath, maxTimeMs)) { \
            return false; \
        } \
    } while(0)