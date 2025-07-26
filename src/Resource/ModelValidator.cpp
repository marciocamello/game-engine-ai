#include "Resource/ModelValidator.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#undef max
#undef min
#endif

namespace GameEngine {

// ModelValidator Implementation

ModelValidator::ModelValidator() {
    // Enable all validation types by default
    m_enabledTypes[ValidationType::FileStructure] = true;
    m_enabledTypes[ValidationType::GeometryData] = true;
    m_enabledTypes[ValidationType::MaterialData] = true;
    m_enabledTypes[ValidationType::AnimationData] = true;
    m_enabledTypes[ValidationType::Performance] = true;
    m_enabledTypes[ValidationType::Compatibility] = true;
    m_enabledTypes[ValidationType::Standards] = true;
}

ModelValidator::~ModelValidator() = default;

ModelValidator::ValidationReport ModelValidator::ValidateFile(const std::string& filepath) {
    ValidationReport report;
    report.filepath = filepath;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Detect format
        std::filesystem::path path(filepath);
        std::string extension = path.extension().string();
        if (!extension.empty() && extension[0] == '.') {
            extension = extension.substr(1);
        }
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        report.format = extension;
        
        // Validate file structure
        if (m_enabledTypes[ValidationType::FileStructure]) {
            auto structureIssues = ValidateFileStructure(filepath);
            report.issues.insert(report.issues.end(), structureIssues.begin(), structureIssues.end());
        }
        
        // Try to load and validate the model
        try {
            // This would normally use ModelLoader, but for now we'll do basic validation
            if (std::filesystem::exists(filepath)) {
                auto fileSize = std::filesystem::file_size(filepath);
                report.memoryUsageBytes = fileSize;
                
                // Basic file size validation
                if (fileSize == 0) {
                    ValidationIssue issue;
                    issue.type = ValidationType::FileStructure;
                    issue.severity = ValidationSeverity::Critical;
                    issue.component = "File";
                    issue.description = "File is empty";
                    issue.suggestion = "Ensure the file was exported correctly";
                    report.issues.push_back(issue);
                    report.isValid = false;
                }
                
                // Check for very large files
                const size_t maxReasonableSize = 500 * 1024 * 1024; // 500MB
                if (fileSize > maxReasonableSize) {
                    ValidationIssue issue;
                    issue.type = ValidationType::Performance;
                    issue.severity = ValidationSeverity::Warning;
                    issue.component = "File";
                    issue.description = "File is very large (" + std::to_string(fileSize / 1024 / 1024) + " MB)";
                    issue.suggestion = "Consider optimizing the model or using LOD levels";
                    report.issues.push_back(issue);
                }
            }
        } catch (const std::exception& e) {
            ValidationIssue issue;
            issue.type = ValidationType::FileStructure;
            issue.severity = ValidationSeverity::Error;
            issue.component = "File Loading";
            issue.description = "Failed to load file: " + std::string(e.what());
            issue.suggestion = "Check file format and integrity";
            report.issues.push_back(issue);
            report.isValid = false;
        }
        
    } catch (const std::exception& e) {
        ValidationIssue issue;
        issue.type = ValidationType::FileStructure;
        issue.severity = ValidationSeverity::Critical;
        issue.component = "Validation";
        issue.description = "Validation failed: " + std::string(e.what());
        issue.suggestion = "Check file accessibility and format";
        report.issues.push_back(issue);
        report.isValid = false;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    report.validationTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    UpdateReportCounts(report);
    
    return report;
}

ModelValidator::ValidationReport ModelValidator::ValidateModel(std::shared_ptr<Model> model) {
    ValidationReport report;
    
    if (!model) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Critical;
        issue.component = "Model";
        issue.description = "Model is null";
        issue.suggestion = "Ensure model was loaded successfully";
        report.issues.push_back(issue);
        report.isValid = false;
        UpdateReportCounts(report);
        return report;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        report.filepath = model->GetPath();
        
        // Get model statistics
        auto meshes = model->GetMeshes();
        report.totalMeshes = meshes.size();
        
        for (const auto& mesh : meshes) {
            if (mesh) {
                report.totalVertices += mesh->GetVertexCount();
                report.totalTriangles += mesh->GetTriangleCount();
                report.memoryUsageBytes += mesh->GetMemoryUsage();
            }
        }
        
        // Validate geometry data
        if (m_enabledTypes[ValidationType::GeometryData]) {
            for (size_t i = 0; i < meshes.size(); ++i) {
                if (meshes[i]) {
                    auto geometryIssues = ValidateGeometry(meshes[i], "Mesh_" + std::to_string(i));
                    report.issues.insert(report.issues.end(), geometryIssues.begin(), geometryIssues.end());
                }
            }
        }
        
        // Validate materials
        if (m_enabledTypes[ValidationType::MaterialData]) {
            auto materialIssues = ValidateMaterials(model);
            report.issues.insert(report.issues.end(), materialIssues.begin(), materialIssues.end());
        }
        
        // Validate animations
        if (m_enabledTypes[ValidationType::AnimationData]) {
            auto animationIssues = ValidateAnimations(model);
            report.issues.insert(report.issues.end(), animationIssues.begin(), animationIssues.end());
        }
        
        // Validate performance
        if (m_enabledTypes[ValidationType::Performance]) {
            auto performanceIssues = ValidatePerformance(model);
            report.issues.insert(report.issues.end(), performanceIssues.begin(), performanceIssues.end());
        }
        
    } catch (const std::exception& e) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Critical;
        issue.component = "Model Validation";
        issue.description = "Model validation failed: " + std::string(e.what());
        issue.suggestion = "Check model data integrity";
        report.issues.push_back(issue);
        report.isValid = false;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    report.validationTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    UpdateReportCounts(report);
    
