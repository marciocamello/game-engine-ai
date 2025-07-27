#include "Resource/ModelDebugger.h"
#include "Graphics/ModelNode.h"
#include "Graphics/Material.h"
#include "Graphics/Animation.h"
#include "Graphics/Skeleton.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <numeric>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#undef max
#undef min
#endif

namespace GameEngine {

ModelDebugger::ModelDebugger() {
    m_validator = std::make_unique<ModelValidator>();
    LogVerbose("ModelDebugger initialized");
}

ModelDebugger::~ModelDebugger() {
    LogVerbose("ModelDebugger destroyed");
}

ModelDebugger::DetailedModelStats ModelDebugger::AnalyzeModel(std::shared_ptr<Model> model) {
    if (!model) {
        LogVerbose("Cannot analyze null model", "AnalyzeModel");
        return DetailedModelStats{};
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    LogVerbose("Starting model analysis for: " + model->GetPath(), "AnalyzeModel");

    DetailedModelStats stats;
    stats.filepath = model->GetPath();
    stats.name = model->GetName();

    try {
        // Analyze different aspects of the model
        AnalyzeHierarchy(model, stats);
        AnalyzeMeshStatistics(model, stats);
        AnalyzeMaterialStatistics(model, stats);
        AnalyzeAnimationStatistics(model, stats);
        AnalyzeMemoryUsage(model, stats);
        AnalyzeGeometryQuality(model, stats);
        AnalyzeBoundingVolumes(model, stats);
        AnalyzePerformanceIndicators(model, stats);

        // Run validation and collect issue counts
        if (m_validator) {
            auto validationReport = m_validator->ValidateModel(model);
            stats.validationIssues = static_cast<uint32_t>(validationReport.issues.size());
            stats.criticalIssues = validationReport.criticalCount;
            stats.errorIssues = validationReport.errorCount;
            stats.warningIssues = validationReport.warningCount;
            stats.infoIssues = validationReport.infoCount;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        LogVerbose("Model analysis completed in " + std::to_string(duration.count()) + "ms", "AnalyzeModel");

    } catch (const std::exception& e) {
        LogVerbose("Error during model analysis: " + std::string(e.what()), "AnalyzeModel");
    }

    return stats;
}

ModelDebugger::DetailedModelStats ModelDebugger::AnalyzeModelFile(const std::string& filepath) {
    LogVerbose("Analyzing model file: " + filepath, "AnalyzeModelFile");

    DetailedModelStats stats;
    stats.filepath = filepath;

    try {
        // Extract format from file extension
        std::filesystem::path path(filepath);
        std::string extension = path.extension().string();
        if (!extension.empty() && extension[0] == '.') {
            extension = extension.substr(1);
        }
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        stats.format = extension;

        // For now, we would need to load the model to analyze it
        // This would integrate with ModelLoader in a real implementation
        LogVerbose("File-based analysis not fully implemented - would need ModelLoader integration", "AnalyzeModelFile");

    } catch (const std::exception& e) {
        LogVerbose("Error analyzing model file: " + std::string(e.what()), "AnalyzeModelFile");
    }

    return stats;
}

std::vector<ModelDebugger::MeshAnalysis> ModelDebugger::AnalyzeMeshes(std::shared_ptr<Model> model) {
    std::vector<MeshAnalysis> analyses;

    if (!model) {
        LogVerbose("Cannot analyze meshes of null model", "AnalyzeMeshes");
        return analyses;
    }

    LogVerbose("Analyzing " + std::to_string(model->GetMeshCount()) + " meshes", "AnalyzeMeshes");

    auto meshes = model->GetMeshes();
    analyses.reserve(meshes.size());

    for (size_t i = 0; i < meshes.size(); ++i) {
        if (meshes[i]) {
            std::string meshName = meshes[i]->GetName();
            if (meshName.empty()) {
                meshName = "Mesh_" + std::to_string(i);
            }
            
            MeshAnalysis analysis = AnalyzeMesh(meshes[i], meshName);
            analyses.push_back(analysis);
        }
    }

    LogVerbose("Completed analysis of " + std::to_string(analyses.size()) + " meshes", "AnalyzeMeshes");
    return analyses;
}

ModelDebugger::MeshAnalysis ModelDebugger::AnalyzeMesh(std::shared_ptr<Mesh> mesh, const std::string& name) {
    MeshAnalysis analysis;
    analysis.name = name;

    if (!mesh) {
        LogVerbose("Cannot analyze null mesh: " + name, "AnalyzeMesh");
        analysis.issues.push_back("Mesh is null");
        return analysis;
    }

    LogVerbose("Analyzing mesh: " + name, "AnalyzeMesh");

    try {
        // Basic statistics
        analysis.vertexCount = mesh->GetVertexCount();
        analysis.triangleCount = mesh->GetTriangleCount();
        analysis.memoryUsage = mesh->GetMemoryUsage();

        // Analyze geometry
        AnalyzeMeshGeometry(mesh, analysis);
        
        // Analyze quality
        AnalyzeMeshQuality(mesh, analysis);
        
        // Analyze performance
        AnalyzeMeshPerformance(mesh, analysis);
        
        // Detect issues
        DetectMeshIssues(mesh, analysis);

        // Check material association
        if (mesh->GetMaterial()) {
            analysis.hasMaterial = true;
            // Material names not implemented in current Material class
            analysis.materialName = "Material_" + std::to_string(mesh->GetMaterialIndex());
        }

        LogVerbose("Mesh analysis completed: " + name + " (" + 
                  std::to_string(analysis.vertexCount) + " vertices, " + 
                  std::to_string(analysis.triangleCount) + " triangles)", "AnalyzeMesh");

    } catch (const std::exception& e) {
        LogVerbose("Error analyzing mesh " + name + ": " + std::string(e.what()), "AnalyzeMesh");
        analysis.issues.push_back("Analysis error: " + std::string(e.what()));
    }

    return analysis;
}

std::string ModelDebugger::GenerateStatisticsReport(const DetailedModelStats& stats) {
    std::stringstream ss;
    
    ss << "=== Model Statistics Report ===\n";
    ss << "File: " << stats.filepath << "\n";
    ss << "Name: " << stats.name << "\n";
    ss << "Format: " << stats.format << "\n";
    if (stats.loadingTimeMs > 0) {
        ss << "Loading Time: " << FormatDuration(stats.loadingTimeMs) << "\n";
    }
    ss << "\n";

    // Hierarchy Statistics
    ss << "Hierarchy:\n";
    ss << "  Nodes: " << stats.nodeCount << "\n";
    ss << "  Max Depth: " << stats.maxDepth << "\n";
    ss << "  Leaf Nodes: " << stats.leafNodeCount << "\n";
    ss << "  Empty Nodes: " << stats.emptyNodeCount << "\n";
    ss << "\n";

    // Mesh Statistics
    ss << "Meshes:\n";
    ss << "  Count: " << stats.meshCount << "\n";
    ss << "  Total Vertices: " << stats.totalVertices << "\n";
    ss << "  Total Triangles: " << stats.totalTriangles << "\n";
    if (stats.meshCount > 0) {
        ss << "  Avg Vertices/Mesh: " << std::fixed << std::setprecision(1) << stats.avgVerticesPerMesh << "\n";
        ss << "  Avg Triangles/Mesh: " << std::fixed << std::setprecision(1) << stats.avgTrianglesPerMesh << "\n";
        ss << "  Min Vertices/Mesh: " << stats.minVerticesPerMesh << "\n";
        ss << "  Max Vertices/Mesh: " << stats.maxVerticesPerMesh << "\n";
        ss << "  Min Triangles/Mesh: " << stats.minTrianglesPerMesh << "\n";
        ss << "  Max Triangles/Mesh: " << stats.maxTrianglesPerMesh << "\n";
    }
    ss << "\n";

    // Material Statistics
    ss << "Materials:\n";
    ss << "  Count: " << stats.materialCount << "\n";
    ss << "  Textures: " << stats.textureCount << "\n";
    ss << "  Unique Textures: " << stats.uniqueTextureCount << "\n";
    ss << "  Meshes without Materials: " << stats.meshesWithoutMaterials << "\n";
    ss << "\n";

    // Animation Statistics
    if (stats.animationCount > 0 || stats.skeletonCount > 0) {
        ss << "Animation:\n";
        ss << "  Animations: " << stats.animationCount << "\n";
        ss << "  Skeletons: " << stats.skeletonCount << "\n";
        ss << "  Skins: " << stats.skinCount << "\n";
        ss << "  Total Bones: " << stats.totalBones << "\n";
        if (stats.totalAnimationDuration > 0) {
            ss << "  Total Duration: " << std::fixed << std::setprecision(2) << stats.totalAnimationDuration << "s\n";
        }
        ss << "\n";
    }

    // Memory Usage
    ss << "Memory Usage:\n";
    ss << "  Total: " << FormatMemorySize(stats.totalMemoryUsage) << "\n";
    ss << "  Vertex Data: " << FormatMemorySize(stats.vertexDataMemory) << "\n";
    ss << "  Index Data: " << FormatMemorySize(stats.indexDataMemory) << "\n";
    ss << "  Texture Data: " << FormatMemorySize(stats.textureMemory) << "\n";
    ss << "  Animation Data: " << FormatMemorySize(stats.animationMemory) << "\n";
    ss << "  Node Data: " << FormatMemorySize(stats.nodeMemory) << "\n";
    ss << "\n";

    // Quality Metrics
    ss << "Quality Metrics:\n";
    if (stats.totalTriangles > 0) {
        ss << "  Avg Triangle Area: " << std::scientific << std::setprecision(3) << stats.averageTriangleArea << "\n";
        ss << "  Min Triangle Area: " << std::scientific << std::setprecision(3) << stats.minTriangleArea << "\n";
        ss << "  Max Triangle Area: " << std::scientific << std::setprecision(3) << stats.maxTriangleArea << "\n";
    }
    ss << "  Degenerate Triangles: " << stats.degenerateTriangles << "\n";
    ss << "  Duplicate Vertices: " << stats.duplicateVertices << "\n";
    ss << "  Cache Efficiency (ACMR): " << std::fixed << std::setprecision(2) << stats.cacheEfficiency << "\n";
    ss << "\n";

    // Bounding Volume
    ss << "Bounding Volume:\n";
    ss << "  Box Min: (" << std::fixed << std::setprecision(3) << stats.boundingBoxMin.x << ", " 
       << stats.boundingBoxMin.y << ", " << stats.boundingBoxMin.z << ")\n";
    ss << "  Box Max: (" << stats.boundingBoxMax.x << ", " 
       << stats.boundingBoxMax.y << ", " << stats.boundingBoxMax.z << ")\n";
    ss << "  Box Size: (" << stats.boundingBoxSize.x << ", " 
       << stats.boundingBoxSize.y << ", " << stats.boundingBoxSize.z << ")\n";
    ss << "  Sphere Center: (" << stats.boundingSphereCenter.x << ", " 
       << stats.boundingSphereCenter.y << ", " << stats.boundingSphereCenter.z << ")\n";
    ss << "  Sphere Radius: " << stats.boundingSphereRadius << "\n";
    ss << "\n";

    // Performance Indicators
    ss << "Performance Indicators:\n";
    ss << "  Has LOD Levels: " << (stats.hasLODLevels ? "Yes" : "No") << "\n";
    ss << "  Is Optimized: " << (stats.isOptimized ? "Yes" : "No") << "\n";
    ss << "  Has Valid Normals: " << (stats.hasValidNormals ? "Yes" : "No") << "\n";
    ss << "  Has Valid UVs: " << (stats.hasValidUVs ? "Yes" : "No") << "\n";
    ss << "  Has Valid Tangents: " << (stats.hasValidTangents ? "Yes" : "No") << "\n";
    ss << "\n";

    // Validation Summary
    if (stats.validationIssues > 0) {
        ss << "Validation Issues:\n";
        ss << "  Total: " << stats.validationIssues << "\n";
        ss << "  Critical: " << stats.criticalIssues << "\n";
        ss << "  Errors: " << stats.errorIssues << "\n";
        ss << "  Warnings: " << stats.warningIssues << "\n";
        ss << "  Info: " << stats.infoIssues << "\n";
    } else {
        ss << "Validation: No issues found\n";
    }

    return ss.str();
}

std::string ModelDebugger::GenerateDetailedBreakdown(const DetailedModelStats& stats) {
    std::stringstream ss;
    
    ss << GenerateStatisticsReport(stats);
    ss << "\n";
    
    ss << "=== Detailed Breakdown ===\n";
    
    // Performance Analysis
    auto performanceIssues = DetectPerformanceIssues(stats);
    if (!performanceIssues.empty()) {
        ss << "\nPerformance Issues:\n";
        for (const auto& issue : performanceIssues) {
            ss << "  - " << issue << "\n";
        }
    }
    
    // Quality Analysis
    auto qualityIssues = DetectQualityIssues(stats);
    if (!qualityIssues.empty()) {
        ss << "\nQuality Issues:\n";
        for (const auto& issue : qualityIssues) {
            ss << "  - " << issue << "\n";
        }
    }
    
    // Optimization Suggestions
    auto optimizationSuggestions = GenerateOptimizationSuggestions(stats);
    if (!optimizationSuggestions.empty()) {
        ss << "\nOptimization Suggestions:\n";
        for (const auto& suggestion : optimizationSuggestions) {
            ss << "  - " << suggestion << "\n";
        }
    }
    
    // Compatibility Suggestions
    auto compatibilitySuggestions = GenerateCompatibilitySuggestions(stats);
    if (!compatibilitySuggestions.empty()) {
        ss << "\nCompatibility Suggestions:\n";
        for (const auto& suggestion : compatibilitySuggestions) {
            ss << "  - " << suggestion << "\n";
        }
    }
    
    return ss.str();
}

std::string ModelDebugger::GenerateMeshAnalysisReport(const std::vector<MeshAnalysis>& analyses) {
    std::stringstream ss;
    
    ss << "=== Mesh Analysis Report ===\n";
    ss << "Total Meshes: " << analyses.size() << "\n\n";
    
    for (size_t i = 0; i < analyses.size(); ++i) {
        const auto& analysis = analyses[i];
        
        ss << "Mesh " << (i + 1) << ": " << analysis.name << "\n";
        ss << "  Vertices: " << analysis.vertexCount << "\n";
        ss << "  Triangles: " << analysis.triangleCount << "\n";
        ss << "  Memory: " << FormatMemorySize(analysis.memoryUsage) << "\n";
        
        // Vertex attributes
        ss << "  Attributes: ";
        std::vector<std::string> attrs;
        if (analysis.hasPositions) attrs.push_back("Positions");
        if (analysis.hasNormals) attrs.push_back("Normals");
        if (analysis.hasTexCoords) attrs.push_back("TexCoords");
        if (analysis.hasTangents) attrs.push_back("Tangents");
        if (analysis.hasColors) attrs.push_back("Colors");
        if (analysis.hasBoneWeights) attrs.push_back("BoneWeights");
        
        if (attrs.empty()) {
            ss << "None";
        } else {
            for (size_t j = 0; j < attrs.size(); ++j) {
                if (j > 0) ss << ", ";
                ss << attrs[j];
            }
        }
        ss << "\n";
        
        // Quality metrics
        if (analysis.averageTriangleArea > 0) {
            ss << "  Avg Triangle Area: " << std::scientific << std::setprecision(3) << analysis.averageTriangleArea << "\n";
        }
        if (analysis.degenerateTriangles > 0) {
            ss << "  Degenerate Triangles: " << analysis.degenerateTriangles << "\n";
        }
        if (analysis.duplicateVertices > 0) {
            ss << "  Duplicate Vertices: " << analysis.duplicateVertices << "\n";
        }
        if (analysis.cacheEfficiency > 0) {
            ss << "  Cache Efficiency: " << std::fixed << std::setprecision(2) << analysis.cacheEfficiency << "\n";
        }
        
        // Material
        ss << "  Material: " << (analysis.hasMaterial ? analysis.materialName : "None") << "\n";
        
        // Performance flags
        ss << "  Optimized: " << (analysis.isOptimized ? "Yes" : "No") << "\n";
        if (analysis.needsOptimization) {
            ss << "  Needs Optimization: Yes\n";
        }
        
        // Issues
        if (!analysis.issues.empty()) {
            ss << "  Issues:\n";
            for (const auto& issue : analysis.issues) {
                ss << "    - " << issue << "\n";
            }
        }
        
        // Suggestions
        if (!analysis.suggestions.empty()) {
            ss << "  Suggestions:\n";
            for (const auto& suggestion : analysis.suggestions) {
                ss << "    - " << suggestion << "\n";
            }
        }
        
        ss << "\n";
    }
    
    return ss.str();
}

void ModelDebugger::EnableVerboseLogging(bool enabled) {
    m_verboseLogging = enabled;
    LogVerbose("Verbose logging " + std::string(enabled ? "enabled" : "disabled"));
}

void ModelDebugger::SetLogLevel(ModelDiagnosticLogger::LogLevel level) {
    ModelDiagnosticLogger::GetInstance().SetLogLevel(level);
    LogVerbose("Log level set to " + ModelDiagnosticLogger::GetLogLevelString(level));
}

void ModelDebugger::SetLogOutputFile(const std::string& filepath) {
    ModelDiagnosticLogger::GetInstance().SetOutputFile(filepath);
    ModelDiagnosticLogger::GetInstance().EnableFileOutput(true);
    LogVerbose("Log output file set to: " + filepath);
}

void ModelDebugger::StartPipelineMonitoring(const std::string& filepath) {
    m_pipelineMonitoring = true;
    m_currentPipeline = PipelineReport{};
    m_currentPipeline.filepath = filepath;
    m_currentPipeline.overallStartTime = std::chrono::high_resolution_clock::now();
    m_activeStages.clear();
    
    LogVerbose("Started pipeline monitoring for: " + filepath, "PipelineMonitor");
}

void ModelDebugger::LogPipelineStage(const std::string& stageName, const std::string& description) {
    if (!m_pipelineMonitoring) return;
    
    PipelineStage stage;
    stage.name = stageName;
    stage.description = description;
    stage.startTime = std::chrono::high_resolution_clock::now();
    
    m_activeStages[stageName] = stage;
    
    LogVerbose("Pipeline stage started: " + stageName + " - " + description, "PipelineMonitor");
}

void ModelDebugger::LogPipelineStageComplete(const std::string& stageName, bool success, const std::string& errorMessage) {
    if (!m_pipelineMonitoring) return;
    
    auto it = m_activeStages.find(stageName);
    if (it != m_activeStages.end()) {
        it->second.endTime = std::chrono::high_resolution_clock::now();
        it->second.success = success;
        it->second.errorMessage = errorMessage;
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(it->second.endTime - it->second.startTime);
        it->second.durationMs = duration.count() / 1000.0f;
        
        m_currentPipeline.stages.push_back(it->second);
        
        LogVerbose("Pipeline stage completed: " + stageName + " (" + 
                  FormatDuration(it->second.durationMs) + ", " + 
                  (success ? "success" : "failed") + ")", "PipelineMonitor");
        
        m_activeStages.erase(it);
    }
}

void ModelDebugger::LogPipelineMetadata(const std::string& key, const std::string& value) {
    if (!m_pipelineMonitoring) return;
    
    m_currentPipeline.globalMetadata[key] = value;
    LogVerbose("Pipeline metadata: " + key + " = " + value, "PipelineMonitor");
}

ModelDebugger::PipelineReport ModelDebugger::FinishPipelineMonitoring() {
    if (!m_pipelineMonitoring) {
        return PipelineReport{};
    }
    
    m_currentPipeline.overallEndTime = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        m_currentPipeline.overallEndTime - m_currentPipeline.overallStartTime);
    m_currentPipeline.totalDurationMs = totalDuration.count() / 1000.0f;
    
    // Calculate overall success
    m_currentPipeline.overallSuccess = true;
    for (const auto& stage : m_currentPipeline.stages) {
        if (!stage.success) {
            m_currentPipeline.overallSuccess = false;
            break;
        }
    }
    
    // Calculate performance breakdown
    for (const auto& stage : m_currentPipeline.stages) {
        if (stage.name.find("FileIO") != std::string::npos || stage.name.find("Loading") != std::string::npos) {
            m_currentPipeline.fileIOTimeMs += stage.durationMs;
        } else if (stage.name.find("Parsing") != std::string::npos) {
            m_currentPipeline.parsingTimeMs += stage.durationMs;
        } else if (stage.name.find("Mesh") != std::string::npos) {
            m_currentPipeline.meshProcessingTimeMs += stage.durationMs;
        } else if (stage.name.find("Material") != std::string::npos) {
            m_currentPipeline.materialProcessingTimeMs += stage.durationMs;
        } else if (stage.name.find("Optimization") != std::string::npos) {
            m_currentPipeline.optimizationTimeMs += stage.durationMs;
        } else if (stage.name.find("Validation") != std::string::npos) {
            m_currentPipeline.validationTimeMs += stage.durationMs;
        }
    }
    
    LogVerbose("Pipeline monitoring finished: " + m_currentPipeline.filepath + " (" + 
              FormatDuration(m_currentPipeline.totalDurationMs) + ", " + 
              (m_currentPipeline.overallSuccess ? "success" : "failed") + ")", "PipelineMonitor");
    
    m_pipelineMonitoring = false;
    return m_currentPipeline;
}

std::string ModelDebugger::GeneratePipelineReport(const PipelineReport& report) {
    std::stringstream ss;
    
    ss << "=== Model Loading Pipeline Report ===\n";
    ss << "File: " << report.filepath << "\n";
    ss << "Total Duration: " << FormatDuration(report.totalDurationMs) << "\n";
    ss << "Overall Success: " << (report.overallSuccess ? "Yes" : "No") << "\n";
    ss << "Stages: " << report.stages.size() << "\n";
    ss << "\n";
    
    // Performance breakdown
    ss << "Performance Breakdown:\n";
    ss << "  File I/O: " << FormatDuration(report.fileIOTimeMs) << " (" << 
          FormatPercentage(report.fileIOTimeMs / report.totalDurationMs * 100.0f) << ")\n";
    ss << "  Parsing: " << FormatDuration(report.parsingTimeMs) << " (" << 
          FormatPercentage(report.parsingTimeMs / report.totalDurationMs * 100.0f) << ")\n";
    ss << "  Mesh Processing: " << FormatDuration(report.meshProcessingTimeMs) << " (" << 
          FormatPercentage(report.meshProcessingTimeMs / report.totalDurationMs * 100.0f) << ")\n";
    ss << "  Material Processing: " << FormatDuration(report.materialProcessingTimeMs) << " (" << 
          FormatPercentage(report.materialProcessingTimeMs / report.totalDurationMs * 100.0f) << ")\n";
    ss << "  Optimization: " << FormatDuration(report.optimizationTimeMs) << " (" << 
          FormatPercentage(report.optimizationTimeMs / report.totalDurationMs * 100.0f) << ")\n";
    ss << "  Validation: " << FormatDuration(report.validationTimeMs) << " (" << 
          FormatPercentage(report.validationTimeMs / report.totalDurationMs * 100.0f) << ")\n";
    ss << "\n";
    
    // Stage details
    ss << "Stage Details:\n";
    for (size_t i = 0; i < report.stages.size(); ++i) {
        const auto& stage = report.stages[i];
        ss << "  " << (i + 1) << ". " << stage.name << "\n";
        ss << "     Description: " << stage.description << "\n";
        ss << "     Duration: " << FormatDuration(stage.durationMs) << "\n";
        ss << "     Success: " << (stage.success ? "Yes" : "No") << "\n";
        if (!stage.errorMessage.empty()) {
            ss << "     Error: " << stage.errorMessage << "\n";
        }
        if (!stage.metadata.empty()) {
            ss << "     Metadata:\n";
            for (const auto& pair : stage.metadata) {
                ss << "       " << pair.first << ": " << pair.second << "\n";
            }
        }
        ss << "\n";
    }
    
    // Global metadata
    if (!report.globalMetadata.empty()) {
        ss << "Global Metadata:\n";
        for (const auto& pair : report.globalMetadata) {
            ss << "  " << pair.first << ": " << pair.second << "\n";
        }
    }
    
    return ss.str();
}

std::vector<std::string> ModelDebugger::DetectPerformanceIssues(const DetailedModelStats& stats) {
    std::vector<std::string> issues;
    
    // Check vertex count
    if (stats.totalVertices > m_maxVertices) {
        issues.push_back("High vertex count: " + std::to_string(stats.totalVertices) + 
                        " (threshold: " + std::to_string(m_maxVertices) + ")");
    }
    
    // Check triangle count
    if (stats.totalTriangles > m_maxTriangles) {
        issues.push_back("High triangle count: " + std::to_string(stats.totalTriangles) + 
                        " (threshold: " + std::to_string(m_maxTriangles) + ")");
    }
    
    // Check memory usage
    float memoryMB = stats.totalMemoryUsage / 1024.0f / 1024.0f;
    if (memoryMB > m_maxMemoryMB) {
        issues.push_back("High memory usage: " + std::to_string(memoryMB) + " MB" + 
                        " (threshold: " + std::to_string(m_maxMemoryMB) + " MB)");
    }
    
    // Check cache efficiency
    if (stats.cacheEfficiency > m_maxCacheThreshold) {
        issues.push_back("Poor cache efficiency (ACMR): " + std::to_string(stats.cacheEfficiency) + 
                        " (threshold: " + std::to_string(m_maxCacheThreshold) + ")");
    }
    
    // Check for many small meshes
    if (stats.meshCount > 100 && stats.avgVerticesPerMesh < 100) {
        issues.push_back("Many small meshes detected (" + std::to_string(stats.meshCount) + 
                        " meshes, avg " + std::to_string(static_cast<int>(stats.avgVerticesPerMesh)) + " vertices)");
    }
    
    return issues;
}

std::vector<std::string> ModelDebugger::DetectQualityIssues(const DetailedModelStats& stats) {
    std::vector<std::string> issues;
    
    // Check for degenerate triangles
    if (stats.degenerateTriangles > 0) {
        issues.push_back("Degenerate triangles found: " + std::to_string(stats.degenerateTriangles));
    }
    
    // Check for duplicate vertices
    if (stats.duplicateVertices > 0) {
        issues.push_back("Duplicate vertices found: " + std::to_string(stats.duplicateVertices));
    }
    
    // Check for missing normals
    if (!stats.hasValidNormals) {
        issues.push_back("Missing or invalid normals detected");
    }
    
    // Check for missing UVs
    if (!stats.hasValidUVs) {
        issues.push_back("Missing or invalid texture coordinates detected");
    }
    
    // Check for meshes without materials
    if (stats.meshesWithoutMaterials > 0) {
        issues.push_back("Meshes without materials: " + std::to_string(stats.meshesWithoutMaterials));
    }
    
    // Check triangle area distribution
    if (stats.minTriangleArea < m_minTriangleArea) {
        issues.push_back("Very small triangles detected (min area: " + 
                        std::to_string(stats.minTriangleArea) + ")");
    }
    
    return issues;
}

std::vector<std::string> ModelDebugger::GenerateOptimizationSuggestions(const DetailedModelStats& stats) {
    std::vector<std::string> suggestions;
    
    // Vertex optimization
    if (stats.duplicateVertices > 0) {
        suggestions.push_back("Remove duplicate vertices to reduce memory usage");
    }
    
    // Cache optimization
    if (stats.cacheEfficiency > 1.5f) {
        suggestions.push_back("Optimize vertex cache ordering to improve rendering performance");
    }
    
    // Mesh consolidation
    if (stats.meshCount > 50 && stats.avgVerticesPerMesh < 500) {
        suggestions.push_back("Consider consolidating small meshes to reduce draw calls");
    }
    
    // LOD suggestions
    if (stats.totalTriangles > 50000 && !stats.hasLODLevels) {
        suggestions.push_back("Generate LOD levels for better performance at distance");
    }
    
    // Normal generation
    if (!stats.hasValidNormals) {
        suggestions.push_back("Generate smooth normals for better lighting");
    }
    
    // Tangent generation
    if (!stats.hasValidTangents && stats.hasValidUVs) {
        suggestions.push_back("Generate tangent vectors for normal mapping support");
    }
    
    // Memory optimization
    float memoryMB = stats.totalMemoryUsage / 1024.0f / 1024.0f;
    if (memoryMB > 50.0f) {
        suggestions.push_back("Consider texture compression or mesh simplification to reduce memory usage");
    }
    
    return suggestions;
}

std::vector<std::string> ModelDebugger::GenerateCompatibilitySuggestions(const DetailedModelStats& stats) {
    std::vector<std::string> suggestions;
    
    // Format suggestions
    if (stats.format == "fbx") {
        suggestions.push_back("Consider converting to GLTF format for better web compatibility");
    }
    
    // Vertex count warnings
    if (stats.totalVertices > 65536) {
        suggestions.push_back("Model exceeds 16-bit index range - ensure 32-bit indices are supported");
    }
    
    // Animation compatibility
    if (stats.animationCount > 0 && stats.totalBones > 64) {
        suggestions.push_back("High bone count may not be supported on all hardware");
    }
    
    // Texture suggestions
    if (stats.textureCount > 16) {
        suggestions.push_back("High texture count may exceed some hardware limits");
    }
    
    return suggestions;
}

// Performance profiling implementation

void ModelDebugger::StartPerformanceProfiling(const std::string& filepath) {
    m_performanceProfiling = true;
    m_currentProfile = PerformanceProfile{};
    m_currentProfile.filepath = filepath;
    m_currentProfile.startTime = std::chrono::high_resolution_clock::now();
    m_stageTimings.clear();
    
    if (m_memoryProfiling) {
        m_currentProfile.initialMemoryUsage = GetCurrentMemoryUsage();
        m_baselineMemory = m_currentProfile.initialMemoryUsage;
    }
    
    LogVerbose("Started performance profiling for: " + filepath, "StartPerformanceProfiling");
}

void ModelDebugger::LogProfilingStage(const std::string& stageName, float durationMs) {
    if (!m_performanceProfiling) return;
    
    m_stageTimings[stageName] = durationMs;
    
    // Categorize timing into appropriate buckets
    if (stageName.find("FileIO") != std::string::npos || stageName.find("Loading") != std::string::npos) {
        m_currentProfile.fileIOTimeMs += durationMs;
    } else if (stageName.find("Parsing") != std::string::npos) {
        m_currentProfile.parsingTimeMs += durationMs;
    } else if (stageName.find("Mesh") != std::string::npos) {
        m_currentProfile.meshProcessingTimeMs += durationMs;
    } else if (stageName.find("Material") != std::string::npos) {
        m_currentProfile.materialProcessingTimeMs += durationMs;
    } else if (stageName.find("Optimization") != std::string::npos) {
        m_currentProfile.optimizationTimeMs += durationMs;
    } else if (stageName.find("Validation") != std::string::npos) {
        m_currentProfile.validationTimeMs += durationMs;
    }
    
    LogVerbose("Profiling stage: " + stageName + " took " + FormatDuration(durationMs), "LogProfilingStage");
}

void ModelDebugger::LogMemoryUsage(const std::string& stage, size_t memoryBytes) {
    if (!m_performanceProfiling || !m_memoryProfiling) return;
    
    m_memorySnapshots.emplace_back(stage, memoryBytes);
    
    if (memoryBytes > m_currentProfile.peakMemoryUsage) {
        m_currentProfile.peakMemoryUsage = memoryBytes;
    }
    
    LogVerbose("Memory usage at " + stage + ": " + FormatMemorySize(memoryBytes), "LogMemoryUsage");
}

ModelDebugger::PerformanceProfile ModelDebugger::FinishPerformanceProfiling() {
    if (!m_performanceProfiling) {
        return PerformanceProfile{};
    }
    
    m_currentProfile.endTime = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        m_currentProfile.endTime - m_currentProfile.startTime);
    m_currentProfile.totalLoadingTimeMs = totalDuration.count() / 1000.0f;
    
    if (m_memoryProfiling) {
        m_currentProfile.finalMemoryUsage = GetCurrentMemoryUsage();
        if (m_currentProfile.finalMemoryUsage > m_currentProfile.initialMemoryUsage) {
            m_currentProfile.memoryLeakBytes = m_currentProfile.finalMemoryUsage - m_currentProfile.initialMemoryUsage;
        }
    }
    
    // Generate optimization suggestions
    m_currentProfile.optimizationSuggestions = GenerateLoadingOptimizations(m_currentProfile);
    
    LogVerbose("Performance profiling completed: " + m_currentProfile.filepath + " (" + 
              FormatDuration(m_currentProfile.totalLoadingTimeMs) + ")", "FinishPerformanceProfiling");
    
    m_performanceProfiling = false;
    return m_currentProfile;
}

ModelDebugger::PerformanceProfile ModelDebugger::ProfileModelLoading(const std::string& filepath) {
    StartPerformanceProfiling(filepath);
    
    // This would integrate with actual model loading
    // For now, simulate the profiling process
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Simulate file I/O
        LogProfilingStage("FileIO", 10.0f);
        LogMemoryUsage("After FileIO", GetCurrentMemoryUsage());
        
        // Simulate parsing
        LogProfilingStage("Parsing", 25.0f);
        LogMemoryUsage("After Parsing", GetCurrentMemoryUsage());
        
        // Simulate mesh processing
        LogProfilingStage("MeshProcessing", 15.0f);
        LogMemoryUsage("After MeshProcessing", GetCurrentMemoryUsage());
        
        // Simulate material processing
        LogProfilingStage("MaterialProcessing", 8.0f);
        LogMemoryUsage("After MaterialProcessing", GetCurrentMemoryUsage());
        
        // Simulate optimization
        LogProfilingStage("Optimization", 12.0f);
        LogMemoryUsage("After Optimization", GetCurrentMemoryUsage());
        
        // Simulate validation
        LogProfilingStage("Validation", 5.0f);
        LogMemoryUsage("After Validation", GetCurrentMemoryUsage());
        
    } catch (const std::exception& e) {
        LogVerbose("Error during model loading profiling: " + std::string(e.what()), "ProfileModelLoading");
    }
    
