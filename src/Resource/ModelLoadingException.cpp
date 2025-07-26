#include "Resource/ModelLoadingException.h"
#include "Core/Logger.h"
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace GameEngine {

// ModelLoadingException Implementation

ModelLoadingException::ModelLoadingException(ErrorType type, 
                                           const std::string& message, 
                                           const std::string& filepath,
                                           Severity severity)
    : std::runtime_error(FormatMessage(message))
    , m_errorType(type)
    , m_severity(severity) {
    m_context.filepath = filepath;
}

ModelLoadingException::ModelLoadingException(ErrorType type,
                                           const std::string& message,
                                           const ErrorContext& context,
                                           Severity severity)
    : std::runtime_error(FormatMessage(message))
    , m_errorType(type)
    , m_severity(severity)
    , m_context(context) {
}

std::string ModelLoadingException::GetErrorTypeString() const {
    switch (m_errorType) {
        case ErrorType::FileNotFound: return "File Not Found";
        case ErrorType::UnsupportedFormat: return "Unsupported Format";
        case ErrorType::CorruptedFile: return "Corrupted File";
        case ErrorType::OutOfMemory: return "Out of Memory";
        case ErrorType::InvalidData: return "Invalid Data";
        case ErrorType::ImporterError: return "Importer Error";
        case ErrorType::PermissionDenied: return "Permission Denied";
        case ErrorType::NetworkError: return "Network Error";
        case ErrorType::TimeoutError: return "Timeout Error";
        case ErrorType::DependencyError: return "Dependency Error";
        case ErrorType::ValidationError: return "Validation Error";
        case ErrorType::ConversionError: return "Conversion Error";
        case ErrorType::UnknownError: return "Unknown Error";
        default: return "Unknown Error Type";
    }
}

std::string ModelLoadingException::GetSeverityString() const {
    switch (m_severity) {
        case Severity::Info: return "Info";
        case Severity::Warning: return "Warning";
        case Severity::Error: return "Error";
        case Severity::Critical: return "Critical";
        default: return "Unknown";
    }
}

std::string ModelLoadingException::GetDetailedMessage() const {
    std::stringstream ss;
    
    ss << "[" << GetSeverityString() << "] " << GetErrorTypeString() << ": " << what() << "\n";
    
    if (!m_context.filepath.empty()) {
        ss << "  File: " << m_context.filepath << "\n";
    }
    
    if (!m_context.formatHint.empty()) {
        ss << "  Format: " << m_context.formatHint << "\n";
    }
    
    if (m_context.fileSize > 0) {
        ss << "  File Size: " << m_context.fileSize << " bytes";
        if (m_context.fileSize > 1024 * 1024) {
            ss << " (" << (m_context.fileSize / 1024.0 / 1024.0) << " MB)";
        } else if (m_context.fileSize > 1024) {
            ss << " (" << (m_context.fileSize / 1024.0) << " KB)";
        }
        ss << "\n";
    }
    
    if (m_context.loadingTime.count() > 0) {
        ss << "  Loading Time: " << m_context.loadingTime.count() << "ms\n";
    }
    
    if (!m_context.systemInfo.empty()) {
        ss << "  System Info: " << m_context.systemInfo << "\n";
    }
    
    if (!m_context.additionalInfo.empty()) {
        ss << "  Additional Info:\n";
        for (const auto& info : m_context.additionalInfo) {
            ss << "    - " << info << "\n";
        }
    }
    
    return ss.str();
}

bool ModelLoadingException::IsRecoverable() const {
    switch (m_errorType) {
        case ErrorType::FileNotFound:
        case ErrorType::PermissionDenied:
        case ErrorType::OutOfMemory:
            return false; // Generally not recoverable
            
        case ErrorType::UnsupportedFormat:
        case ErrorType::CorruptedFile:
        case ErrorType::InvalidData:
        case ErrorType::ImporterError:
        case ErrorType::ValidationError:
        case ErrorType::ConversionError:
            return true; // May be recoverable with different strategies
            
        case ErrorType::NetworkError:
        case ErrorType::TimeoutError:
        case ErrorType::DependencyError:
            return true; // Often recoverable with retry
            
        case ErrorType::UnknownError:
        default:
            return false; // Unknown errors are not recoverable
    }
}

void ModelLoadingException::AddContextInfo(const std::string& info) {
    m_context.additionalInfo.push_back(info);
}

void ModelLoadingException::SetSystemInfo(const std::string& info) {
    m_context.systemInfo = info;
}

void ModelLoadingException::SetLoadingTime(std::chrono::milliseconds time) {
    m_context.loadingTime = time;
}

std::string ModelLoadingException::FormatMessage(const std::string& message) const {
    return GetErrorTypeString() + ": " + message;
}

// ModelValidationException Implementation

ModelValidationException::ModelValidationException(const std::string& message,
                                                 const std::string& filepath,
                                                 const std::vector<ValidationError>& errors)
    : ModelLoadingException(ErrorType::ValidationError, message, filepath, Severity::Error)
    , m_validationErrors(errors) {
}

void ModelValidationException::AddValidationError(const ValidationError& error) {
    m_validationErrors.push_back(error);
}

std::string ModelValidationException::GetValidationSummary() const {
    std::stringstream ss;
    
    ss << "Model validation failed with " << m_validationErrors.size() << " error(s):\n";
    
    for (size_t i = 0; i < m_validationErrors.size(); ++i) {
        const auto& error = m_validationErrors[i];
        ss << "  " << (i + 1) << ". ";
        
        if (error.isCritical) {
            ss << "[CRITICAL] ";
        }
        
        if (!error.component.empty()) {
            ss << error.component << ": ";
        }
        
        ss << error.description;
        
        if (!error.suggestion.empty()) {
            ss << " (Suggestion: " << error.suggestion << ")";
        }
        
        ss << "\n";
    }
    
    return ss.str();
}

bool ModelValidationException::HasCriticalErrors() const {
    return std::any_of(m_validationErrors.begin(), m_validationErrors.end(),
                      [](const ValidationError& error) { return error.isCritical; });
}

// ModelCorruptionException Implementation

ModelCorruptionException::ModelCorruptionException(const std::string& message,
                                                 const std::string& filepath,
                                                 CorruptionType corruptionType,
                                                 size_t corruptionOffset)
    : ModelLoadingException(ErrorType::CorruptedFile, message, filepath, Severity::Error)
    , m_corruptionType(corruptionType)
    , m_corruptionOffset(corruptionOffset) {
}

std::string ModelCorruptionException::GetCorruptionTypeString() const {
    switch (m_corruptionType) {
        case CorruptionType::InvalidHeader: return "Invalid Header";
        case CorruptionType::TruncatedFile: return "Truncated File";
        case CorruptionType::InvalidChecksum: return "Invalid Checksum";
        case CorruptionType::MalformedData: return "Malformed Data";
        case CorruptionType::MissingData: return "Missing Data";
        case CorruptionType::InconsistentData: return "Inconsistent Data";
        case CorruptionType::UnknownCorruption: return "Unknown Corruption";
        default: return "Unknown Corruption Type";
    }
}

std::string ModelCorruptionException::GetRecoveryAdvice() const {
    switch (m_corruptionType) {
        case CorruptionType::InvalidHeader:
            return "Try opening the file in the original application and re-exporting it.";
            
        case CorruptionType::TruncatedFile:
            return "The file appears to be incomplete. Re-download or re-export the file.";
            
        case CorruptionType::InvalidChecksum:
            return "File integrity check failed. Verify the file hasn't been corrupted during transfer.";
            
        case CorruptionType::MalformedData:
            return "Try loading with different import flags or use a file repair tool.";
            
        case CorruptionType::MissingData:
            return "Essential data sections are missing. Check if this is a complete file.";
            
        case CorruptionType::InconsistentData:
            return "Data inconsistencies detected. Try re-exporting from the source application.";
            
        case CorruptionType::UnknownCorruption:
        default:
            return "Unknown corruption detected. Try using a different file format or repair tool.";
    }
}

// ModelExceptionFactory Implementation

ModelLoadingException ModelExceptionFactory::CreateFileNotFoundError(const std::string& filepath) {
    ModelLoadingException::ErrorContext context;
    context.filepath = filepath;
    
    // Add file system information
    std::filesystem::path path(filepath);
    if (path.has_parent_path()) {
        std::string parentDir = path.parent_path().string();
        if (std::filesystem::exists(parentDir)) {
            context.additionalInfo.push_back("Parent directory exists: " + parentDir);
            
            // List similar files in the directory
            try {
                std::vector<std::string> similarFiles;
                std::string filename = path.filename().string();
                std::string stem = path.stem().string();
                
                for (const auto& entry : std::filesystem::directory_iterator(parentDir)) {
                    if (entry.is_regular_file()) {
                        std::string entryName = entry.path().filename().string();
                        std::string entryStem = entry.path().stem().string();
                        
                        // Check for similar names (case-insensitive)
                        std::string lowerFilename = filename;
                        std::string lowerEntryName = entryName;
                        std::transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);
                        std::transform(lowerEntryName.begin(), lowerEntryName.end(), lowerEntryName.begin(), ::tolower);
                        
                        if (lowerEntryName.find(stem) != std::string::npos || 
                            entryStem.find(stem) != std::string::npos) {
                            similarFiles.push_back(entryName);
                        }
                    }
                }
                
                if (!similarFiles.empty()) {
                    context.additionalInfo.push_back("Similar files found:");
                    for (const auto& file : similarFiles) {
                        context.additionalInfo.push_back("  - " + file);
                    }
                }
            } catch (const std::exception& e) {
                context.additionalInfo.push_back("Could not scan directory: " + std::string(e.what()));
            }
        } else {
            context.additionalInfo.push_back("Parent directory does not exist: " + parentDir);
        }
    }
    
    std::string message = "File not found or cannot be accessed: " + filepath;
    return ModelLoadingException(ModelLoadingException::ErrorType::FileNotFound, message, context);
}