    return report;
}

ModelValidator::ValidationReport ModelValidator::ValidateMesh(std::shared_ptr<Mesh> mesh, const std::string& meshName) {
    ValidationReport report;
    
    if (!mesh) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Critical;
        issue.component = "Mesh";
        issue.description = "Mesh is null";
        issue.suggestion = "Ensure mesh was created successfully";
        report.issues.push_back(issue);
        report.isValid = false;
        UpdateReportCounts(report);
        return report;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Get mesh statistics
        report.totalMeshes = 1;
        report.totalVertices = mesh->GetVertexCount();
        report.totalTriangles = mesh->GetTriangleCount();
        report.memoryUsageBytes = mesh->GetMemoryUsage();
        
        // Validate geometry
        if (m_enabledTypes[ValidationType::GeometryData]) {
            auto geometryIssues = ValidateGeometry(mesh, meshName);
            report.issues.insert(report.issues.end(), geometryIssues.begin(), geometryIssues.end());
        }
        
    } catch (const std::exception& e) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Critical;
        issue.component = "Mesh Validation";
        issue.description = "Mesh validation failed: " + std::string(e.what());
        issue.suggestion = "Check mesh data integrity";
        report.issues.push_back(issue);
        report.isValid = false;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    report.validationTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    UpdateReportCounts(report);
    
    return report;
}

std::vector<ModelValidator::ValidationIssue> ModelValidator::ValidateFileStructure(const std::string& filepath) {
    std::vector<ValidationIssue> issues;
    
    try {
        // Check file existence
        if (!std::filesystem::exists(filepath)) {
            ValidationIssue issue;
            issue.type = ValidationType::FileStructure;
            issue.severity = ValidationSeverity::Critical;
            issue.component = "File";
            issue.description = "File does not exist: " + filepath;
            issue.suggestion = "Check file path and ensure file exists";
            issues.push_back(issue);
            return issues;
        }
        
        // Check file permissions
        try {
            std::ifstream file(filepath, std::ios::binary);
            if (!file.is_open()) {
                ValidationIssue issue;
                issue.type = ValidationType::FileStructure;
                issue.severity = ValidationSeverity::Error;
                issue.component = "File";
                issue.description = "Cannot open file for reading";
                issue.suggestion = "Check file permissions";
                issues.push_back(issue);
                return issues;
            }
            file.close();
        } catch (const std::exception& e) {
            ValidationIssue issue;
            issue.type = ValidationType::FileStructure;
            issue.severity = ValidationSeverity::Error;
            issue.component = "File";
            issue.description = "File access error: " + std::string(e.what());
            issue.suggestion = "Check file permissions and disk space";
            issues.push_back(issue);
        }
        
        // Format-specific validation
        std::filesystem::path path(filepath);
        std::string extension = path.extension().string();
        if (!extension.empty() && extension[0] == '.') {
            extension = extension.substr(1);
        }
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        if (extension == "obj") {
            auto objIssues = ValidateOBJFile(filepath);
            issues.insert(issues.end(), objIssues.begin(), objIssues.end());
        } else if (extension == "fbx") {
            auto fbxIssues = ValidateFBXFile(filepath);
            issues.insert(issues.end(), fbxIssues.begin(), fbxIssues.end());
        } else if (extension == "gltf" || extension == "glb") {
            auto gltfIssues = ValidateGLTFFile(filepath);
            issues.insert(issues.end(), gltfIssues.begin(), gltfIssues.end());
        }
        
    } catch (const std::exception& e) {
        ValidationIssue issue;
        issue.type = ValidationType::FileStructure;
        issue.severity = ValidationSeverity::Error;
        issue.component = "File Structure Validation";
        issue.description = "Validation error: " + std::string(e.what());
        issue.suggestion = "Check file format and integrity";
        issues.push_back(issue);
    }
    
    return issues;
}