    return FinishPerformanceProfiling();
}

ModelDebugger::LoadingBenchmark ModelDebugger::BenchmarkModelLoading(const std::vector<std::string>& testFiles, const std::string& benchmarkName) {
    LoadingBenchmark benchmark;
    benchmark.testName = benchmarkName.empty() ? "Model Loading Benchmark" : benchmarkName;
    benchmark.testFiles = testFiles;
    
    LogVerbose("Starting benchmark: " + benchmark.testName + " with " + 
              std::to_string(testFiles.size()) + " files", "BenchmarkModelLoading");
    
    float totalTime = 0.0f;
    
    for (const auto& filepath : testFiles) {
        try {
            auto profile = ProfileModelLoading(filepath);
            benchmark.profiles.push_back(profile);
            
            totalTime += profile.totalLoadingTimeMs;
            benchmark.minLoadingTime = std::min(benchmark.minLoadingTime, profile.totalLoadingTimeMs);
            benchmark.maxLoadingTime = std::max(benchmark.maxLoadingTime, profile.totalLoadingTimeMs);
            
            // Accumulate processing statistics (would be real data in actual implementation)
            benchmark.totalVerticesProcessed += 1000; // Placeholder
            benchmark.totalTrianglesProcessed += 2000; // Placeholder
            
            // Estimate file size
            try {
                if (std::filesystem::exists(filepath)) {
                    benchmark.totalBytesProcessed += std::filesystem::file_size(filepath);
                }
            } catch (const std::exception&) {
                // Ignore file size errors
            }
            
        } catch (const std::exception& e) {
            LogVerbose("Error profiling file " + filepath + ": " + std::string(e.what()), "BenchmarkModelLoading");
        }
    }
    
    if (!benchmark.profiles.empty()) {
        benchmark.averageLoadingTime = totalTime / benchmark.profiles.size();
    }
    
    LogVerbose("Benchmark completed: " + std::to_string(benchmark.profiles.size()) + " files processed, " +
              "avg time: " + FormatDuration(benchmark.averageLoadingTime), "BenchmarkModelLoading");
    
    return benchmark;
}