ModelLoadingException ModelExceptionFactory::CreateUnsupportedFormatError(const std::string& filepath, const std::string& format) {
    ModelLoadingException::ErrorContext context;
    context.filepath = filepath;
    context.formatHint = format;
    
    context.additionalInfo.push_back("Detected format: " + format);
    context.additionalInfo.push_back("Supported formats: obj, fbx, gltf, glb, dae, 3ds, blend, x, ply, stl");
    
    std::string message = "Unsupported file format '" + format + "' for file: " + filepath;
    return ModelLoadingException(ModelLoadingException::ErrorType::UnsupportedFormat, message, context);
}

ModelLoadingException ModelExceptionFactory::CreateOutOfMemoryError(const std::string& filepath, size_t requestedBytes) {
    ModelLoadingException::ErrorContext context;
    context.filepath = filepath;
    
    context.additionalInfo.push_back("Requested memory: " + std::to_string(requestedBytes) + " bytes");
    if (requestedBytes > 1024 * 1024) {
        context.additionalInfo.push_back("Requested memory: " + std::to_string(requestedBytes / 1024.0 / 1024.0) + " MB");
    }
    
    std::string message = "Insufficient memory to load model: " + filepath;
    return ModelLoadingException(ModelLoadingException::ErrorType::OutOfMemory, message, context, ModelLoadingException::Severity::Critical);
}