std::vector<ModelValidator::ValidationIssue> ModelValidator::ValidateGeometry(std::shared_ptr<Mesh> mesh, const std::string& meshName) {
    std::vector<ValidationIssue> issues;
    
    if (!mesh) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Critical;
        issue.component = meshName;
        issue.description = "Mesh is null";
        issue.suggestion = "Ensure mesh was created successfully";
        issues.push_back(issue);
        return issues;
    }
    
    try {
        // Validate vertex data
        ValidateVertexData(mesh, issues, meshName);
        
        // Validate index data
        ValidateIndexData(mesh, issues, meshName);
        
        // Validate triangle quality
        ValidateTriangleQuality(mesh, issues, meshName);
        
        // Validate normals
        ValidateNormals(mesh, issues, meshName);
        
        // Validate texture coordinates
        ValidateTextureCoordinates(mesh, issues, meshName);
        
    } catch (const std::exception& e) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Error;
        issue.component = meshName;
        issue.description = "Geometry validation error: " + std::string(e.what());
        issue.suggestion = "Check mesh data integrity";
        issues.push_back(issue);
    }
    
    return issues;
}

std::vector<ModelValidator::ValidationIssue> ModelValidator::ValidateMaterials(std::shared_ptr<Model> model) {
    std::vector<ValidationIssue> issues;
    
    if (!model) {
        return issues;
    }
    
    try {
        auto materials = model->GetMaterials();
        
        if (materials.empty()) {
            ValidationIssue issue;
            issue.type = ValidationType::MaterialData;
            issue.severity = ValidationSeverity::Warning;
            issue.component = "Materials";
            issue.description = "Model has no materials";
            issue.suggestion = "Consider adding materials for better visual quality";
            issues.push_back(issue);
        }
        
        // Validate each material
        for (size_t i = 0; i < materials.size(); ++i) {
            if (!materials[i]) {
                ValidationIssue issue;
                issue.type = ValidationType::MaterialData;
                issue.severity = ValidationSeverity::Error;
                issue.component = "Material_" + std::to_string(i);
                issue.description = "Material is null";
                issue.suggestion = "Ensure material was loaded correctly";
                issues.push_back(issue);
            }
        }
        
    } catch (const std::exception& e) {
        ValidationIssue issue;
        issue.type = ValidationType::MaterialData;
        issue.severity = ValidationSeverity::Error;
        issue.component = "Material Validation";
        issue.description = "Material validation error: " + std::string(e.what());
        issue.suggestion = "Check material data integrity";
        issues.push_back(issue);
    }
    
    return issues;
}

std::vector<ModelValidator::ValidationIssue> ModelValidator::ValidateAnimations(std::shared_ptr<Model> model) {
    std::vector<ValidationIssue> issues;
    
    if (!model) {
        return issues;
    }
    
    try {
        if (model->HasAnimations()) {
            auto animations = model->GetAnimations();
            
            for (size_t i = 0; i < animations.size(); ++i) {
                if (!animations[i]) {
                    ValidationIssue issue;
                    issue.type = ValidationType::AnimationData;
                    issue.severity = ValidationSeverity::Error;
                    issue.component = "Animation_" + std::to_string(i);
                    issue.description = "Animation is null";
                    issue.suggestion = "Ensure animation was loaded correctly";
                    issues.push_back(issue);
                }
            }
        }
        
    } catch (const std::exception& e) {
        ValidationIssue issue;
        issue.type = ValidationType::AnimationData;
        issue.severity = ValidationSeverity::Error;
        issue.component = "Animation Validation";
        issue.description = "Animation validation error: " + std::string(e.what());
        issue.suggestion = "Check animation data integrity";
        issues.push_back(issue);
    }
    
    return issues;
}

std::vector<ModelValidator::ValidationIssue> ModelValidator::ValidatePerformance(std::shared_ptr<Model> model) {
    std::vector<ValidationIssue> issues;
    
    if (!model) {
        return issues;
    }
    
    try {
        auto meshes = model->GetMeshes();
        size_t totalVertices = 0;
        size_t totalTriangles = 0;
        
        for (const auto& mesh : meshes) {
            if (mesh) {
                totalVertices += mesh->GetVertexCount();
                totalTriangles += mesh->GetTriangleCount();
            }
        }
        
        // Check vertex count
        if (totalVertices > m_maxVertices) {
            ValidationIssue issue;
            issue.type = ValidationType::Performance;
            issue.severity = ValidationSeverity::Warning;
            issue.component = "Performance";
            issue.description = "High vertex count: " + std::to_string(totalVertices) + " (threshold: " + std::to_string(m_maxVertices) + ")";
            issue.suggestion = "Consider using LOD levels or mesh optimization";
            issues.push_back(issue);
        }
        
        // Check triangle count
        if (totalTriangles > m_maxTriangles) {
            ValidationIssue issue;
            issue.type = ValidationType::Performance;
            issue.severity = ValidationSeverity::Warning;
            issue.component = "Performance";
            issue.description = "High triangle count: " + std::to_string(totalTriangles) + " (threshold: " + std::to_string(m_maxTriangles) + ")";
            issue.suggestion = "Consider mesh simplification or LOD levels";
            issues.push_back(issue);
        }
        
        // Check memory usage
        size_t totalMemory = 0;
        for (const auto& mesh : meshes) {
            if (mesh) {
                totalMemory += mesh->GetMemoryUsage();
            }
        }
        
        float memoryMB = totalMemory / 1024.0f / 1024.0f;
        if (memoryMB > m_maxMemoryMB) {
            ValidationIssue issue;
            issue.type = ValidationType::Performance;
            issue.severity = ValidationSeverity::Warning;
            issue.component = "Performance";
            issue.description = "High memory usage: " + std::to_string(memoryMB) + " MB (threshold: " + std::to_string(m_maxMemoryMB) + " MB)";
            issue.suggestion = "Consider texture compression or mesh optimization";
            issues.push_back(issue);
        }
        
    } catch (const std::exception& e) {
        ValidationIssue issue;
        issue.type = ValidationType::Performance;
        issue.severity = ValidationSeverity::Error;
        issue.component = "Performance Validation";
        issue.description = "Performance validation error: " + std::string(e.what());
        issue.suggestion = "Check model data integrity";
        issues.push_back(issue);
    }
    
    return issues;
}