std::string ModelDebugger::GeneratePerformanceReport(const PerformanceProfile& profile) {
    std::stringstream ss;
    
    ss << "=== Performance Profile Report ===\n";
    ss << "File: " << profile.filepath << "\n";
    ss << "Total Loading Time: " << FormatDuration(profile.totalLoadingTimeMs) << "\n";
    ss << "\n";
    
    ss << "Stage Breakdown:\n";
    ss << "  File I/O: " << FormatDuration(profile.fileIOTimeMs) << " (" << 
          FormatPercentage(profile.fileIOTimeMs / profile.totalLoadingTimeMs * 100.0f) << ")\n";
    ss << "  Parsing: " << FormatDuration(profile.parsingTimeMs) << " (" << 
          FormatPercentage(profile.parsingTimeMs / profile.totalLoadingTimeMs * 100.0f) << ")\n";
    ss << "  Mesh Processing: " << FormatDuration(profile.meshProcessingTimeMs) << " (" << 
          FormatPercentage(profile.meshProcessingTimeMs / profile.totalLoadingTimeMs * 100.0f) << ")\n";
    ss << "  Material Processing: " << FormatDuration(profile.materialProcessingTimeMs) << " (" << 
          FormatPercentage(profile.materialProcessingTimeMs / profile.totalLoadingTimeMs * 100.0f) << ")\n";
    ss << "  Optimization: " << FormatDuration(profile.optimizationTimeMs) << " (" << 
          FormatPercentage(profile.optimizationTimeMs / profile.totalLoadingTimeMs * 100.0f) << ")\n";
    ss << "  Validation: " << FormatDuration(profile.validationTimeMs) << " (" << 
          FormatPercentage(profile.validationTimeMs / profile.totalLoadingTimeMs * 100.0f) << ")\n";
    ss << "\n";
    
    if (profile.peakMemoryUsage > 0) {
        ss << "Memory Usage:\n";
        ss << "  Initial: " << FormatMemorySize(profile.initialMemoryUsage) << "\n";
        ss << "  Peak: " << FormatMemorySize(profile.peakMemoryUsage) << "\n";
        ss << "  Final: " << FormatMemorySize(profile.finalMemoryUsage) << "\n";
        if (profile.memoryLeakBytes > 0) {
            ss << "  Potential Leak: " << FormatMemorySize(profile.memoryLeakBytes) << "\n";
        }
        ss << "\n";
    }
    
    if (profile.verticesPerSecond > 0) {
        ss << "Performance Metrics:\n";
        ss << "  Vertices/Second: " << std::fixed << std::setprecision(0) << profile.verticesPerSecond << "\n";
        ss << "  Triangles/Second: " << std::fixed << std::setprecision(0) << profile.trianglesPerSecond << "\n";
        ss << "  MB/Second: " << std::fixed << std::setprecision(2) << profile.mbPerSecond << "\n";
        ss << "\n";
    }
    
    if (!profile.optimizationSuggestions.empty()) {
        ss << "Optimization Suggestions:\n";
        for (const auto& suggestion : profile.optimizationSuggestions) {
            ss << "  - " << suggestion << "\n";
        }
        ss << "\n";
    }
    
    if (!profile.performanceIssues.empty()) {
        ss << "Performance Issues:\n";
        for (const auto& issue : profile.performanceIssues) {
            ss << "  - " << issue << "\n";
        }
    }
    
    return ss.str();
}

