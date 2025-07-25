#include "Resource/MTLLoader.h"
#include "Resource/ResourceManager.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"
#include "Core/Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <cctype>

namespace GameEngine {

MTLLoader::LoadResult MTLLoader::LoadMTL(const std::string& filepath) {
    LoadResult result;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (m_verboseLogging) {
        Logger::GetInstance().Info("Loading MTL file: " + filepath);
    }
    
    if (!std::filesystem::exists(filepath)) {
        result.errorMessage = "MTL file not found: " + filepath;
        Logger::GetInstance().Error("MTLLoader::LoadMTL: " + result.errorMessage);
        return result;
    }
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        result.errorMessage = "Could not open MTL file: " + filepath;
        Logger::GetInstance().Error("MTLLoader::LoadMTL: " + result.errorMessage);
        return result;
    }
    
    std::string basePath = MTLLoader::GetDirectoryPath(filepath);
    std::string line;
    int lineNumber = 0;
    MTLMaterial currentMaterial;
    std::string currentMaterialName;
    bool hasMaterial = false;
    
    while (std::getline(file, line)) {
        lineNumber++;
        line = TrimString(line);
        
        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        }
        
        // Check for new material definition
        std::string materialName;
        if (ParseNewMaterial(line, materialName)) {
            // Save previous material if it exists
            if (hasMaterial && !currentMaterialName.empty()) {
                result.materials[currentMaterialName] = currentMaterial;
            }
            
            // Start new material
            currentMaterial = MTLMaterial();
            currentMaterial.name = materialName;
            currentMaterialName = materialName;
            hasMaterial = true;
            
            if (m_verboseLogging) {
                Logger::GetInstance().Debug("Found material: " + materialName);
            }
            continue;
        }
        
        // Parse material properties
        if (hasMaterial) {
            if (!ParseMTLLine(line, currentMaterial, basePath)) {
                if (m_verboseLogging) {
                    Logger::GetInstance().Warning("Could not parse line " + std::to_string(lineNumber) + 
                                                 " in MTL file " + filepath + ": " + line);
                }
            }
        }
    }
    
    // Save the last material
    if (hasMaterial && !currentMaterialName.empty()) {
        result.materials[currentMaterialName] = currentMaterial;
    }
    
    file.close();
    
    // Calculate loading time
    auto endTime = std::chrono::high_resolution_clock::now();
    result.loadingTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    result.materialCount = static_cast<uint32_t>(result.materials.size());
    result.success = result.materialCount > 0;
    
    if (result.success) {
        Logger::GetInstance().Info("Successfully loaded MTL file: " + filepath + 
                                 " (" + std::to_string(result.materialCount) + " materials, " +
                                 std::to_string(result.loadingTimeMs) + "ms)");
    } else {
        result.errorMessage = "No valid materials found in MTL file: " + filepath;
        Logger::GetInstance().Warning("MTLLoader::LoadMTL: " + result.errorMessage);
    }
    
    return result;
}

MTLLoader::LoadResult MTLLoader::LoadMTLFromString(const std::string& mtlContent, const std::string& basePath) {
    LoadResult result;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::istringstream stream(mtlContent);
    std::string line;
    int lineNumber = 0;
    MTLMaterial currentMaterial;
    std::string currentMaterialName;
    bool hasMaterial = false;
    
    while (std::getline(stream, line)) {
        lineNumber++;
        line = TrimString(line);
        
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Check for new material definition
        std::string materialName;
        if (ParseNewMaterial(line, materialName)) {
            // Save previous material if it exists
            if (hasMaterial && !currentMaterialName.empty()) {
                result.materials[currentMaterialName] = currentMaterial;
            }
            
            // Start new material
            currentMaterial = MTLMaterial();
            currentMaterial.name = materialName;
            currentMaterialName = materialName;
            hasMaterial = true;
            continue;
        }
        
        // Parse material properties
        if (hasMaterial) {
            ParseMTLLine(line, currentMaterial, basePath);
        }
    }
    
    // Save the last material
    if (hasMaterial && !currentMaterialName.empty()) {
        result.materials[currentMaterialName] = currentMaterial;
    }
    
    // Calculate loading time
    auto endTime = std::chrono::high_resolution_clock::now();
    result.loadingTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    result.materialCount = static_cast<uint32_t>(result.materials.size());
    result.success = result.materialCount > 0;
    
    return result;
}

