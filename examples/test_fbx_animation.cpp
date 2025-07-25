#include "Resource/FBXLoader.h"
#include "Core/Logger.h"
#include <iostream>

using namespace GameEngine;

int main() {
    std::cout << "=== Testing FBX Animation and Rigging ===" << std::endl;
    
    try {
        FBXLoader loader;
        std::cout << "1. Created FBXLoader" << std::endl;
        
        if (!loader.Initialize()) {
            std::cout << "ERROR: Failed to initialize FBX loader" << std::endl;
            return 1;
        }
        std::cout << "2. Initialized FBXLoader successfully" << std::endl;
        
        // Configure to import everything
        FBXLoader::FBXLoadingConfig config = loader.GetLoadingConfig();
        config.importMaterials = true;
        config.importTextures = false; // Keep disabled for now
        config.importSkeleton = true;
        config.importAnimations = true;
        config.optimizeMeshes = true;
        loader.SetLoadingConfig(config);
        std::cout << "3. Configured FBXLoader for full import" << std::endl;
        
        // Test loading XBot.fbx (T-Poser with skeleton)
        std::cout << "4. Starting to load XBot.fbx..." << std::endl;
        auto xbotResult = loader.LoadFBX("assets/meshes/XBot.fbx");
        std::cout << "5. XBot.fbx LoadFBX call completed" << std::endl;
        
        if (xbotResult.success) {
            std::cout << "SUCCESS: Loaded XBot.fbx" << std::endl;
            std::cout << "  Meshes: " << xbotResult.meshes.size() << std::endl;
            std::cout << "  Materials: " << xbotResult.materialCount << std::endl;
            std::cout << "  Vertices: " << xbotResult.totalVertices << std::endl;
            std::cout << "  Triangles: " << xbotResult.totalTriangles << std::endl;
            std::cout << "  Has Skeleton: " << (xbotResult.hasSkeleton ? "Yes" : "No") << std::endl;
            std::cout << "  Bone Count: " << xbotResult.boneCount << std::endl;
            std::cout << "  Has Animations: " << (xbotResult.hasAnimations ? "Yes" : "No") << std::endl;
            std::cout << "  Animation Count: " << xbotResult.animationCount << std::endl;
            std::cout << "  Source App: " << xbotResult.sourceApplication << std::endl;
            
            // Test skeleton
            if (xbotResult.skeleton) {
                std::cout << "\n--- Skeleton Information ---" << std::endl;
                std::cout << "  Total Bones: " << xbotResult.skeleton->GetBoneCount() << std::endl;
                auto rootBone = xbotResult.skeleton->GetRootBone();
                std::cout << "  Has Root Bone: " << (rootBone ? "Yes" : "No") << std::endl;
                if (rootBone) {
                    std::cout << "  Root Bone Name: " << rootBone->GetName() << std::endl;
                }
                
                // Print some bone names
                std::cout << "  Sample Bone Names:" << std::endl;
                const auto& bones = xbotResult.skeleton->GetBones();
                for (size_t i = 0; i < std::min(size_t(5), bones.size()); i++) {
                    if (bones[i]) {
                        std::cout << "    - " << bones[i]->GetName() << std::endl;
                    }
                }
            }
            
            // Test animations
            if (!xbotResult.animations.empty()) {
                std::cout << "\n--- Animation Information ---" << std::endl;
                for (const auto& animation : xbotResult.animations) {
                    if (animation) {
                        std::cout << "  Animation: " << animation->GetName() << std::endl;
                        std::cout << "    Duration: " << animation->GetDuration() << std::endl;
                        std::cout << "    Channels: " << animation->GetChannelCount() << std::endl;
                    }
                }
            }
        } else {
            std::cout << "ERROR: Failed to load XBot.fbx: " << xbotResult.errorMessage << std::endl;
        }
        
        std::cout << "\n6. Testing Idle.fbx (animation file)..." << std::endl;
        auto idleResult = loader.LoadFBX("assets/meshes/Idle.fbx");
        
        if (idleResult.success) {
            std::cout << "SUCCESS: Loaded Idle.fbx" << std::endl;
            std::cout << "  Meshes: " << idleResult.meshes.size() << std::endl;
            std::cout << "  Has Skeleton: " << (idleResult.hasSkeleton ? "Yes" : "No") << std::endl;
            std::cout << "  Bone Count: " << idleResult.boneCount << std::endl;
            std::cout << "  Has Animations: " << (idleResult.hasAnimations ? "Yes" : "No") << std::endl;
            std::cout << "  Animation Count: " << idleResult.animationCount << std::endl;
            
            // Test animations from Idle.fbx
            if (!idleResult.animations.empty()) {
                std::cout << "\n--- Idle Animation Information ---" << std::endl;
                for (const auto& animation : idleResult.animations) {
                    if (animation) {
                        std::cout << "  Animation: " << animation->GetName() << std::endl;
                        std::cout << "    Duration: " << animation->GetDuration() << std::endl;
                        std::cout << "    Channels: " << animation->GetChannelCount() << std::endl;
                        
                        // Test animation playback
                        std::cout << "    Animation playback test:" << std::endl;
                        std::cout << "      Current Time: " << animation->GetCurrentTime() << std::endl;
                        std::cout << "      Looping: " << (animation->IsLooping() ? "Yes" : "No") << std::endl;
                        std::cout << "      Playback Speed: " << animation->GetPlaybackSpeed() << std::endl;
                    }
                }
            }
        } else {
            std::cout << "ERROR: Failed to load Idle.fbx: " << idleResult.errorMessage << std::endl;
        }
        
        std::cout << "\n7. Shutting down loader..." << std::endl;
        loader.Shutdown();
        std::cout << "8. Animation and rigging test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "UNKNOWN EXCEPTION occurred" << std::endl;
        return 1;
    }
    
    return 0;
}