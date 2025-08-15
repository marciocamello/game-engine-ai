#include "Animation/AnimationProfiler.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/BlendTree.h"
#include "Animation/IKSolver.h"
#include "Core/Logger.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>

namespace GameEngine {
    namespace Animation {

        // AnimationTimingData implementation
        void AnimationTimingData::AddSample(double timeMs) {
            if (sampleCount == 0) {
                minTimeMs = maxTimeMs = timeMs;
            } else {
                minTimeMs = std::min(minTimeMs, timeMs);
                maxTimeMs = std::max(maxTimeMs, timeMs);
            }
            
            totalTimeMs += timeMs;
            sampleCount++;
            lastTimeMs = timeMs;
            averageTimeMs = totalTimeMs / sampleCount;
        }

        void AnimationTimingData::Reset() {
            averageTimeMs = minTimeMs = maxTimeMs = totalTimeMs = lastTimeMs = 0.0;
            sampleCount = 0;
        }

        // AnimationMemoryStats implementation
        void AnimationMemoryStats::CalculateTotal() {
            totalMemory = skeletonMemory + animationDataMemory + stateMachineMemory + 
                         blendTreeMemory + ikSolverMemory + morphTargetMemory;
        }

        // AnimationValidationReport implementation
        void AnimationValidationReport::CalculateCounts() {
            warningCount = errorCount = performanceIssueCount = 0;
            
            for (const auto& issue : issues) {
                switch (issue.type) {
                    case AnimationValidationIssueType::Warning:
                        warningCount++;
                        break;
                    case AnimationValidationIssueType::Error:
                        errorCount++;
                        break;
                    case AnimationValidationIssueType::Performance:
                        performanceIssueCount++;
                        break;
                }
            }
            
            // Calculate overall score based on issues
            float totalIssues = static_cast<float>(issues.size());
            if (totalIssues == 0) {
                overallScore = 1.0f;
            } else {
                float severitySum = 0.0f;
                for (const auto& issue : issues) {
                    severitySum += issue.severity;
                }
                overallScore = std::max(0.0f, 1.0f - (severitySum / (totalIssues * 2.0f)));
            }
        }

        // AnimationTimer implementation
        AnimationTimer::AnimationTimer() = default;

        void AnimationTimer::Start() {
            m_startTime = std::chrono::high_resolution_clock::now();
            m_isRunning = true;
        }

        void AnimationTimer::Stop() {
            if (m_isRunning) {
                m_endTime = std::chrono::high_resolution_clock::now();
                m_isRunning = false;
            }
        }

        double AnimationTimer::GetElapsedMs() const {
            auto endTime = m_isRunning ? std::chrono::high_resolution_clock::now() : m_endTime;
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime);
            return duration.count() / 1000.0;
        }