std::shared_ptr<Material> MTLLoader::ConvertToEngineMaterial(const MTLMaterial& mtlMaterial, const std::string& basePath) {
    auto material = std::make_shared<Material>();
    
    // Set basic PBR properties
    // Convert traditional material properties to PBR approximation
    material->SetAlbedo(mtlMaterial.diffuse);
    
    // Estimate metallic from specular color (if specular is close to diffuse, it's likely metallic)
    float specularIntensity = (mtlMaterial.specular.r + mtlMaterial.specular.g + mtlMaterial.specular.b) / 3.0f;
    float diffuseIntensity = (mtlMaterial.diffuse.r + mtlMaterial.diffuse.g + mtlMaterial.diffuse.b) / 3.0f;
    float estimatedMetallic = (diffuseIntensity > 0.1f) ? std::min(specularIntensity / diffuseIntensity, 1.0f) : 0.0f;
    material->SetMetallic(mtlMaterial.metallic > 0.0f ? mtlMaterial.metallic : estimatedMetallic);
    
    // Convert shininess to roughness (inverse relationship)
    float estimatedRoughness = 1.0f - std::min(mtlMaterial.shininess / 128.0f, 1.0f);
    material->SetRoughness(mtlMaterial.roughness > 0.0f ? mtlMaterial.roughness : estimatedRoughness);
    
    // Set ambient occlusion (use ambient color intensity as approximation)
    float ambientIntensity = (mtlMaterial.ambient.r + mtlMaterial.ambient.g + mtlMaterial.ambient.b) / 3.0f;
    material->SetAO(ambientIntensity);
    
    // Set additional properties
    material->SetFloat("u_transparency", mtlMaterial.transparency);
    material->SetVec3("u_emissive", mtlMaterial.emissive);
    material->SetFloat("u_shininess", mtlMaterial.shininess);
    material->SetInt("u_illuminationModel", mtlMaterial.illuminationModel);
    
    // Load textures
    if (!mtlMaterial.diffuseMap.empty()) {
        auto texture = LoadTexture(mtlMaterial.diffuseMap, basePath);
        if (texture) {
            material->SetTexture("u_albedoMap", texture);
            material->SetTexture("u_diffuseMap", texture); // Legacy compatibility
        }
    }
    
    if (!mtlMaterial.normalMap.empty()) {
        auto texture = LoadTexture(mtlMaterial.normalMap, basePath);
        if (texture) {
            material->SetTexture("u_normalMap", texture);
        }
    }
    
    if (!mtlMaterial.specularMap.empty()) {
        auto texture = LoadTexture(mtlMaterial.specularMap, basePath);
        if (texture) {
            material->SetTexture("u_specularMap", texture);
        }
    }
    
    if (!mtlMaterial.metallicMap.empty()) {
        auto texture = LoadTexture(mtlMaterial.metallicMap, basePath);
        if (texture) {
            material->SetTexture("u_metallicMap", texture);
        }
    }
    
    if (!mtlMaterial.roughnessMap.empty()) {
        auto texture = LoadTexture(mtlMaterial.roughnessMap, basePath);
        if (texture) {
            material->SetTexture("u_roughnessMap", texture);
        }
    }
    
    if (!mtlMaterial.aoMap.empty()) {
        auto texture = LoadTexture(mtlMaterial.aoMap, basePath);
        if (texture) {
            material->SetTexture("u_aoMap", texture);
        }
    }
    
    if (!mtlMaterial.alphaMap.empty()) {
        auto texture = LoadTexture(mtlMaterial.alphaMap, basePath);
        if (texture) {
            material->SetTexture("u_alphaMap", texture);
        }
    }
    
    if (!mtlMaterial.heightMap.empty()) {
        auto texture = LoadTexture(mtlMaterial.heightMap, basePath);
        if (texture) {
            material->SetTexture("u_heightMap", texture);
        }
    }
    
    // Create default textures if none were loaded and option is enabled
    if (m_createDefaultTextures) {
        if (!material->GetTexture("u_albedoMap")) {
            auto defaultTexture = CreateDefaultTexture(mtlMaterial.diffuse);
            if (defaultTexture) {
                material->SetTexture("u_albedoMap", defaultTexture);
            }
        }
    }
    
    if (m_verboseLogging) {
        Logger::GetInstance().Debug("Converted MTL material '" + mtlMaterial.name + "' to engine material");
    }
    
    return material;
}

