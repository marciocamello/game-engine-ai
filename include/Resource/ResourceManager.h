#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <typeindex>

namespace GameEngine {
    class Resource {
    public:
        Resource(const std::string& path) : m_path(path) {}
        virtual ~Resource() = default;
        
        const std::string& GetPath() const { return m_path; }
        
    protected:
        std::string m_path;
    };

    class ResourceManager {
    public:
        ResourceManager();
        ~ResourceManager();

        bool Initialize();
        void Shutdown();

        template<typename T>
        std::shared_ptr<T> Load(const std::string& path);

        template<typename T>
        void Unload(const std::string& path);

        void UnloadAll();
        
        // Asset pipeline functions
        bool ImportAsset(const std::string& sourcePath, const std::string& targetPath);
        bool ExportAsset(const std::string& assetPath, const std::string& exportPath);

    private:
        template<typename T>
        std::string GetResourceKey(const std::string& path);

        std::unordered_map<std::string, std::shared_ptr<Resource>> m_resources;
        std::string m_assetDirectory = "assets/";
    };

    template<typename T>
    std::shared_ptr<T> ResourceManager::Load(const std::string& path) {
        std::string key = GetResourceKey<T>(path);
        
        auto it = m_resources.find(key);
        if (it != m_resources.end()) {
            return std::static_pointer_cast<T>(it->second);
        }

        // Create new resource
        auto resource = std::make_shared<T>(m_assetDirectory + path);
        m_resources[key] = resource;
        return resource;
    }

    template<typename T>
    void ResourceManager::Unload(const std::string& path) {
        std::string key = GetResourceKey<T>(path);
        m_resources.erase(key);
    }

    template<typename T>
    std::string ResourceManager::GetResourceKey(const std::string& path) {
        return std::string(typeid(T).name()) + ":" + path;
    }
}