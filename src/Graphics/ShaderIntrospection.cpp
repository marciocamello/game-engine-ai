#include "Graphics/ShaderIntrospection.h"
#include "Graphics/Shader.h"
#include "Graphics/Material.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace GameEngine {

    // ShaderIntrospection implementation
    ShaderIntrospectionData ShaderIntrospection::IntrospectShader(std::shared_ptr<Shader> shader) {
        if (!shader || !shader->IsValid()) {
            ShaderIntrospectionData data;
            data.isValid = false;
            data.errors.push_back("Invalid shader provided");
            return data;
        }
        
        return IntrospectShaderProgram(shader->GetProgramID(), "Shader");
    }

    ShaderIntrospectionData ShaderIntrospection::IntrospectShaderProgram(uint32_t programId, const std::string& name) {
        ShaderIntrospectionData data;
        data.shaderName = name.empty() ? "Shader_" + std::to_string(programId) : name;
        data.programId = programId;
        
        if (programId == 0) {
            data.isValid = false;
            data.errors.push_back("Invalid program ID");
            return data;
        }
        
        try {
            // Get uniforms
            data.uniforms = GetShaderUniforms(programId);
            data.activeUniforms = static_cast<int>(data.uniforms.size());
            
            // Get attributes
            data.attributes = GetShaderAttributes(programId);
            data.activeAttributes = static_cast<int>(data.attributes.size());
            
            // Get storage buffers (for compute shaders)
            data.storageBuffers = GetStorageBuffers(programId);
            data.activeStorageBuffers = static_cast<int>(data.storageBuffers.size());
            
            // Count texture units
            data.textureUnits = 0;
            for (const auto& uniform : data.uniforms) {
                if (IsTextureType(uniform.type)) {
                    data.textureUnits++;
                }
            }
            
            // Get max texture units
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &data.maxTextureUnits);
            
            // Estimate performance metrics
            data.estimatedComplexity = EstimateShaderComplexity(programId);
            data.estimatedMemoryUsage = EstimateShaderMemoryUsage(programId);
            
            data.isValid = data.errors.empty();
            
        } catch (const std::exception& e) {
            data.isValid = false;
            data.errors.push_back("Exception during introspection: " + std::string(e.what()));
        }
        
        return data;
    }

    std::vector<UniformInfo> ShaderIntrospection::GetShaderUniforms(uint32_t programId) {
        std::vector<UniformInfo> uniforms;
        
        GLint uniformCount = 0;
        glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount);
        
        GLint maxNameLength = 0;
        glGetProgramiv(programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
        
        for (GLint i = 0; i < uniformCount; ++i) {
            std::vector<char> nameBuffer(maxNameLength);
            GLsizei nameLength = 0;
            GLint size = 0;
            GLenum type = 0;
            
            glGetActiveUniform(programId, i, maxNameLength, &nameLength, &size, &type, nameBuffer.data());
            
            std::string name(nameBuffer.data(), nameLength);
            GLint location = glGetUniformLocation(programId, name.c_str());
            
            UniformInfo info(name, location, type, size, location != -1);
            info.typeName = GetUniformTypeName(type);
            info.description = GetResourceDescription(name, type);
            
            uniforms.push_back(info);
        }
        
        return uniforms;
    }

    std::vector<AttributeInfo> ShaderIntrospection::GetShaderAttributes(uint32_t programId) {
        std::vector<AttributeInfo> attributes;
        
        GLint attributeCount = 0;
        glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &attributeCount);
        
        GLint maxNameLength = 0;
        glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);
        
        for (GLint i = 0; i < attributeCount; ++i) {
            std::vector<char> nameBuffer(maxNameLength);
            GLsizei nameLength = 0;
            GLint size = 0;
            GLenum type = 0;
            
            glGetActiveAttrib(programId, i, maxNameLength, &nameLength, &size, &type, nameBuffer.data());
            
            std::string name(nameBuffer.data(), nameLength);
            GLint location = glGetAttribLocation(programId, name.c_str());
            
            AttributeInfo info(name, location, type, size, location != -1);
            info.typeName = GetAttributeTypeName(type);
            info.description = GetResourceDescription(name, type);
            
            attributes.push_back(info);
        }
        
        return attributes;
    }

    std::vector<StorageBufferInfo> ShaderIntrospection::GetStorageBuffers(uint32_t programId) {
        std::vector<StorageBufferInfo> buffers;
        
        // Check if the OpenGL version supports shader storage buffers
        GLint major, minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        
        if (major < 4 || (major == 4 && minor < 3)) {
            // Shader storage buffers require OpenGL 4.3+
            return buffers;
        }
        
        GLint bufferCount = 0;
        glGetProgramInterfaceiv(programId, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &bufferCount);
        
        for (GLint i = 0; i < bufferCount; ++i) {
            GLint nameLength = 0;
            const GLenum nameLengthProp = GL_NAME_LENGTH;
            glGetProgramResourceiv(programId, GL_SHADER_STORAGE_BLOCK, i, 1, 
                                  &nameLengthProp, 1, nullptr, &nameLength);
            
            std::vector<char> nameBuffer(nameLength);
            glGetProgramResourceName(programId, GL_SHADER_STORAGE_BLOCK, i, nameLength, 
                                   nullptr, nameBuffer.data());
            
            std::string name(nameBuffer.data(), nameLength - 1); // Remove null terminator
            
            GLint binding = 0;
            GLint bufferDataSize = 0;
            const GLenum bufferBindingProp = GL_BUFFER_BINDING;
            const GLenum bufferDataSizeProp = GL_BUFFER_DATA_SIZE;
            glGetProgramResourceiv(programId, GL_SHADER_STORAGE_BLOCK, i, 1,
                                  &bufferBindingProp, 1, nullptr, &binding);
            glGetProgramResourceiv(programId, GL_SHADER_STORAGE_BLOCK, i, 1,
                                  &bufferDataSizeProp, 1, nullptr, &bufferDataSize);
            
            StorageBufferInfo info(name, binding, i, bufferDataSize, true);
            info.description = GetResourceDescription(name, GL_SHADER_STORAGE_BUFFER);
            
            buffers.push_back(info);
        }
        
        return buffers;
    }

    std::string ShaderIntrospection::GetUniformTypeName(uint32_t glType) {
        switch (glType) {
            case GL_FLOAT: return "float";
            case GL_FLOAT_VEC2: return "vec2";
            case GL_FLOAT_VEC3: return "vec3";
            case GL_FLOAT_VEC4: return "vec4";
            case GL_INT: return "int";
            case GL_INT_VEC2: return "ivec2";
            case GL_INT_VEC3: return "ivec3";
            case GL_INT_VEC4: return "ivec4";
            case GL_UNSIGNED_INT: return "uint";
            case GL_BOOL: return "bool";
            case GL_BOOL_VEC2: return "bvec2";
            case GL_BOOL_VEC3: return "bvec3";
            case GL_BOOL_VEC4: return "bvec4";
            case GL_FLOAT_MAT2: return "mat2";
            case GL_FLOAT_MAT3: return "mat3";
            case GL_FLOAT_MAT4: return "mat4";
            case GL_SAMPLER_2D: return "sampler2D";
            case GL_SAMPLER_CUBE: return "samplerCube";
            case GL_SAMPLER_3D: return "sampler3D";
            case GL_SAMPLER_2D_ARRAY: return "sampler2DArray";
            case GL_IMAGE_2D: return "image2D";
            case GL_IMAGE_3D: return "image3D";
            default: return "unknown(" + std::to_string(glType) + ")";
        }
    }

    std::string ShaderIntrospection::GetAttributeTypeName(uint32_t glType) {
        return GetUniformTypeName(glType); // Same type names for attributes
    }

    bool ShaderIntrospection::IsTextureType(uint32_t glType) {
        return glType == GL_SAMPLER_2D || glType == GL_SAMPLER_CUBE || 
               glType == GL_SAMPLER_3D || glType == GL_SAMPLER_2D_ARRAY ||
               glType == GL_IMAGE_2D || glType == GL_IMAGE_3D;
    }

    int ShaderIntrospection::EstimateShaderComplexity(uint32_t programId) {
        // This is a simplified complexity estimation
        int complexity = 0;
        
        // Base complexity from uniforms and attributes
        GLint uniformCount = 0, attributeCount = 0;
        glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount);
        glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &attributeCount);
        
        complexity += uniformCount * 2; // Each uniform adds some complexity
        complexity += attributeCount * 1; // Each attribute adds minimal complexity
        
        // Add complexity for texture samplers (more expensive)
        auto uniforms = GetShaderUniforms(programId);
        for (const auto& uniform : uniforms) {
            if (IsTextureType(uniform.type)) {
                complexity += 10; // Texture sampling is expensive
            }
        }
        
        return complexity;
    }

    size_t ShaderIntrospection::EstimateShaderMemoryUsage(uint32_t programId) {
        size_t memoryUsage = 0;
        
        // Base program memory
        memoryUsage += 1024; // Base shader program overhead
        
        // Memory for uniforms
        auto uniforms = GetShaderUniforms(programId);
        for (const auto& uniform : uniforms) {
            memoryUsage += GetTypeSize(uniform.type) * uniform.size;
        }
        
        // Memory for attributes
        auto attributes = GetShaderAttributes(programId);
        for (const auto& attribute : attributes) {
            memoryUsage += GetTypeSize(attribute.type) * attribute.size;
        }
        
        return memoryUsage;
    }

    int ShaderIntrospection::GetTypeSize(uint32_t glType) {
        switch (glType) {
            case GL_FLOAT: return 4;
            case GL_FLOAT_VEC2: return 8;
            case GL_FLOAT_VEC3: return 12;
            case GL_FLOAT_VEC4: return 16;
            case GL_INT: return 4;
            case GL_INT_VEC2: return 8;
            case GL_INT_VEC3: return 12;
            case GL_INT_VEC4: return 16;
            case GL_UNSIGNED_INT: return 4;
            case GL_BOOL: return 1;
            case GL_BOOL_VEC2: return 2;
            case GL_BOOL_VEC3: return 3;
            case GL_BOOL_VEC4: return 4;
            case GL_FLOAT_MAT2: return 16;
            case GL_FLOAT_MAT3: return 36;
            case GL_FLOAT_MAT4: return 64;
            case GL_SAMPLER_2D:
            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_3D:
            case GL_SAMPLER_2D_ARRAY:
            case GL_IMAGE_2D:
            case GL_IMAGE_3D: return 4; // Texture handle size
            default: return 4; // Default to 4 bytes
        }
    }

    std::string ShaderIntrospection::GetResourceDescription(const std::string& name, uint32_t type) {
        // Generate helpful descriptions based on common naming conventions
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find("mvp") != std::string::npos || lowerName.find("projection") != std::string::npos) {
            return "Model-View-Projection matrix for vertex transformation";
        } else if (lowerName.find("model") != std::string::npos) {
            return "Model transformation matrix";
        } else if (lowerName.find("view") != std::string::npos) {
            return "View transformation matrix";
        } else if (lowerName.find("normal") != std::string::npos) {
            return "Normal transformation matrix or normal vector";
        } else if (lowerName.find("light") != std::string::npos) {
            return "Lighting-related parameter";
        } else if (lowerName.find("color") != std::string::npos || lowerName.find("albedo") != std::string::npos) {
            return "Color or albedo value";
        } else if (lowerName.find("texture") != std::string::npos || lowerName.find("map") != std::string::npos) {
            return "Texture sampler";
        } else if (lowerName.find("time") != std::string::npos) {
            return "Time-based animation parameter";
        } else if (lowerName.find("position") != std::string::npos) {
            return "Position vector";
        }
        
        return "Shader resource";
    }

    std::string ShaderIntrospection::GenerateShaderReport(const ShaderIntrospectionData& data) {
        std::stringstream report;
        
        report << "=== Shader Introspection Report ===\n";
        report << "Shader: " << data.shaderName << " (ID: " << data.programId << ")\n";
        report << "Status: " << (data.isValid ? "Valid" : "Invalid") << "\n\n";
        
        // Resource summary
        report << "Resource Summary:\n";
        report << "  Active Uniforms: " << data.activeUniforms << "\n";
        report << "  Active Attributes: " << data.activeAttributes << "\n";
        report << "  Storage Buffers: " << data.activeStorageBuffers << "\n";
        report << "  Texture Units Used: " << data.textureUnits << "/" << data.maxTextureUnits << "\n\n";
        
        // Performance metrics
        report << "Performance Metrics:\n";
        report << "  Estimated Complexity: " << data.estimatedComplexity << "\n";
        report << "  Estimated Memory Usage: " << data.estimatedMemoryUsage << " bytes\n\n";
        
        // Uniforms
        if (!data.uniforms.empty()) {
            report << "Uniforms:\n";
            for (const auto& uniform : data.uniforms) {
                report << "  " << uniform.name << " (" << uniform.typeName << ")";
                if (uniform.size > 1) {
                    report << "[" << uniform.size << "]";
                }
                report << " - Location: " << uniform.location;
                if (!uniform.isActive) {
                    report << " [INACTIVE]";
                }
                report << "\n";
            }
            report << "\n";
        }
        
        // Attributes
        if (!data.attributes.empty()) {
            report << "Attributes:\n";
            for (const auto& attribute : data.attributes) {
                report << "  " << attribute.name << " (" << attribute.typeName << ")";
                if (attribute.size > 1) {
                    report << "[" << attribute.size << "]";
                }
                report << " - Location: " << attribute.location;
                if (!attribute.isActive) {
                    report << " [INACTIVE]";
                }
                report << "\n";
            }
            report << "\n";
        }
        
        // Warnings
        if (!data.warnings.empty()) {
            report << "Warnings:\n";
            for (const auto& warning : data.warnings) {
                report << "  - " << warning << "\n";
            }
            report << "\n";
        }
        
        // Errors
        if (!data.errors.empty()) {
            report << "Errors:\n";
            for (const auto& error : data.errors) {
                report << "  - " << error << "\n";
            }
        }
        
        return report.str();
    }

    void ShaderIntrospection::DumpShaderInfo(uint32_t programId, const std::string& shaderName) {
        auto data = IntrospectShaderProgram(programId, shaderName);
        std::string report = GenerateShaderReport(data);
        LOG_INFO("Shader Introspection:\n" + report);
    }

}