ModelValidator::DiagnosticInfo ModelValidator::GenerateDiagnosticInfo(const std::string& filepath, const std::string& errorMessage) {
    DiagnosticInfo info;
    info.filepath = filepath;
    info.timestamp = std::chrono::system_clock::now();
    info.errorMessage = errorMessage;
    
    try {
        // File information
        if (std::filesystem::exists(filepath)) {
            info.fileSize = std::filesystem::file_size(filepath);
            // Convert file_time to system_clock time_point (simplified)
            auto ftime = std::filesystem::last_write_time(filepath);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            info.lastModified = sctp;
            info.fileHash = CalculateFileHash(filepath);
            
            // Detect format
            std::filesystem::path path(filepath);
            std::string extension = path.extension().string();
            if (!extension.empty() && extension[0] == '.') {
                extension = extension.substr(1);
            }
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            info.format = extension;
        }
        
        // System information
        info.platform = "Windows";
        info.engineVersion = "Kiro v1.1";
        
#ifdef _WIN32
        // Get available memory on Windows
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        if (GlobalMemoryStatusEx(&memStatus)) {
            info.availableMemory = memStatus.ullAvailPhys;
        }
#endif
        
        // Environment information
        info.workingDirectory = std::filesystem::current_path().string();
        
        // Add common search paths
        info.searchPaths.push_back("assets/");
        info.searchPaths.push_back("assets/meshes/");
        info.searchPaths.push_back("assets/models/");
        
    } catch (const std::exception& e) {
        // If we can't get diagnostic info, at least record the error
        info.errorMessage += " (Diagnostic error: " + std::string(e.what()) + ")";
    }
    
    return info;
}

std::string ModelValidator::GenerateDiagnosticReport(const DiagnosticInfo& info) {
    std::stringstream ss;
    
    ss << "=== Model Loading Diagnostic Report ===\n";
    auto timeT = std::chrono::system_clock::to_time_t(info.timestamp);
    ss << "Timestamp: " << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S") << "\n";
    ss << "File: " << info.filepath << "\n";
    ss << "Format: " << info.format << "\n";
    ss << "\n";
    
    ss << "File Information:\n";
    ss << "  Size: " << info.fileSize << " bytes";
    if (info.fileSize > 1024 * 1024) {
        ss << " (" << (info.fileSize / 1024.0 / 1024.0) << " MB)";
    } else if (info.fileSize > 1024) {
        ss << " (" << (info.fileSize / 1024.0) << " KB)";
    }
    ss << "\n";
    
    if (!info.fileHash.empty()) {
        ss << "  Hash: " << info.fileHash << "\n";
    }
    
    ss << "\n";
    
    ss << "System Information:\n";
    ss << "  Platform: " << info.platform << "\n";
    ss << "  Engine Version: " << info.engineVersion << "\n";
    if (info.availableMemory > 0) {
        ss << "  Available Memory: " << (info.availableMemory / 1024 / 1024) << " MB\n";
    }
    ss << "  Working Directory: " << info.workingDirectory << "\n";
    ss << "\n";
    
    if (!info.searchPaths.empty()) {
        ss << "Search Paths:\n";
        for (const auto& path : info.searchPaths) {
            ss << "  - " << path << "\n";
        }
        ss << "\n";
    }
    
    if (info.loadingTime.count() > 0) {
        ss << "Loading Time: " << info.loadingTime.count() << "ms\n";
    }
    
    if (!info.loadingFlags.empty()) {
        ss << "Loading Flags: " << info.loadingFlags << "\n";
    }
    
    if (!info.errorMessage.empty()) {
        ss << "\nError Message:\n";
        ss << info.errorMessage << "\n";
    }
    
    if (!info.stackTrace.empty()) {
        ss << "\nStack Trace:\n";
        ss << info.stackTrace << "\n";
    }
    
    return ss.str();
}

void ModelValidator::LogDiagnosticInfo(const DiagnosticInfo& info) {
    std::string report = GenerateDiagnosticReport(info);
    Logger::GetInstance().Info("Model Diagnostic Report:\n" + report);
}