std::vector<std::shared_ptr<Material>> MTLLoader::ConvertAllMaterials(const LoadResult& result, const std::string& basePath) {
    std::vector<std::shared_ptr<Material>> materials;
    materials.reserve(result.materials.size());
    
    for (const auto& pair : result.materials) {
        auto material = ConvertToEngineMaterial(pair.second, basePath);
        if (material) {
            materials.push_back(material);
        }
    }
    
    return materials;
}

bool MTLLoader::IsMTLFile(const std::string& filepath) {
    if (filepath.length() < 4) return false;
    
    std::string extension = filepath.substr(filepath.length() - 4);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".mtl";
}

std::string MTLLoader::FindMTLFile(const std::string& objFilepath, const std::string& mtlFilename) {
    std::string objDir = MTLLoader::GetDirectoryPath(objFilepath);
    std::string mtlPath = objDir + "/" + mtlFilename;
    
    if (std::filesystem::exists(mtlPath)) {
        return mtlPath;
    }
    
    // Try without directory (same directory as OBJ)
    mtlPath = objDir + "/" + std::filesystem::path(mtlFilename).filename().string();
    if (std::filesystem::exists(mtlPath)) {
        return mtlPath;
    }
    
    return ""; // Not found
}

bool MTLLoader::ParseMTLLine(const std::string& line, MTLMaterial& currentMaterial, const std::string& basePath) {
    if (line.length() < 2) return false;
    
    std::istringstream iss(line);
    std::string command;
    iss >> command;
    
    // Convert command to lowercase for case-insensitive comparison
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);
    
    if (command == "ka") {
        // Ambient color
        return ParseColor(line, currentMaterial.ambient);
    }
    else if (command == "kd") {
        // Diffuse color
        return ParseColor(line, currentMaterial.diffuse);
    }
    else if (command == "ks") {
        // Specular color
        return ParseColor(line, currentMaterial.specular);
    }
    else if (command == "ke") {
        // Emissive color
        return ParseColor(line, currentMaterial.emissive);
    }
    else if (command == "ns") {
        // Shininess
        return ParseFloat(line, currentMaterial.shininess);
    }
    else if (command == "d") {
        // Transparency (dissolve)
        return ParseFloat(line, currentMaterial.transparency);
    }
    else if (command == "tr") {
        // Transparency (alternative)
        float tr;
        if (ParseFloat(line, tr)) {
            currentMaterial.transparency = 1.0f - tr; // Tr is inverted
            return true;
        }
        return false;
    }
    else if (command == "ni") {
        // Index of refraction
        return ParseFloat(line, currentMaterial.indexOfRefraction);
    }
    else if (command == "illum") {
        // Illumination model
        return ParseInt(line, currentMaterial.illuminationModel);
    }
    else if (command == "map_kd") {
        // Diffuse texture map
        return ParseTexture(line, currentMaterial.diffuseMap, basePath);
    }
    else if (command == "map_ka") {
        // Ambient texture map (or AO map in PBR)
        return ParseTexture(line, currentMaterial.ambientMap, basePath);
    }
    else if (command == "map_ks") {
        // Specular texture map
        return ParseTexture(line, currentMaterial.specularMap, basePath);
    }
    else if (command == "map_bump" || command == "bump") {
        // Normal/bump map
        return ParseTexture(line, currentMaterial.normalMap, basePath);
    }
    else if (command == "map_disp") {
        // Displacement/height map
        return ParseTexture(line, currentMaterial.heightMap, basePath);
    }
    else if (command == "map_d") {
        // Alpha map
        return ParseTexture(line, currentMaterial.alphaMap, basePath);
    }
    else if (command == "refl") {
        // Reflection map
        return ParseTexture(line, currentMaterial.reflectionMap, basePath);
    }
    // PBR extensions
    else if (command == "pm") {
        // Metallic
        return ParseFloat(line, currentMaterial.metallic);
    }
    else if (command == "pr") {
        // Roughness
        return ParseFloat(line, currentMaterial.roughness);
    }
    else if (command == "map_pm") {
        // Metallic map
        return ParseTexture(line, currentMaterial.metallicMap, basePath);
    }
    else if (command == "map_pr") {
        // Roughness map
        return ParseTexture(line, currentMaterial.roughnessMap, basePath);
    }
    
    return true; // Unknown commands are ignored but not considered errors
}