ModelLoadingException ModelExceptionFactory::CreateImporterError(const std::string& filepath, const std::string& importerMessage) {
    ModelLoadingException::ErrorContext context;
    context.filepath = filepath;
    context.additionalInfo.push_back("Importer message: " + importerMessage);
    
    std::string message = "Model importer failed: " + importerMessage;
    return ModelLoadingException(ModelLoadingException::ErrorType::ImporterError, message, context);
}

ModelValidationException ModelExceptionFactory::CreateValidationError(const std::string& filepath, 
                                                                     const std::vector<ModelValidationException::ValidationError>& errors) {
    std::string message = "Model validation failed with " + std::to_string(errors.size()) + " error(s)";
    return ModelValidationException(message, filepath, errors);
}

ModelCorruptionException ModelExceptionFactory::CreateCorruptionError(const std::string& filepath, 
                                                                     ModelCorruptionException::CorruptionType type, 
                                                                     size_t offset) {
    std::string message = "File corruption detected";
    if (offset > 0) {
        message += " at offset " + std::to_string(offset);
    }
    return ModelCorruptionException(message, filepath, type, offset);
}

void ModelExceptionFactory::EnhanceWithSystemInfo(ModelLoadingException& exception) {
    std::stringstream ss;
    
    // Add basic system information
    ss << "Platform: Windows";
    
    // Add memory information if available
    try {
        // This is a simplified version - in a real implementation you might want to use Windows APIs
        ss << ", Available memory: Unknown";
    } catch (...) {
        // Ignore errors getting system info
    }
    
    exception.SetSystemInfo(ss.str());
}

void ModelExceptionFactory::EnhanceWithFileInfo(ModelLoadingException& exception, const std::string& filepath) {
    try {
        if (std::filesystem::exists(filepath)) {
            auto fileSize = std::filesystem::file_size(filepath);
            exception.GetContext().fileSize = fileSize;
            
            // Add file timestamp information
            auto lastWrite = std::filesystem::last_write_time(filepath);
            // Note: Converting file_time_type to string is complex in C++17/20
            // For now, we'll just note that the file exists
            exception.AddContextInfo("File exists and is accessible");
            
            // Check if file is empty
            if (fileSize == 0) {
                exception.AddContextInfo("File is empty (0 bytes)");
            }
            
            // Check if file is unusually large
            if (fileSize > 100 * 1024 * 1024) { // 100MB
                exception.AddContextInfo("File is very large (" + std::to_string(fileSize / 1024 / 1024) + " MB)");
            }
        }
    } catch (const std::exception& e) {
        exception.AddContextInfo("Could not get file information: " + std::string(e.what()));
    }
}

void ModelExceptionFactory::EnhanceWithTimingInfo(ModelLoadingException& exception, std::chrono::milliseconds loadingTime) {
    exception.SetLoadingTime(loadingTime);
    
    if (loadingTime.count() > 10000) { // 10 seconds
        exception.AddContextInfo("Loading took unusually long (" + std::to_string(loadingTime.count()) + "ms)");
    }
}