std::string ModelDebugger::GenerateBenchmarkReport(const LoadingBenchmark& benchmark) {
    std::stringstream ss;
    
    ss << "=== Loading Benchmark Report ===\n";
    ss << "Benchmark: " << benchmark.testName << "\n";
    ss << "Files Tested: " << benchmark.testFiles.size() << "\n";
    ss << "Successful Profiles: " << benchmark.profiles.size() << "\n";
    ss << "\n";
    
    ss << "Timing Summary:\n";
    ss << "  Average: " << FormatDuration(benchmark.averageLoadingTime) << "\n";
    ss << "  Minimum: " << FormatDuration(benchmark.minLoadingTime) << "\n";
    ss << "  Maximum: " << FormatDuration(benchmark.maxLoadingTime) << "\n";
    ss << "\n";
    
    ss << "Processing Summary:\n";
    ss << "  Total Vertices: " << benchmark.totalVerticesProcessed << "\n";
    ss << "  Total Triangles: " << benchmark.totalTrianglesProcessed << "\n";
    ss << "  Total Data: " << FormatMemorySize(benchmark.totalBytesProcessed) << "\n";
    ss << "\n";
    
    if (!benchmark.profiles.empty()) {
        ss << "Individual Results:\n";
        for (size_t i = 0; i < benchmark.profiles.size(); ++i) {
            const auto& profile = benchmark.profiles[i];
            ss << "  " << (i + 1) << ". " << std::filesystem::path(profile.filepath).filename().string() 
               << ": " << FormatDuration(profile.totalLoadingTimeMs) << "\n";
        }
    }
    
    return ss.str();
}