bool MTLLoader::ParseNewMaterial(const std::string& line, std::string& materialName) {
    std::istringstream iss(line);
    std::string command;
    iss >> command;
    
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);
    
    if (command == "newmtl") {
        iss >> materialName;
        return !materialName.empty();
    }
    
    return false;
}

bool MTLLoader::ParseColor(const std::string& line, Math::Vec3& color) {
    std::istringstream iss(line);
    std::string command;
    iss >> command; // Skip command
    
    return !!(iss >> color.r >> color.g >> color.b);
}

bool MTLLoader::ParseFloat(const std::string& line, float& value) {
    std::istringstream iss(line);
    std::string command;
    iss >> command; // Skip command
    
    return !!(iss >> value);
}

bool MTLLoader::ParseInt(const std::string& line, int& value) {
    std::istringstream iss(line);
    std::string command;
    iss >> command; // Skip command
    
    return !!(iss >> value);
}

bool MTLLoader::ParseTexture(const std::string& line, std::string& texturePath, const std::string& basePath) {
    std::istringstream iss(line);
    std::string command;
    iss >> command; // Skip command
    
    // Parse texture filename (may have options before it)
    std::string token;
    std::string filename;
    
    while (iss >> token) {
        // Skip texture options (like -blendu, -blendv, etc.)
        if (token[0] == '-') {
            // Skip option and its value
            std::string optionValue;
            if (iss >> optionValue) {
                continue;
            }
        } else {
            // This should be the filename
            filename = token;
            break;
        }
    }
    
    if (!filename.empty()) {
        texturePath = filename;
        return true;
    }
    
    return false;
}

std::shared_ptr<Texture> MTLLoader::LoadTexture(const std::string& texturePath, const std::string& basePath) {
    std::string resolvedPath = ResolveTexturePath(texturePath, basePath);
    
    if (resolvedPath.empty()) {
        if (m_verboseLogging) {
            Logger::GetInstance().Warning("Could not resolve texture path: " + texturePath);
        }
        return nullptr;
    }
    
    // Check cache first
    auto it = m_textureCache.find(resolvedPath);
    if (it != m_textureCache.end()) {
        return it->second;
    }
    
    // Create and load texture directly
    auto texture = std::make_shared<Texture>(resolvedPath);
    bool loaded = texture->LoadFromFile(resolvedPath);
    
    if (loaded) {
        m_textureCache[resolvedPath] = texture;
        if (m_verboseLogging) {
            Logger::GetInstance().Debug("Loaded texture: " + resolvedPath);
        }
    } else {
        if (m_verboseLogging) {
            Logger::GetInstance().Warning("Failed to load texture: " + resolvedPath);
        }
        texture = nullptr;
    }
    
    return texture;
}

std::string MTLLoader::ResolveTexturePath(const std::string& texturePath, const std::string& basePath) {
    // Try absolute path first
    if (std::filesystem::exists(texturePath)) {
        return texturePath;
    }
    
    // Try relative to base path
    if (!basePath.empty()) {
        std::string fullPath = basePath + "/" + texturePath;
        if (std::filesystem::exists(fullPath)) {
            return fullPath;
        }
    }
    
    // Try search paths
    for (const auto& searchPath : m_textureSearchPaths) {
        std::string fullPath = searchPath + "/" + texturePath;
        if (std::filesystem::exists(fullPath)) {
            return fullPath;
        }
    }
    
    // Try just the filename in base path (in case texture path includes directories)
    if (!basePath.empty()) {
        std::string filename = std::filesystem::path(texturePath).filename().string();
        std::string fullPath = basePath + "/" + filename;
        if (std::filesystem::exists(fullPath)) {
            return fullPath;
        }
    }
    
    return ""; // Not found
}

std::shared_ptr<Texture> MTLLoader::CreateDefaultTexture(const Math::Vec3& color) {
    // Create a simple 1x1 texture with the specified color
    // This would need to be implemented based on your texture creation system
    // For now, return nullptr and let the material system handle defaults
    return nullptr;
}

std::vector<std::string> MTLLoader::SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        token = TrimString(token);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::string MTLLoader::TrimString(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string MTLLoader::GetDirectoryPath(const std::string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos != std::string::npos) {
        return filepath.substr(0, pos);
    }
    return "."; // Current directory
}

} // namespace GameEngine