// ModelErrorRecovery Implementation

std::vector<ModelErrorRecovery::RecoveryStrategy> ModelErrorRecovery::GetRecoveryStrategies(const ModelLoadingException& exception) {
    std::vector<RecoveryStrategy> strategies;
    
    switch (exception.GetErrorType()) {
        case ModelLoadingException::ErrorType::UnsupportedFormat:
            strategies.push_back(RecoveryStrategy::ConvertFormat);
            strategies.push_back(RecoveryStrategy::FallbackToDefault);
            break;
            
        case ModelLoadingException::ErrorType::CorruptedFile:
            strategies.push_back(RecoveryStrategy::SkipCorruptedParts);
            strategies.push_back(RecoveryStrategy::RepairFile);
            strategies.push_back(RecoveryStrategy::FallbackToDefault);
            break;
            
        case ModelLoadingException::ErrorType::InvalidData:
        case ModelLoadingException::ErrorType::ValidationError:
            strategies.push_back(RecoveryStrategy::RetryWithDifferentFlags);
            strategies.push_back(RecoveryStrategy::SkipCorruptedParts);
            strategies.push_back(RecoveryStrategy::FallbackToDefault);
            break;
            
        case ModelLoadingException::ErrorType::ImporterError:
            strategies.push_back(RecoveryStrategy::RetryWithDifferentFlags);
            strategies.push_back(RecoveryStrategy::SimplifyModel);
            strategies.push_back(RecoveryStrategy::FallbackToDefault);
            break;
            
        case ModelLoadingException::ErrorType::OutOfMemory:
            strategies.push_back(RecoveryStrategy::SimplifyModel);
            strategies.push_back(RecoveryStrategy::FallbackToDefault);
            break;
            
        case ModelLoadingException::ErrorType::NetworkError:
        case ModelLoadingException::ErrorType::TimeoutError:
            // These would be handled by retry mechanisms at a higher level
            break;
            
        case ModelLoadingException::ErrorType::FileNotFound:
        case ModelLoadingException::ErrorType::PermissionDenied:
            strategies.push_back(RecoveryStrategy::FallbackToDefault);
            break;
            
        default:
            strategies.push_back(RecoveryStrategy::FallbackToDefault);
            break;
    }
    
    return strategies;
}

ModelErrorRecovery::RecoveryResult ModelErrorRecovery::AttemptRecovery(const ModelLoadingException& exception, RecoveryStrategy strategy) {
    RecoveryResult result;
    result.strategyUsed = strategy;
    
    switch (strategy) {
        case RecoveryStrategy::FallbackToDefault:
            result.message = "Using default fallback model";
            // In a real implementation, this would create a default model
            result.success = true;
            break;
            
        case RecoveryStrategy::RetryWithDifferentFlags:
            result.message = "Retrying with different loading flags";
            // This would be implemented by the caller with different ModelLoader flags
            result.success = false; // Caller needs to implement
            break;
            
        case RecoveryStrategy::SimplifyModel:
            result.message = "Attempting to load simplified version";
            // This would involve reducing quality settings
            result.success = false; // Caller needs to implement
            break;
            
        case RecoveryStrategy::SkipCorruptedParts:
            result.message = "Attempting to skip corrupted parts";
            // This would involve partial loading
            result.success = false; // Caller needs to implement
            break;
            
        case RecoveryStrategy::ConvertFormat:
            result.message = "Format conversion not implemented";
            result.success = false;
            break;
            
        case RecoveryStrategy::RepairFile:
            result.message = "File repair not implemented";
            result.success = false;
            break;
            
        default:
            result.message = "Recovery strategy not implemented";
            result.success = false;
            break;
    }
    
    return result;
}

bool ModelErrorRecovery::IsRecoveryPossible(const ModelLoadingException& exception) {
    auto strategies = GetRecoveryStrategies(exception);
    return !strategies.empty();
}

std::string ModelErrorRecovery::GetRecoveryStrategyDescription(RecoveryStrategy strategy) {
    switch (strategy) {
        case RecoveryStrategy::None:
            return "No recovery possible";
        case RecoveryStrategy::RetryWithDifferentFlags:
            return "Retry loading with different processing flags";
        case RecoveryStrategy::FallbackToDefault:
            return "Use a default fallback model";
        case RecoveryStrategy::SimplifyModel:
            return "Attempt to load a simplified version";
        case RecoveryStrategy::SkipCorruptedParts:
            return "Skip corrupted parts and load the rest";
        case RecoveryStrategy::ConvertFormat:
            return "Convert to a supported format";
        case RecoveryStrategy::RepairFile:
            return "Attempt to repair the corrupted file";
        default:
            return "Unknown recovery strategy";
    }
}

} // namespace GameEngine