size_t ModelDebugger::GetCurrentMemoryUsage() {
    // Platform-specific memory usage detection
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
#endif
    return 0; // Fallback
}

void ModelDebugger::StartMemoryProfiling() {
    m_memoryProfiling = true;
    m_baselineMemory = GetCurrentMemoryUsage();
    m_memorySnapshots.clear();
    LogVerbose("Memory profiling started, baseline: " + FormatMemorySize(m_baselineMemory), "StartMemoryProfiling");
}

void ModelDebugger::StopMemoryProfiling() {
    m_memoryProfiling = false;
    LogVerbose("Memory profiling stopped", "StopMemoryProfiling");
}

std::vector<std::string> ModelDebugger::AnalyzeMemoryUsage(const DetailedModelStats& stats) {
    std::vector<std::string> analysis;
    
    // Analyze memory distribution
    float vertexMemoryPercent = (stats.vertexDataMemory * 100.0f) / stats.totalMemoryUsage;
    float indexMemoryPercent = (stats.indexDataMemory * 100.0f) / stats.totalMemoryUsage;
    float textureMemoryPercent = (stats.textureMemory * 100.0f) / stats.totalMemoryUsage;
    
    analysis.push_back("Memory Distribution Analysis:");
    analysis.push_back("  Vertex Data: " + FormatMemorySize(stats.vertexDataMemory) + " (" + 
                      FormatPercentage(vertexMemoryPercent) + ")");
    analysis.push_back("  Index Data: " + FormatMemorySize(stats.indexDataMemory) + " (" + 
                      FormatPercentage(indexMemoryPercent) + ")");
    analysis.push_back("  Texture Data: " + FormatMemorySize(stats.textureMemory) + " (" + 
                      FormatPercentage(textureMemoryPercent) + ")");
    
    // Memory efficiency analysis
    if (stats.totalVertices > 0) {
        size_t avgBytesPerVertex = stats.vertexDataMemory / stats.totalVertices;
        analysis.push_back("  Average bytes per vertex: " + std::to_string(avgBytesPerVertex));
        
        if (avgBytesPerVertex > 64) {
            analysis.push_back("  WARNING: High memory usage per vertex, consider vertex compression");
        }
    }
    
    return analysis;
}