std::string ModelValidator::GenerateValidationReport(const ValidationReport& report) {
    std::stringstream ss;
    
    ss << "=== Model Validation Report ===\n";
    ss << "File: " << report.filepath << "\n";
    ss << "Format: " << report.format << "\n";
    ss << "Valid: " << (report.isValid ? "Yes" : "No") << "\n";
    ss << "Validation Time: " << report.validationTime.count() << "ms\n";
    ss << "\n";
    
    ss << FormatStatistics(report);
    ss << "\n";
    
    if (!report.issues.empty()) {
        ss << "Issues Found (" << report.issues.size() << "):\n";
        for (const auto& issue : report.issues) {
            if (static_cast<int>(issue.severity) >= static_cast<int>(m_minSeverity)) {
                ss << FormatIssue(issue) << "\n";
            }
        }
    } else {
        ss << "No issues found.\n";
    }
    
    return ss.str();
}

std::string ModelValidator::GenerateDetailedReport(const ValidationReport& report) {
    std::stringstream ss;
    
    ss << GenerateValidationReport(report);
    ss << "\n";
    
    ss << "=== Detailed Analysis ===\n";
    
    // Group issues by type
    std::unordered_map<ValidationType, std::vector<ValidationIssue>> issuesByType;
    for (const auto& issue : report.issues) {
        issuesByType[issue.type].push_back(issue);
    }
    
    for (const auto& pair : issuesByType) {
        ss << "\n" << GetValidationTypeString(pair.first) << " Issues (" << pair.second.size() << "):\n";
        for (const auto& issue : pair.second) {
            ss << FormatIssue(issue, true) << "\n";
        }
    }
    
    return ss.str();
}

void ModelValidator::LogValidationReport(const ValidationReport& report) {
    std::string reportStr = GenerateValidationReport(report);
    
    if (report.isValid) {
        Logger::GetInstance().Info("Model validation passed:\n" + reportStr);
    } else {
        Logger::GetInstance().Error("Model validation failed:\n" + reportStr);
    }
}

bool ModelValidator::SaveReportToFile(const ValidationReport& report, const std::string& outputPath) {
    try {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            return false;
        }
        
        file << GenerateDetailedReport(report);
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to save validation report: " + std::string(e.what()));
        return false;
    }
}

void ModelValidator::SetValidationLevel(ValidationSeverity minSeverity) {
    m_minSeverity = minSeverity;
}

void ModelValidator::EnableValidationType(ValidationType type, bool enabled) {
    m_enabledTypes[type] = enabled;
}

void ModelValidator::SetPerformanceThresholds(size_t maxVertices, size_t maxTriangles, float maxMemoryMB) {
    m_maxVertices = maxVertices;
    m_maxTriangles = maxTriangles;
    m_maxMemoryMB = maxMemoryMB;
}

std::string ModelValidator::GetValidationTypeString(ValidationType type) {
    switch (type) {
        case ValidationType::FileStructure: return "File Structure";
        case ValidationType::GeometryData: return "Geometry Data";
        case ValidationType::MaterialData: return "Material Data";
        case ValidationType::AnimationData: return "Animation Data";
        case ValidationType::Performance: return "Performance";
        case ValidationType::Compatibility: return "Compatibility";
        case ValidationType::Standards: return "Standards";
        default: return "Unknown";
    }
}

std::string ModelValidator::GetValidationSeverityString(ValidationSeverity severity) {
    switch (severity) {
        case ValidationSeverity::Info: return "Info";
        case ValidationSeverity::Warning: return "Warning";
        case ValidationSeverity::Error: return "Error";
        case ValidationSeverity::Critical: return "Critical";
        default: return "Unknown";
    }
}

ModelValidator::ValidationSeverity ModelValidator::GetSeverityFromString(const std::string& severityStr) {
    std::string lower = severityStr;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "info") return ValidationSeverity::Info;
    if (lower == "warning") return ValidationSeverity::Warning;
    if (lower == "error") return ValidationSeverity::Error;
    if (lower == "critical") return ValidationSeverity::Critical;
    
    return ValidationSeverity::Info;
}

// Private implementation methods would continue here...
// For brevity, I'll implement key methods

void ModelValidator::ValidateVertexData(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName) {
    if (!mesh) return;
    
    uint32_t vertexCount = mesh->GetVertexCount();
    
    if (vertexCount == 0) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Error;
        issue.component = meshName;
        issue.description = "Mesh has no vertices";
        issue.suggestion = "Ensure mesh data was loaded correctly";
        issues.push_back(issue);
        return;
    }
    
    // Check for reasonable vertex count
    if (vertexCount > 100000) {
        ValidationIssue issue;
        issue.type = ValidationType::Performance;
        issue.severity = ValidationSeverity::Warning;
        issue.component = meshName;
        issue.description = "High vertex count: " + std::to_string(vertexCount);
        issue.suggestion = "Consider mesh optimization or LOD levels";
        issues.push_back(issue);
    }
}