        double AnimationTimer::GetElapsedMicroseconds() const {
            auto endTime = m_isRunning ? std::chrono::high_resolution_clock::now() : m_endTime;
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime);
            return static_cast<double>(duration.count());
        }

        bool AnimationTimer::IsRunning() const {
            return m_isRunning;
        }

        // ScopedAnimationTimer implementation
        ScopedAnimationTimer::ScopedAnimationTimer(AnimationTimingData& timingData)
            : m_timingData(timingData) {
            m_timer.Start();
        }

        ScopedAnimationTimer::~ScopedAnimationTimer() {
            m_timer.Stop();
            m_timingData.AddSample(m_timer.GetElapsedMs());
        }

        // AnimationProfiler implementation
        AnimationProfiler::AnimationProfiler() = default;

        AnimationProfiler::~AnimationProfiler() {
            Shutdown();
        }

        bool AnimationProfiler::Initialize() {
            LOG_INFO("AnimationProfiler initialized");
            return true;
        }

        void AnimationProfiler::Shutdown() {
            StopProfiling();
            m_operationTimings.clear();
            m_activeTimers.clear();
            m_frameTimeHistory.clear();
        }

        void AnimationProfiler::StartProfiling() {
            m_isProfilingActive = true;
            m_isPaused = false;
            ResetPerformanceStats();
            LOG_INFO("Animation profiling started");
        }

        void AnimationProfiler::StopProfiling() {
            m_isProfilingActive = false;
            m_isPaused = false;
            
            // Stop all active timers
            for (auto& [name, timer] : m_activeTimers) {
                if (timer.IsRunning()) {
                    timer.Stop();
                }
            }
            
            LOG_INFO("Animation profiling stopped");
        }

        void AnimationProfiler::PauseProfiling() {
            m_isPaused = true;
        }

        void AnimationProfiler::ResumeProfiling() {
            m_isPaused = false;
        }

        bool AnimationProfiler::IsProfilingActive() const {
            return m_isProfilingActive && !m_isPaused;
        }

        void AnimationProfiler::BeginFrame() {
            if (!IsProfilingActive()) return;
            
            m_frameTimer.Start();
        }

        void AnimationProfiler::EndFrame() {
            if (!IsProfilingActive()) return;
            
            m_frameTimer.Stop();
            double frameTime = m_frameTimer.GetElapsedMs();
            
            // Update frame time history
            m_frameTimeHistory.push_back(frameTime);
            if (m_frameTimeHistory.size() > MAX_FRAME_HISTORY) {
                m_frameTimeHistory.erase(m_frameTimeHistory.begin());
            }
            
            // Update performance stats
            m_performanceStats.frameTimeMs = frameTime;
            m_performanceStats.framesSinceLastReset++;
            
            UpdateFrameStats();
            ValidatePerformanceThresholds();
            
            // Call monitoring callback if enabled
            if (m_realTimeMonitoringEnabled && m_monitoringCallback) {
                m_monitoringCallback(m_performanceStats);
            }
        }

        void AnimationProfiler::ResetFrameStats() {
            m_frameTimeHistory.clear();
            m_performanceStats.framesSinceLastReset = 0;
        }

        void AnimationProfiler::BeginOperation(const std::string& operationName) {
            if (!IsProfilingActive()) return;
            
            auto& timer = m_activeTimers[operationName];
            timer.Start();
        }

        void AnimationProfiler::EndOperation(const std::string& operationName) {
            if (!IsProfilingActive()) return;
            
            auto it = m_activeTimers.find(operationName);
            if (it != m_activeTimers.end() && it->second.IsRunning()) {
                it->second.Stop();
                double elapsedTime = it->second.GetElapsedMs();
                m_operationTimings[operationName].AddSample(elapsedTime);
            }
        }

        AnimationTimingData AnimationProfiler::GetOperationTiming(const std::string& operationName) const {
            auto it = m_operationTimings.find(operationName);
            return it != m_operationTimings.end() ? it->second : AnimationTimingData{};
        }

        void AnimationProfiler::UpdateMemoryStats(const AnimationController& controller) {
            if (!m_memoryTrackingEnabled) return;
            
            m_performanceStats.memoryStats.skeletonMemory = CalculateControllerMemoryUsage(controller);
            m_performanceStats.memoryStats.CalculateTotal();
        }

        void AnimationProfiler::UpdateMemoryStats(const AnimationSkeleton& skeleton) {
            if (!m_memoryTrackingEnabled) return;
            
            m_performanceStats.memoryStats.skeletonMemory = CalculateSkeletonMemoryUsage(skeleton);
            m_performanceStats.memoryStats.CalculateTotal();
        }

        AnimationMemoryStats AnimationProfiler::GetMemoryStats() const {
            return m_performanceStats.memoryStats;
        }

        AnimationPerformanceStats AnimationProfiler::GetPerformanceStats() const {
            return m_performanceStats;
        }

        void AnimationProfiler::ResetPerformanceStats() {
            m_performanceStats = AnimationPerformanceStats{};
            
            for (auto& [name, timing] : m_operationTimings) {
                timing.Reset();
            }
            
            ResetFrameStats();
        }

        AnimationValidationReport AnimationProfiler::ValidateAnimationController(const AnimationController& controller) {
            AnimationValidationReport report;
            
            if (!m_validationEnabled) return report;
            
            // Validate controller state
            auto stateMachine = controller.GetStateMachine();
            if (!stateMachine) {
                AddValidationIssue(report, AnimationValidationIssueType::Warning,
                                 "Controller", "No state machine assigned to controller",
                                 "Assign a state machine for proper animation control", 0.3f);
            }
            
            // Validate skeleton
            auto skeleton = controller.GetSkeleton();
            if (!skeleton) {
                AddValidationIssue(report, AnimationValidationIssueType::Error,
                                 "Controller", "No skeleton assigned to controller",
                                 "Assign a valid skeleton to the controller", 0.8f);
            } else {
                auto skeletonReport = ValidateAnimationSkeleton(*skeleton);
                report.issues.insert(report.issues.end(), skeletonReport.issues.begin(), skeletonReport.issues.end());
            }
            
            // Validate state machine if present
            if (stateMachine) {
                auto stateMachineReport = ValidateStateMachine(*stateMachine);
                report.issues.insert(report.issues.end(), stateMachineReport.issues.begin(), stateMachineReport.issues.end());
            }
            
            report.CalculateCounts();
            return report;
        }

        AnimationValidationReport AnimationProfiler::ValidateAnimationSkeleton(const AnimationSkeleton& skeleton) {
            AnimationValidationReport report;
            
            if (!m_validationEnabled) return report;
            
            // Check bone count
            size_t boneCount = skeleton.GetBoneCount();
            if (boneCount == 0) {
                AddValidationIssue(report, AnimationValidationIssueType::Error,
                                 "Skeleton", "Skeleton has no bones",
                                 "Add bones to the skeleton", 1.0f);
            } else if (boneCount > 256) {
                AddValidationIssue(report, AnimationValidationIssueType::Performance,
                                 "Skeleton", "Skeleton has excessive bone count (" + std::to_string(boneCount) + ")",
                                 "Consider using LOD or bone reduction techniques", 0.6f);
            }
            
            // Validate bone hierarchy
            auto rootBones = skeleton.GetRootBones();
            if (rootBones.empty()) {
                AddValidationIssue(report, AnimationValidationIssueType::Error,
                                 "Skeleton", "Skeleton has no root bones",
                                 "Ensure skeleton has at least one root bone", 0.9f);
            } else if (rootBones.size() > 1) {
                AddValidationIssue(report, AnimationValidationIssueType::Warning,
                                 "Skeleton", "Skeleton has multiple root bones",
                                 "Consider using a single root bone for better performance", 0.2f);
            }
            
            // Validate hierarchy using the skeleton's built-in validation
            if (!skeleton.ValidateHierarchy()) {
                AddValidationIssue(report, AnimationValidationIssueType::Error,
                                 "Skeleton", "Skeleton hierarchy validation failed",
                                 "Fix bone hierarchy structure", 0.8f);
            }
            
            report.CalculateCounts();
            return report;
        }

        AnimationValidationReport AnimationProfiler::ValidateStateMachine(const AnimationStateMachine& stateMachine) {
            AnimationValidationReport report;
            
            if (!m_validationEnabled) return report;
            
            // Check if state machine has states
            auto states = stateMachine.GetAllStates();
            if (states.empty()) {
                AddValidationIssue(report, AnimationValidationIssueType::Error,
                                 "StateMachine", "State machine has no states",
                                 "Add states to the state machine", 1.0f);
            }
            
            // Check entry state
            std::string entryState = stateMachine.GetEntryState();
            if (entryState.empty()) {
                AddValidationIssue(report, AnimationValidationIssueType::Warning,
                                 "StateMachine", "No entry state defined",
                                 "Define an entry state for the state machine", 0.4f);
            }
            
            // Validate transitions
            for (const auto& state : states) {
                auto transitions = stateMachine.GetTransitions(state->GetName());
                if (transitions.empty() && states.size() > 1) {
                    AddValidationIssue(report, AnimationValidationIssueType::Warning,
                                     "StateMachine", "State '" + state->GetName() + "' has no transitions",
                                     "Add transitions to connect states", 0.3f);
                }
            }
            
            report.CalculateCounts();
            return report;
        }

        AnimationValidationReport AnimationProfiler::ValidateBlendTree(const BlendTree& blendTree) {
            AnimationValidationReport report;
            
            if (!m_validationEnabled) return report;
            
            // Validate blend tree structure
            if (!blendTree.Validate()) {
                auto errors = blendTree.GetValidationErrors();
                for (const auto& error : errors) {
                    AddValidationIssue(report, AnimationValidationIssueType::Error,
                                     "BlendTree", error,
                                     "Fix blend tree configuration", 0.7f);
                }
            }
            
            report.CalculateCounts();
            return report;
        }

        std::vector<AnimationValidationIssue> AnimationProfiler::DetectPerformanceIssues(const AnimationPerformanceStats& stats) {
            std::vector<AnimationValidationIssue> issues;
            
            // Check frame time
            if (stats.frameTimeMs > m_maxFrameTimeMs) {
                AnimationValidationIssue issue;
                issue.type = AnimationValidationIssueType::Performance;
                issue.category = "Performance";
                issue.description = "Frame time exceeds target (" + std::to_string(stats.frameTimeMs) + "ms > " + std::to_string(m_maxFrameTimeMs) + "ms)";
                issue.suggestion = "Optimize animation updates or reduce animated character count";
                issue.severity = std::min(1.0f, static_cast<float>(stats.frameTimeMs / m_maxFrameTimeMs - 1.0));
                issues.push_back(issue);
            }
            
            // Check character count
            if (stats.animatedCharacterCount > 50) {
                AnimationValidationIssue issue;
                issue.type = AnimationValidationIssueType::Performance;
                issue.category = "Performance";
                issue.description = "High animated character count (" + std::to_string(stats.animatedCharacterCount) + ")";
                issue.suggestion = "Consider using animation LOD or culling distant characters";
                issue.severity = std::min(1.0f, static_cast<float>(stats.animatedCharacterCount) / 100.0f);
                issues.push_back(issue);
            }
            
            return issues;
        }

        std::vector<AnimationValidationIssue> AnimationProfiler::DetectMemoryIssues(const AnimationMemoryStats& memoryStats) {
            std::vector<AnimationValidationIssue> issues;
            
            // Check total memory usage (threshold: 100MB)
            const size_t memoryThreshold = 100 * 1024 * 1024;
            if (memoryStats.totalMemory > memoryThreshold) {
                AnimationValidationIssue issue;
                issue.type = AnimationValidationIssueType::Performance;
                issue.category = "Memory";
                issue.description = "High animation memory usage (" + std::to_string(memoryStats.totalMemory / (1024 * 1024)) + "MB)";
                issue.suggestion = "Consider animation compression or streaming";
                issue.severity = std::min(1.0f, static_cast<float>(memoryStats.totalMemory) / (memoryThreshold * 2.0f));
                issues.push_back(issue);
            }
            
            return issues;
        }

        std::vector<AnimationValidationIssue> AnimationProfiler::DetectAnimationIssues(const AnimationController& controller) {
            std::vector<AnimationValidationIssue> issues;
            
            // This would analyze the controller for common animation issues
            // Implementation depends on specific controller interface
            
            return issues;
        }

        std::string AnimationProfiler::GeneratePerformanceReport() const {
            std::ostringstream report;
            
            report << "=== Animation Performance Report ===\n\n";
            
            // Frame statistics
            report << "Frame Statistics:\n";
            report << "  Current Frame Time: " << std::fixed << std::setprecision(2) << m_performanceStats.frameTimeMs << "ms\n";
            report << "  Target Frame Time: " << m_maxFrameTimeMs << "ms\n";
            report << "  Frames Analyzed: " << m_performanceStats.framesSinceLastReset << "\n\n";
            
            // Operation timings
            report << "Operation Timings:\n";
            for (const auto& [name, timing] : m_operationTimings) {
                if (timing.sampleCount > 0) {
                    report << "  " << name << ":\n";
                    report << "    Average: " << std::fixed << std::setprecision(3) << timing.averageTimeMs << "ms\n";
                    report << "    Min: " << timing.minTimeMs << "ms, Max: " << timing.maxTimeMs << "ms\n";
                    report << "    Samples: " << timing.sampleCount << "\n";
                }
            }
            
            // System statistics
            report << "\nSystem Statistics:\n";
            report << "  Animated Characters: " << m_performanceStats.animatedCharacterCount << "\n";
            report << "  Active Bones: " << m_performanceStats.activeBoneCount << "\n";
            report << "  Active Animations: " << m_performanceStats.activeAnimationCount << "\n";
            report << "  Active IK Solvers: " << m_performanceStats.activeIKSolverCount << "\n";
            
            return report.str();
        }

        std::string AnimationProfiler::GenerateMemoryReport() const {
            std::ostringstream report;
            
            report << "=== Animation Memory Report ===\n\n";
            
            const auto& memStats = m_performanceStats.memoryStats;
            
            report << "Memory Usage (bytes):\n";
            report << "  Skeleton Data: " << memStats.skeletonMemory << " (" << (memStats.skeletonMemory / 1024) << " KB)\n";
            report << "  Animation Data: " << memStats.animationDataMemory << " (" << (memStats.animationDataMemory / 1024) << " KB)\n";
            report << "  State Machines: " << memStats.stateMachineMemory << " (" << (memStats.stateMachineMemory / 1024) << " KB)\n";
            report << "  Blend Trees: " << memStats.blendTreeMemory << " (" << (memStats.blendTreeMemory / 1024) << " KB)\n";
            report << "  IK Solvers: " << memStats.ikSolverMemory << " (" << (memStats.ikSolverMemory / 1024) << " KB)\n";
            report << "  Morph Targets: " << memStats.morphTargetMemory << " (" << (memStats.morphTargetMemory / 1024) << " KB)\n";
            report << "  Total: " << memStats.totalMemory << " (" << (memStats.totalMemory / 1024) << " KB)\n";
            
            return report.str();
        }

        std::string AnimationProfiler::GenerateValidationReport(const AnimationValidationReport& report) const {
            std::ostringstream output;
            
            output << "=== Animation Validation Report ===\n\n";
            output << "Overall Score: " << std::fixed << std::setprecision(2) << (report.overallScore * 100.0f) << "%\n";
            output << "Issues Found: " << report.issues.size() << "\n";
            output << "  Errors: " << report.errorCount << "\n";
            output << "  Warnings: " << report.warningCount << "\n";
            output << "  Performance Issues: " << report.performanceIssueCount << "\n\n";
            
            if (!report.issues.empty()) {
                output << "Issues:\n";
                for (const auto& issue : report.issues) {
                    std::string typeStr;
                    switch (issue.type) {
                        case AnimationValidationIssueType::Error: typeStr = "ERROR"; break;
                        case AnimationValidationIssueType::Warning: typeStr = "WARNING"; break;
                        case AnimationValidationIssueType::Performance: typeStr = "PERFORMANCE"; break;
                    }
                    
                    output << "  [" << typeStr << "] " << issue.category << ": " << issue.description << "\n";
                    output << "    Suggestion: " << issue.suggestion << "\n";
                    output << "    Severity: " << std::fixed << std::setprecision(1) << (issue.severity * 100.0f) << "%\n\n";
                }
            }
            
            return output.str();
        }

        void AnimationProfiler::ExportPerformanceData(const std::string& filename) const {
            std::ofstream file(filename);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file for performance data export: " + filename);
                return;
            }
            
            file << GeneratePerformanceReport() << "\n";
            file << GenerateMemoryReport() << "\n";
            
            LOG_INFO("Performance data exported to: " + filename);
        }

        void AnimationProfiler::SetProfilingEnabled(bool enabled) {
            if (enabled) {
                StartProfiling();
            } else {
                StopProfiling();
            }
        }

        void AnimationProfiler::SetMemoryTrackingEnabled(bool enabled) {
            m_memoryTrackingEnabled = enabled;
        }

        void AnimationProfiler::SetValidationEnabled(bool enabled) {
            m_validationEnabled = enabled;
        }

        void AnimationProfiler::SetPerformanceThresholds(double maxFrameTimeMs, double maxOperationTimeMs) {
            m_maxFrameTimeMs = maxFrameTimeMs;
            m_maxOperationTimeMs = maxOperationTimeMs;
        }

        void AnimationProfiler::EnableRealTimeMonitoring(bool enabled) {
            m_realTimeMonitoringEnabled = enabled;
        }

        bool AnimationProfiler::IsRealTimeMonitoringEnabled() const {
            return m_realTimeMonitoringEnabled;
        }

        void AnimationProfiler::SetMonitoringCallback(std::function<void(const AnimationPerformanceStats&)> callback) {
            m_monitoringCallback = callback;
        }

        void AnimationProfiler::UpdateFrameStats() {
            if (m_frameTimeHistory.empty()) return;
            
            // Calculate average frame time
            double totalTime = 0.0;
            for (double frameTime : m_frameTimeHistory) {
                totalTime += frameTime;
            }
            m_performanceStats.frameTimeMs = totalTime / m_frameTimeHistory.size();
            
            // Calculate CPU usage percentage (simplified)
            m_performanceStats.animationCpuUsagePercent = (m_performanceStats.frameTimeMs / m_maxFrameTimeMs) * 100.0;
        }

        void AnimationProfiler::ValidatePerformanceThresholds() {
            // Check if current performance exceeds thresholds
            if (m_performanceStats.frameTimeMs > m_maxFrameTimeMs) {
                LOG_WARNING("Animation frame time exceeds threshold: " + std::to_string(m_performanceStats.frameTimeMs) + "ms");
            }
            
            for (const auto& [name, timing] : m_operationTimings) {
                if (timing.lastTimeMs > m_maxOperationTimeMs) {
                    LOG_WARNING("Animation operation '" + name + "' exceeds threshold: " + std::to_string(timing.lastTimeMs) + "ms");
                }
            }
        }

        size_t AnimationProfiler::CalculateSkeletonMemoryUsage(const AnimationSkeleton& skeleton) const {
            // Estimate skeleton memory usage
            size_t boneCount = skeleton.GetBoneCount();
            size_t boneSize = sizeof(Bone); // Approximate bone size
            size_t matrixSize = sizeof(Math::Mat4);
            
            return boneCount * (boneSize + matrixSize * 3); // bone data + transforms
        }

        size_t AnimationProfiler::CalculateControllerMemoryUsage(const AnimationController& controller) const {
            // This would calculate the total memory usage of the controller
            // Implementation depends on controller's internal structure
            return sizeof(AnimationController); // Simplified
        }

        void AnimationProfiler::AddValidationIssue(AnimationValidationReport& report, AnimationValidationIssueType type,
                                                  const std::string& category, const std::string& description,
                                                  const std::string& suggestion, float severity) const {
            AnimationValidationIssue issue;
            issue.type = type;
            issue.category = category;
            issue.description = description;
            issue.suggestion = suggestion;
            issue.severity = glm::clamp(severity, 0.0f, 1.0f);
            report.issues.push_back(issue);
        }

        // AnimationProfilerManager implementation
        std::unique_ptr<AnimationProfiler> AnimationProfilerManager::s_instance = nullptr;

        AnimationProfiler& AnimationProfilerManager::GetInstance() {
            if (!s_instance) {
                s_instance = std::make_unique<AnimationProfiler>();
                s_instance->Initialize();
            }
            return *s_instance;
        }

        void AnimationProfilerManager::Initialize() {
            GetInstance(); // Ensure instance is created and initialized
        }

        void AnimationProfilerManager::Shutdown() {
            if (s_instance) {
                s_instance->Shutdown();
                s_instance.reset();
            }
        }

    } // namespace Animation
} // namespace GameEngine