std::vector<std::string> ModelDebugger::DetectMemoryLeaks(const PerformanceProfile& profile) {
    std::vector<std::string> leaks;
    
    if (profile.memoryLeakBytes > 1024) { // More than 1KB leak
        leaks.push_back("Potential memory leak detected: " + FormatMemorySize(profile.memoryLeakBytes));
        leaks.push_back("Memory increased from " + FormatMemorySize(profile.initialMemoryUsage) + 
                       " to " + FormatMemorySize(profile.finalMemoryUsage));
    }
    
    return leaks;
}

std::vector<std::string> ModelDebugger::GenerateLoadingOptimizations(const PerformanceProfile& profile) {
    std::vector<std::string> optimizations;
    
    // Analyze stage timings for optimization opportunities
    if (profile.fileIOTimeMs > profile.totalLoadingTimeMs * 0.3f) {
        optimizations.push_back("File I/O is taking " + FormatPercentage(profile.fileIOTimeMs / profile.totalLoadingTimeMs * 100.0f) + 
                               " of loading time - consider using memory-mapped files or async I/O");
    }
    
    if (profile.parsingTimeMs > profile.totalLoadingTimeMs * 0.4f) {
        optimizations.push_back("Parsing is taking " + FormatPercentage(profile.parsingTimeMs / profile.totalLoadingTimeMs * 100.0f) + 
                               " of loading time - consider optimizing parser or using binary formats");
    }
    
    if (profile.meshProcessingTimeMs > profile.totalLoadingTimeMs * 0.3f) {
        optimizations.push_back("Mesh processing is taking " + FormatPercentage(profile.meshProcessingTimeMs / profile.totalLoadingTimeMs * 100.0f) + 
                               " of loading time - consider pre-processing meshes or using simpler algorithms");
    }
    
    if (profile.memoryLeakBytes > 0) {
        optimizations.push_back("Memory leak detected - review resource cleanup and smart pointer usage");
    }
    
    if (profile.peakMemoryUsage > profile.finalMemoryUsage * 2) {
        optimizations.push_back("High peak memory usage detected - consider streaming or progressive loading");
    }
    
    return optimizations;
}

std::vector<std::string> ModelDebugger::GenerateMemoryOptimizations(const DetailedModelStats& stats) {
    std::vector<std::string> optimizations;
    
    // Check for memory optimization opportunities
    if (stats.duplicateVertices > 0) {
        optimizations.push_back("Remove " + std::to_string(stats.duplicateVertices) + 
                               " duplicate vertices to save memory");
    }
    
    if (stats.totalMemoryUsage > 100 * 1024 * 1024) { // > 100MB
        optimizations.push_back("Large memory usage (" + FormatMemorySize(stats.totalMemoryUsage) + 
                               ") - consider texture compression and mesh simplification");
    }
    
    if (stats.meshCount > 100) {
        optimizations.push_back("High mesh count (" + std::to_string(stats.meshCount) + 
                               ") - consider mesh batching to reduce memory overhead");
    }
    
    return optimizations;
}

std::vector<std::string> ModelDebugger::GenerateCacheOptimizations(const DetailedModelStats& stats) {
    std::vector<std::string> optimizations;
    
    if (stats.cacheEfficiency > 1.5f) {
        optimizations.push_back("Poor vertex cache efficiency (ACMR: " + std::to_string(stats.cacheEfficiency) + 
                               ") - optimize vertex ordering for better cache performance");
    }
    
    if (stats.totalTriangles > 50000 && stats.cacheEfficiency > 1.2f) {
        optimizations.push_back("High triangle count with suboptimal cache efficiency - consider using mesh optimization tools");
    }
    
    return optimizations;
}

bool ModelDebugger::SavePerformanceProfile(const PerformanceProfile& profile, const std::string& outputPath) {
    try {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            LogVerbose("Failed to open output file: " + outputPath, "SavePerformanceProfile");
            return false;
        }
        
        file << GeneratePerformanceReport(profile);
        file.close();
        
        LogVerbose("Performance profile saved to: " + outputPath, "SavePerformanceProfile");
        return true;
    } catch (const std::exception& e) {
        LogVerbose("Error saving performance profile: " + std::string(e.what()), "SavePerformanceProfile");
        return false;
    }
}

bool ModelDebugger::SaveBenchmarkResults(const LoadingBenchmark& benchmark, const std::string& outputPath) {
    try {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            LogVerbose("Failed to open output file: " + outputPath, "SaveBenchmarkResults");
            return false;
        }
        
        file << GenerateBenchmarkReport(benchmark);
        file.close();
        
        LogVerbose("Benchmark results saved to: " + outputPath, "SaveBenchmarkResults");
        return true;
    } catch (const std::exception& e) {
        LogVerbose("Error saving benchmark results: " + std::string(e.what()), "SaveBenchmarkResults");
        return false;
    }
}

bool ModelDebugger::SaveAnalysisToFile(const DetailedModelStats& stats, const std::string& outputPath) {
    try {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            LogVerbose("Failed to open output file: " + outputPath, "SaveAnalysisToFile");
            return false;
        }
        
        file << GenerateDetailedBreakdown(stats);
        file.close();
        
        LogVerbose("Analysis saved to: " + outputPath, "SaveAnalysisToFile");
        return true;
    } catch (const std::exception& e) {
        LogVerbose("Error saving analysis: " + std::string(e.what()), "SaveAnalysisToFile");
        return false;
    }
}

void ModelDebugger::SetPerformanceThresholds(uint32_t maxVertices, uint32_t maxTriangles, float maxMemoryMB) {
    m_maxVertices = maxVertices;
    m_maxTriangles = maxTriangles;
    m_maxMemoryMB = maxMemoryMB;
    
    LogVerbose("Performance thresholds updated: " + 
              std::to_string(maxVertices) + " vertices, " + 
              std::to_string(maxTriangles) + " triangles, " + 
              std::to_string(maxMemoryMB) + " MB", "SetPerformanceThresholds");
}

void ModelDebugger::SetQualityThresholds(float minTriangleArea, float maxCacheThreshold) {
    m_minTriangleArea = minTriangleArea;
    m_maxCacheThreshold = maxCacheThreshold;
    
    LogVerbose("Quality thresholds updated: min triangle area " + 
              std::to_string(minTriangleArea) + ", max cache threshold " + 
              std::to_string(maxCacheThreshold), "SetQualityThresholds");
}

void ModelDebugger::EnableDetailedMeshAnalysis(bool enabled) {
    m_detailedMeshAnalysis = enabled;
    LogVerbose("Detailed mesh analysis " + std::string(enabled ? "enabled" : "disabled"));
}

void ModelDebugger::EnableMemoryProfiling(bool enabled) {
    m_memoryProfiling = enabled;
    LogVerbose("Memory profiling " + std::string(enabled ? "enabled" : "disabled"));
}

// Private implementation methods

void ModelDebugger::AnalyzeHierarchy(std::shared_ptr<Model> model, DetailedModelStats& stats) {
    auto rootNode = model->GetRootNode();
    if (!rootNode) {
        LogVerbose("Model has no root node", "AnalyzeHierarchy");
        return;
    }
    
    auto allNodes = model->GetAllNodes();
    stats.nodeCount = static_cast<uint32_t>(allNodes.size());
    stats.maxDepth = CalculateHierarchyDepth(rootNode);
    
    CountNodeTypes(rootNode, stats.leafNodeCount, stats.emptyNodeCount);
    
    LogVerbose("Hierarchy analysis: " + std::to_string(stats.nodeCount) + " nodes, " + 
              "max depth " + std::to_string(stats.maxDepth), "AnalyzeHierarchy");
}