void ModelValidator::ValidateIndexData(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName) {
    if (!mesh) return;
    
    const auto& indices = mesh->GetIndices();
    uint32_t vertexCount = mesh->GetVertexCount();
    
    if (indices.empty()) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Warning;
        issue.component = meshName;
        issue.description = "Mesh has no indices (non-indexed rendering)";
        issue.suggestion = "Consider using indexed rendering for better performance";
        issues.push_back(issue);
        return;
    }
    
    // Check for out-of-bounds indices
    for (size_t i = 0; i < indices.size(); ++i) {
        if (indices[i] >= vertexCount) {
            ValidationIssue issue;
            issue.type = ValidationType::GeometryData;
            issue.severity = ValidationSeverity::Error;
            issue.component = meshName;
            issue.description = "Index out of bounds at position " + std::to_string(i) + ": " + std::to_string(indices[i]) + " >= " + std::to_string(vertexCount);
            issue.suggestion = "Check mesh loading and indexing logic";
            issues.push_back(issue);
            break; // Don't spam with too many similar errors
        }
    }
}

std::string ModelValidator::CalculateFileHash(const std::string& filepath) {
    // Simple hash implementation - in production you'd want a proper hash function
    try {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        // Simple hash calculation
        std::hash<std::string> hasher;
        size_t hash = hasher(content);
        
        std::stringstream ss;
        ss << std::hex << hash;
        return ss.str();
    } catch (const std::exception&) {
        return "";
    }
}

std::string ModelValidator::FormatIssue(const ValidationIssue& issue, bool detailed) {
    std::stringstream ss;
    
    ss << "[" << GetValidationSeverityString(issue.severity) << "] ";
    
    if (!issue.component.empty()) {
        ss << issue.component << ": ";
    }
    
    ss << issue.description;
    
    if (detailed) {
        if (!issue.suggestion.empty()) {
            ss << "\n    Suggestion: " << issue.suggestion;
        }
        
        if (!issue.location.empty()) {
            ss << "\n    Location: " << issue.location;
        }
        
        if (issue.lineNumber > 0) {
            ss << "\n    Line: " << issue.lineNumber;
        }
        
        if (issue.byteOffset > 0) {
            ss << "\n    Offset: " << issue.byteOffset;
        }
    }
    
    return ss.str();
}

std::string ModelValidator::FormatStatistics(const ValidationReport& report) {
    std::stringstream ss;
    
    ss << "Statistics:\n";
    ss << "  Meshes: " << report.totalMeshes << "\n";
    ss << "  Vertices: " << report.totalVertices << "\n";
    ss << "  Triangles: " << report.totalTriangles << "\n";
    ss << "  Materials: " << report.totalMaterials << "\n";
    ss << "  Textures: " << report.totalTextures << "\n";
    ss << "  Animations: " << report.totalAnimations << "\n";
    ss << "  Memory Usage: " << (report.memoryUsageBytes / 1024.0 / 1024.0) << " MB\n";
    
    ss << "\nIssue Summary:\n";
    ss << "  Info: " << report.infoCount << "\n";
    ss << "  Warnings: " << report.warningCount << "\n";
    ss << "  Errors: " << report.errorCount << "\n";
    ss << "  Critical: " << report.criticalCount << "\n";
    
    return ss.str();
}

void ModelValidator::UpdateReportCounts(ValidationReport& report) {
    report.infoCount = 0;
    report.warningCount = 0;
    report.errorCount = 0;
    report.criticalCount = 0;
    
    for (const auto& issue : report.issues) {
        switch (issue.severity) {
            case ValidationSeverity::Info: report.infoCount++; break;
            case ValidationSeverity::Warning: report.warningCount++; break;
            case ValidationSeverity::Error: report.errorCount++; break;
            case ValidationSeverity::Critical: report.criticalCount++; break;
        }
    }
    
    // Report is invalid if there are any critical errors
    if (report.criticalCount > 0) {
        report.isValid = false;
    }
}

void ModelValidator::ValidateTriangleQuality(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName) {
    if (!mesh) return;
    
    const auto& vertices = mesh->GetVertices();
    const auto& indices = mesh->GetIndices();
    
    if (indices.size() % 3 != 0) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Error;
        issue.component = meshName;
        issue.description = "Index count is not divisible by 3";
        issue.suggestion = "Ensure mesh is properly triangulated";
        issues.push_back(issue);
        return;
    }
    
    size_t degenerateCount = 0;
    float totalArea = 0.0f;
    float minArea = std::numeric_limits<float>::max();
    float maxArea = 0.0f;
    
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (indices[i] >= vertices.size() || indices[i+1] >= vertices.size() || indices[i+2] >= vertices.size()) {
            continue; // Skip invalid triangles
        }
        
        const auto& v1 = vertices[indices[i]].position;
        const auto& v2 = vertices[indices[i+1]].position;
        const auto& v3 = vertices[indices[i+2]].position;
        
        float area = CalculateTriangleArea(v1, v2, v3);
        
        if (IsTriangleDegenerate(v1, v2, v3)) {
            degenerateCount++;
        }
        
        totalArea += area;
        minArea = std::min(minArea, area);
        maxArea = std::max(maxArea, area);
    }
    
    if (degenerateCount > 0) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Warning;
        issue.component = meshName;
        issue.description = "Found " + std::to_string(degenerateCount) + " degenerate triangles";
        issue.suggestion = "Remove or fix degenerate triangles";
        issues.push_back(issue);
    }
}