void ModelDebugger::AnalyzeMeshStatistics(std::shared_ptr<Model> model, DetailedModelStats& stats) {
    auto meshes = model->GetMeshes();
    stats.meshCount = static_cast<uint32_t>(meshes.size());
    
    if (meshes.empty()) {
        LogVerbose("Model has no meshes", "AnalyzeMeshStatistics");
        return;
    }
    
    std::vector<uint32_t> vertexCounts;
    std::vector<uint32_t> triangleCounts;
    
    for (const auto& mesh : meshes) {
        if (mesh) {
            uint32_t vertexCount = mesh->GetVertexCount();
            uint32_t triangleCount = mesh->GetTriangleCount();
            
            stats.totalVertices += vertexCount;
            stats.totalTriangles += triangleCount;
            
            vertexCounts.push_back(vertexCount);
            triangleCounts.push_back(triangleCount);
            
            stats.minVerticesPerMesh = std::min(stats.minVerticesPerMesh, vertexCount);
            stats.maxVerticesPerMesh = std::max(stats.maxVerticesPerMesh, vertexCount);
            stats.minTrianglesPerMesh = std::min(stats.minTrianglesPerMesh, triangleCount);
            stats.maxTrianglesPerMesh = std::max(stats.maxTrianglesPerMesh, triangleCount);
        }
    }
    
    if (!vertexCounts.empty()) {
        stats.avgVerticesPerMesh = static_cast<float>(stats.totalVertices) / vertexCounts.size();
        stats.avgTrianglesPerMesh = static_cast<float>(stats.totalTriangles) / triangleCounts.size();
    }
    
    LogVerbose("Mesh statistics: " + std::to_string(stats.meshCount) + " meshes, " + 
              std::to_string(stats.totalVertices) + " vertices, " + 
              std::to_string(stats.totalTriangles) + " triangles", "AnalyzeMeshStatistics");
}

void ModelDebugger::AnalyzeMaterialStatistics(std::shared_ptr<Model> model, DetailedModelStats& stats) {
    auto materials = model->GetMaterials();
    stats.materialCount = static_cast<uint32_t>(materials.size());
    
    // Count meshes without materials
    auto meshes = model->GetMeshes();
    for (const auto& mesh : meshes) {
        if (mesh && !mesh->GetMaterial()) {
            stats.meshesWithoutMaterials++;
        }
    }
    
    // Note: Texture counting would require Material class to expose texture information
    // This is a placeholder for when that functionality is available
    
    LogVerbose("Material statistics: " + std::to_string(stats.materialCount) + " materials, " + 
              std::to_string(stats.meshesWithoutMaterials) + " meshes without materials", "AnalyzeMaterialStatistics");
}

void ModelDebugger::AnalyzeAnimationStatistics(std::shared_ptr<Model> model, DetailedModelStats& stats) {
    stats.animationCount = static_cast<uint32_t>(model->GetAnimationCount());
    stats.skeletonCount = model->HasSkeleton() ? 1u : 0u;
    stats.skinCount = static_cast<uint32_t>(model->GetSkinCount());
    
    // Note: Bone counting and animation duration would require Animation/Skeleton classes
    // to expose this information. This is a placeholder for when that functionality is available.
    
    if (stats.animationCount > 0 || stats.skeletonCount > 0) {
        LogVerbose("Animation statistics: " + std::to_string(stats.animationCount) + " animations, " + 
                  std::to_string(stats.skeletonCount) + " skeletons, " + 
                  std::to_string(stats.skinCount) + " skins", "AnalyzeAnimationStatistics");
    }
}

void ModelDebugger::AnalyzeMemoryUsage(std::shared_ptr<Model> model, DetailedModelStats& stats) {
    if (!m_memoryProfiling) {
        return;
    }
    
    stats.totalMemoryUsage = model->GetMemoryUsage();
    
    // Break down memory usage by component
    auto meshes = model->GetMeshes();
    for (const auto& mesh : meshes) {
        if (mesh) {
            size_t meshMemory = mesh->GetMemoryUsage();
            // Rough estimation of vertex vs index data
            stats.vertexDataMemory += static_cast<size_t>(meshMemory * 0.8f); // Assume 80% vertex data
            stats.indexDataMemory += static_cast<size_t>(meshMemory * 0.2f);  // Assume 20% index data
        }
    }
    
    // Estimate node memory
    stats.nodeMemory = stats.nodeCount * sizeof(void*) * 10; // Rough estimate
    
    LogVerbose("Memory analysis: " + FormatMemorySize(stats.totalMemoryUsage) + " total", "AnalyzeMemoryUsage");
}

void ModelDebugger::AnalyzeGeometryQuality(std::shared_ptr<Model> model, DetailedModelStats& stats) {
    auto meshes = model->GetMeshes();
    if (meshes.empty()) {
        return;
    }
    
    std::vector<float> triangleAreas;
    uint32_t totalDegenerateTriangles = 0;
    uint32_t totalDuplicateVertices = 0;
    float totalCacheEfficiency = 0.0f;
    uint32_t validMeshes = 0;
    
    for (const auto& mesh : meshes) {
        if (mesh) {
            // Analyze triangle quality (placeholder - would need vertex data access)
            uint32_t triangleCount = mesh->GetTriangleCount();
            if (triangleCount > 0) {
                // These would be calculated from actual vertex data
                totalDegenerateTriangles += CountDuplicateVertices(mesh); // Placeholder
                totalDuplicateVertices += CountDuplicateVertices(mesh);
                totalCacheEfficiency += CalculateCacheEfficiency(mesh);
                validMeshes++;
            }
        }
    }
    
    stats.degenerateTriangles = totalDegenerateTriangles;
    stats.duplicateVertices = totalDuplicateVertices;
    
    if (validMeshes > 0) {
        stats.cacheEfficiency = totalCacheEfficiency / validMeshes;
    }
    
    LogVerbose("Geometry quality: " + std::to_string(stats.degenerateTriangles) + " degenerate triangles, " + 
              std::to_string(stats.duplicateVertices) + " duplicate vertices", "AnalyzeGeometryQuality");
}

void ModelDebugger::AnalyzeBoundingVolumes(std::shared_ptr<Model> model, DetailedModelStats& stats) {
    auto boundingBox = model->GetBoundingBox();
    auto boundingSphere = model->GetBoundingSphere();
    
    if (boundingBox.IsValid()) {
        stats.boundingBoxMin = boundingBox.min;
        stats.boundingBoxMax = boundingBox.max;
        stats.boundingBoxSize = boundingBox.GetSize();
    }
    
    if (boundingSphere.IsValid()) {
        stats.boundingSphereCenter = boundingSphere.center;
        stats.boundingSphereRadius = boundingSphere.radius;
    }
    
    LogVerbose("Bounding volumes analyzed", "AnalyzeBoundingVolumes");
}

void ModelDebugger::AnalyzePerformanceIndicators(std::shared_ptr<Model> model, DetailedModelStats& stats) {
    stats.hasLODLevels = model->GetLODCount() > 0;
    
    // Check if model appears to be optimized (heuristic)
    stats.isOptimized = (stats.duplicateVertices == 0) && 
                       (stats.degenerateTriangles == 0) && 
                       (stats.cacheEfficiency < 1.5f);
    
    // Check for valid vertex attributes (placeholder - would need vertex data access)
    stats.hasValidNormals = true;  // Assume valid for now
    stats.hasValidUVs = true;      // Assume valid for now
    stats.hasValidTangents = false; // Assume not present for now
    
    LogVerbose("Performance indicators analyzed", "AnalyzePerformanceIndicators");
}

void ModelDebugger::AnalyzeMeshGeometry(std::shared_ptr<Mesh> mesh, MeshAnalysis& analysis) {
    // Basic geometry information
    auto boundingBox = mesh->GetBoundingBox();
    if (boundingBox.IsValid()) {
        analysis.boundingBoxMin = boundingBox.min;
        analysis.boundingBoxMax = boundingBox.max;
        analysis.boundingBoxSize = boundingBox.GetSize();
    }
    
    // Vertex attributes (placeholder - would need vertex layout access)
    analysis.hasPositions = true;  // Always assume positions
    analysis.hasNormals = true;    // Placeholder
    analysis.hasTexCoords = true;  // Placeholder
    analysis.hasTangents = false;  // Placeholder
    analysis.hasColors = false;    // Placeholder
    analysis.hasBoneWeights = false; // Placeholder
}

void ModelDebugger::AnalyzeMeshQuality(std::shared_ptr<Mesh> mesh, MeshAnalysis& analysis) {
    // Quality metrics (placeholders - would need actual vertex data)
    analysis.degenerateTriangles = 0;
    analysis.duplicateVertices = CountDuplicateVertices(mesh);
    analysis.cacheEfficiency = CalculateCacheEfficiency(mesh);
    analysis.averageTriangleArea = 1.0f; // Placeholder
}

void ModelDebugger::AnalyzeMeshPerformance(std::shared_ptr<Mesh> mesh, MeshAnalysis& analysis) {
    // Performance heuristics
    analysis.isOptimized = (analysis.duplicateVertices == 0) && 
                          (analysis.degenerateTriangles == 0) && 
                          (analysis.cacheEfficiency < 1.5f);
    
    analysis.needsOptimization = !analysis.isOptimized || 
                                (analysis.vertexCount > 10000) || 
                                (analysis.triangleCount > 20000);
}

void ModelDebugger::DetectMeshIssues(std::shared_ptr<Mesh> mesh, MeshAnalysis& analysis) {
    // Detect common issues
    if (analysis.vertexCount == 0) {
        analysis.issues.push_back("Mesh has no vertices");
    }
    
    if (analysis.triangleCount == 0) {
        analysis.issues.push_back("Mesh has no triangles");
    }
    
    if (analysis.duplicateVertices > 0) {
        analysis.issues.push_back("Duplicate vertices found: " + std::to_string(analysis.duplicateVertices));
        analysis.suggestions.push_back("Remove duplicate vertices to optimize memory usage");
    }
    
    if (analysis.degenerateTriangles > 0) {
        analysis.issues.push_back("Degenerate triangles found: " + std::to_string(analysis.degenerateTriangles));
        analysis.suggestions.push_back("Remove degenerate triangles to improve rendering quality");
    }
    
    if (analysis.cacheEfficiency > 2.0f) {
        analysis.issues.push_back("Poor vertex cache efficiency: " + std::to_string(analysis.cacheEfficiency));
        analysis.suggestions.push_back("Optimize vertex ordering for better cache performance");
    }
    
    if (!analysis.hasMaterial) {
        analysis.issues.push_back("Mesh has no material assigned");
        analysis.suggestions.push_back("Assign a material for proper rendering");
    }
    
    if (analysis.vertexCount > 50000) {
        analysis.issues.push_back("High vertex count: " + std::to_string(analysis.vertexCount));
        analysis.suggestions.push_back("Consider mesh simplification or LOD generation");
    }
}

// Utility methods

void ModelDebugger::LogVerbose(const std::string& message, const std::string& component) {
    if (m_verboseLogging) {
        ModelDiagnosticLogger::GetInstance().LogDebug(message, component, "");
    }
}

std::string ModelDebugger::FormatMemorySize(size_t bytes) {
    if (bytes >= 1024 * 1024 * 1024) {
        return std::to_string(bytes / 1024 / 1024 / 1024) + " GB";
    } else if (bytes >= 1024 * 1024) {
        return std::to_string(bytes / 1024 / 1024) + " MB";
    } else if (bytes >= 1024) {
        return std::to_string(bytes / 1024) + " KB";
    } else {
        return std::to_string(bytes) + " bytes";
    }
}

std::string ModelDebugger::FormatDuration(float milliseconds) {
    if (milliseconds >= 1000.0f) {
        return std::to_string(milliseconds / 1000.0f) + "s";
    } else {
        return std::to_string(milliseconds) + "ms";
    }
}

std::string ModelDebugger::FormatPercentage(float value) {
    return std::to_string(static_cast<int>(value)) + "%";
}

float ModelDebugger::CalculateTriangleArea(const Math::Vec3& v1, const Math::Vec3& v2, const Math::Vec3& v3) {
    Math::Vec3 edge1 = v2 - v1;
    Math::Vec3 edge2 = v3 - v1;
    Math::Vec3 cross = glm::cross(edge1, edge2);
    return glm::length(cross) * 0.5f;
}

bool ModelDebugger::IsTriangleDegenerate(const Math::Vec3& v1, const Math::Vec3& v2, const Math::Vec3& v3, float epsilon) {
    return CalculateTriangleArea(v1, v2, v3) < epsilon;
}

uint32_t ModelDebugger::CountDuplicateVertices(std::shared_ptr<Mesh> mesh, float epsilon) {
    // Placeholder implementation - would need access to vertex data
    return 0;
}

float ModelDebugger::CalculateCacheEfficiency(std::shared_ptr<Mesh> mesh) {
    // Placeholder implementation - would need access to index data
    // Returns ACMR (Average Cache Miss Ratio)
    return 1.0f;
}

uint32_t ModelDebugger::CalculateHierarchyDepth(std::shared_ptr<ModelNode> node, uint32_t currentDepth) {
    if (!node) {
        return currentDepth;
    }
    
    uint32_t maxDepth = currentDepth;
    auto children = node->GetChildren();
    
    for (const auto& child : children) {
        uint32_t childDepth = CalculateHierarchyDepth(child, currentDepth + 1);
        maxDepth = std::max(maxDepth, childDepth);
    }
    
    return maxDepth;
}

void ModelDebugger::CountNodeTypes(std::shared_ptr<ModelNode> node, uint32_t& leafNodes, uint32_t& emptyNodes) {
    if (!node) {
        return;
    }
    
    auto children = node->GetChildren();
    
    if (children.empty()) {
        leafNodes++;
    }
    
    if (children.empty() && node->GetMeshIndices().empty()) {
        emptyNodes++;
    }
    
    for (const auto& child : children) {
        CountNodeTypes(child, leafNodes, emptyNodes);
    }
}

bool ModelDebugger::ExportToJSON(const DetailedModelStats& stats, const std::string& outputPath) {
    try {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            LogVerbose("Failed to open output file: " + outputPath, "ExportToJSON");
            return false;
        }
        
        // Simple JSON export (would use a proper JSON library in production)
        file << "{\n";
        file << "  \"filepath\": \"" << stats.filepath << "\",\n";
        file << "  \"format\": \"" << stats.format << "\",\n";
        file << "  \"name\": \"" << stats.name << "\",\n";
        file << "  \"loadingTimeMs\": " << stats.loadingTimeMs << ",\n";
        file << "  \"nodeCount\": " << stats.nodeCount << ",\n";
        file << "  \"meshCount\": " << stats.meshCount << ",\n";
        file << "  \"materialCount\": " << stats.materialCount << ",\n";
        file << "  \"totalVertices\": " << stats.totalVertices << ",\n";
        file << "  \"totalTriangles\": " << stats.totalTriangles << ",\n";
        file << "  \"totalMemoryUsage\": " << stats.totalMemoryUsage << ",\n";
        file << "  \"cacheEfficiency\": " << stats.cacheEfficiency << ",\n";
        file << "  \"degenerateTriangles\": " << stats.degenerateTriangles << ",\n";
        file << "  \"duplicateVertices\": " << stats.duplicateVertices << ",\n";
        file << "  \"validationIssues\": " << stats.validationIssues << "\n";
        file << "}\n";
        
        file.close();
        LogVerbose("JSON export saved to: " + outputPath, "ExportToJSON");
        return true;
    } catch (const std::exception& e) {
        LogVerbose("Error exporting to JSON: " + std::string(e.what()), "ExportToJSON");
        return false;
    }
}

bool ModelDebugger::ExportToCSV(const std::vector<DetailedModelStats>& statsCollection, const std::string& outputPath) {
    try {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            LogVerbose("Failed to open output file: " + outputPath, "ExportToCSV");
            return false;
        }
        
        // CSV header
        file << "Filepath,Format,Name,LoadingTimeMs,NodeCount,MeshCount,MaterialCount,TotalVertices,TotalTriangles,TotalMemoryUsage,CacheEfficiency,DegenerateTriangles,DuplicateVertices,ValidationIssues\n";
        
        // CSV data
        for (const auto& stats : statsCollection) {
            file << "\"" << stats.filepath << "\","
                 << "\"" << stats.format << "\","
                 << "\"" << stats.name << "\","
                 << stats.loadingTimeMs << ","
                 << stats.nodeCount << ","
                 << stats.meshCount << ","
                 << stats.materialCount << ","
                 << stats.totalVertices << ","
                 << stats.totalTriangles << ","
                 << stats.totalMemoryUsage << ","
                 << stats.cacheEfficiency << ","
                 << stats.degenerateTriangles << ","
                 << stats.duplicateVertices << ","
                 << stats.validationIssues << "\n";
        }
        
        file.close();
        LogVerbose("CSV export saved to: " + outputPath, "ExportToCSV");
        return true;
    } catch (const std::exception& e) {
        LogVerbose("Error exporting to CSV: " + std::string(e.what()), "ExportToCSV");
        return false;
    }
}} 
// namespace GameEngine