void ModelValidator::ValidateNormals(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName) {
    if (!mesh) return;
    
    const auto& vertices = mesh->GetVertices();
    
    bool hasNormals = false;
    size_t invalidNormals = 0;
    
    for (const auto& vertex : vertices) {
        if (vertex.normal.x != 0.0f || vertex.normal.y != 0.0f || vertex.normal.z != 0.0f) {
            hasNormals = true;
            
            // Check if normal is normalized
            float length = std::sqrt(vertex.normal.x * vertex.normal.x + 
                                   vertex.normal.y * vertex.normal.y + 
                                   vertex.normal.z * vertex.normal.z);
            
            if (std::abs(length - 1.0f) > 0.01f) {
                invalidNormals++;
            }
        }
    }
    
    if (!hasNormals) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Warning;
        issue.component = meshName;
        issue.description = "Mesh has no normals";
        issue.suggestion = "Generate normals for proper lighting";
        issues.push_back(issue);
    } else if (invalidNormals > 0) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Warning;
        issue.component = meshName;
        issue.description = "Found " + std::to_string(invalidNormals) + " non-normalized normals";
        issue.suggestion = "Normalize vertex normals";
        issues.push_back(issue);
    }
}

void ModelValidator::ValidateTextureCoordinates(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName) {
    if (!mesh) return;
    
    const auto& vertices = mesh->GetVertices();
    
    bool hasTexCoords = false;
    size_t outOfRangeCoords = 0;
    
    for (const auto& vertex : vertices) {
        if (vertex.texCoords.x != 0.0f || vertex.texCoords.y != 0.0f) {
            hasTexCoords = true;
            
            // Check if texture coordinates are in reasonable range
            if (vertex.texCoords.x < -1.0f || vertex.texCoords.x > 2.0f ||
                vertex.texCoords.y < -1.0f || vertex.texCoords.y > 2.0f) {
                outOfRangeCoords++;
            }
        }
    }
    
    if (!hasTexCoords) {
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Info;
        issue.component = meshName;
        issue.description = "Mesh has no texture coordinates";
        issue.suggestion = "Add texture coordinates if texturing is needed";
        issues.push_back(issue);
    } else if (outOfRangeCoords > vertices.size() / 10) { // More than 10% out of range
        ValidationIssue issue;
        issue.type = ValidationType::GeometryData;
        issue.severity = ValidationSeverity::Warning;
        issue.component = meshName;
        issue.description = "Many texture coordinates are out of normal range [0,1]";
        issue.suggestion = "Check texture coordinate generation";
        issues.push_back(issue);
    }
}

float ModelValidator::CalculateTriangleArea(const Math::Vec3& v1, const Math::Vec3& v2, const Math::Vec3& v3) {
    Math::Vec3 edge1 = v2 - v1;
    Math::Vec3 edge2 = v3 - v1;
    Math::Vec3 cross = glm::cross(edge1, edge2);
    return glm::length(cross) * 0.5f;
}

bool ModelValidator::IsTriangleDegenerate(const Math::Vec3& v1, const Math::Vec3& v2, const Math::Vec3& v3, float epsilon) {
    return CalculateTriangleArea(v1, v2, v3) < epsilon;
}

size_t ModelValidator::CountDuplicateVertices(std::shared_ptr<Mesh> mesh, float epsilon) {
    if (!mesh) return 0;
    
    const auto& vertices = mesh->GetVertices();
    size_t duplicates = 0;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        for (size_t j = i + 1; j < vertices.size(); ++j) {
            const auto& v1 = vertices[i].position;
            const auto& v2 = vertices[j].position;
            
            if (glm::length(v1 - v2) < epsilon) {
                duplicates++;
                break; // Count each vertex only once
            }
        }
    }
    
    return duplicates;
}

float ModelValidator::CalculateCacheEfficiency(std::shared_ptr<Mesh> mesh) {
    if (!mesh) return 0.0f;
    
    const auto& indices = mesh->GetIndices();
    if (indices.empty()) return 0.0f;
    
    // Simplified ACMR calculation
    const size_t cacheSize = 32; // Typical vertex cache size
    std::vector<bool> inCache(mesh->GetVertexCount(), false);
    size_t cacheHits = 0;
    size_t cachePosition = 0;
    
    for (uint32_t index : indices) {
        if (index < inCache.size() && inCache[index]) {
            cacheHits++;
        } else if (index < inCache.size()) {
            inCache[index] = true;
            cachePosition = (cachePosition + 1) % cacheSize;
        }
    }
    
    return static_cast<float>(cacheHits) / indices.size();
}

// Add missing ModelDiagnosticLogger methods
void ModelDiagnosticLogger::SetLogLevel(LogLevel minLevel) {
    m_minLevel = minLevel;
}

std::vector<ModelDiagnosticLogger::LogEntry> ModelDiagnosticLogger::GetRecentEntries(size_t count) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    if (count >= m_entries.size()) {
        return m_entries;
    }
    
    return std::vector<LogEntry>(m_entries.end() - count, m_entries.end());
}

// Stub implementations for format-specific validation
std::vector<ModelValidator::ValidationIssue> ModelValidator::ValidateOBJFile(const std::string& filepath) {
    std::vector<ValidationIssue> issues;
    // OBJ-specific validation would go here
    return issues;
}

std::vector<ModelValidator::ValidationIssue> ModelValidator::ValidateFBXFile(const std::string& filepath) {
    std::vector<ValidationIssue> issues;
    // FBX-specific validation would go here
    return issues;
}

std::vector<ModelValidator::ValidationIssue> ModelValidator::ValidateGLTFFile(const std::string& filepath) {
    std::vector<ValidationIssue> issues;
    // GLTF-specific validation would go here
    return issues;
}

// ModelDiagnosticLogger Implementation

ModelDiagnosticLogger& ModelDiagnosticLogger::GetInstance() {
    static ModelDiagnosticLogger instance;
    return instance;
}

void ModelDiagnosticLogger::LogTrace(const std::string& message, const std::string& component, const std::string& filepath) {
    if (m_minLevel <= LogLevel::Trace) {
        LogEntry entry;
        entry.level = LogLevel::Trace;
        entry.timestamp = std::chrono::system_clock::now();
        entry.message = message;
        entry.component = component;
        entry.filepath = filepath;
        WriteLogEntry(entry);
    }
}

void ModelDiagnosticLogger::LogDebug(const std::string& message, const std::string& component, const std::string& filepath) {
    if (m_minLevel <= LogLevel::Debug) {
        LogEntry entry;
        entry.level = LogLevel::Debug;
        entry.timestamp = std::chrono::system_clock::now();
        entry.message = message;
        entry.component = component;
        entry.filepath = filepath;
        WriteLogEntry(entry);
    }
}

void ModelDiagnosticLogger::LogInfo(const std::string& message, const std::string& component, const std::string& filepath) {
    if (m_minLevel <= LogLevel::Info) {
        LogEntry entry;
        entry.level = LogLevel::Info;
        entry.timestamp = std::chrono::system_clock::now();
        entry.message = message;
        entry.component = component;
        entry.filepath = filepath;
        WriteLogEntry(entry);
    }
}

void ModelDiagnosticLogger::LogWarning(const std::string& message, const std::string& component, const std::string& filepath) {
    if (m_minLevel <= LogLevel::Warning) {
        LogEntry entry;
        entry.level = LogLevel::Warning;
        entry.timestamp = std::chrono::system_clock::now();
        entry.message = message;
        entry.component = component;
        entry.filepath = filepath;
        WriteLogEntry(entry);
    }
}

void ModelDiagnosticLogger::LogError(const std::string& message, const std::string& component, const std::string& filepath) {
    if (m_minLevel <= LogLevel::Error) {
        LogEntry entry;
        entry.level = LogLevel::Error;
        entry.timestamp = std::chrono::system_clock::now();
        entry.message = message;
        entry.component = component;
        entry.filepath = filepath;
        WriteLogEntry(entry);
    }
}

void ModelDiagnosticLogger::LogCritical(const std::string& message, const std::string& component, const std::string& filepath) {
    if (m_minLevel <= LogLevel::Critical) {
        LogEntry entry;
        entry.level = LogLevel::Critical;
        entry.timestamp = std::chrono::system_clock::now();
        entry.message = message;
        entry.component = component;
        entry.filepath = filepath;
        WriteLogEntry(entry);
    }
}

void ModelDiagnosticLogger::WriteLogEntry(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    m_entries.push_back(entry);
    
    // Keep only recent entries to prevent memory growth
    if (m_entries.size() > 1000) {
        m_entries.erase(m_entries.begin(), m_entries.begin() + 100);
    }
    
    std::string formatted = FormatLogEntry(entry);
    
    if (m_consoleOutput) {
        Logger::GetInstance().Info("[ModelDiag] " + formatted);
    }
    
    if (m_fileOutput && !m_outputFile.empty()) {
        try {
            std::ofstream file(m_outputFile, std::ios::app);
            if (file.is_open()) {
                file << formatted << std::endl;
            }
        } catch (const std::exception&) {
            // Ignore file output errors
        }
    }
}

std::string ModelDiagnosticLogger::FormatLogEntry(const LogEntry& entry) {
    std::stringstream ss;
    
    auto timeT = std::chrono::system_clock::to_time_t(entry.timestamp);
    ss << std::put_time(std::localtime(&timeT), "%H:%M:%S");
    
    ss << " [" << GetLogLevelString(entry.level) << "]";
    
    if (!entry.component.empty()) {
        ss << " [" << entry.component << "]";
    }
    
    ss << " " << entry.message;
    
    if (!entry.filepath.empty()) {
        ss << " (File: " << entry.filepath << ")";
    }
    
    return ss.str();
}

std::string ModelDiagnosticLogger::GetLogLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRIT";
        default: return "UNKNOWN";
    }
}

} // namespace GameEngine