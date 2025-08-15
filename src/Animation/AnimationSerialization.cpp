#include "Animation/AnimationSerialization.h"
#include "Core/Logger.h"
#include <fstream>
#include <chrono>

#ifdef GAMEENGINE_HAS_JSON
#include <nlohmann/json.hpp>
#endif

namespace GameEngine {
namespace Animation {

    // Version information
    static const std::string ANIMATION_SERIALIZATION_VERSION = "1.0.0";
    static const std::vector<std::string> COMPATIBLE_VERSIONS = {"1.0.0"};

    std::string AnimationSerialization::GetCurrentVersion() {
        return ANIMATION_SERIALIZATION_VERSION;
    }

    bool AnimationSerialization::IsVersionCompatible(const std::string& version) {
        return std::find(COMPATIBLE_VERSIONS.begin(), COMPATIBLE_VERSIONS.end(), version) != COMPATIBLE_VERSIONS.end();
    }

#ifdef GAMEENGINE_HAS_JSON

    // Skeletal Animation serialization
    std::string AnimationSerialization::SerializeSkeletalAnimation(const SkeletalAnimation& animation) {
        try {
            nlohmann::json json = SkeletalAnimationToJson(animation);
            return json.dump(4);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to serialize skeletal animation: " + std::string(e.what()));
            return "";
        }
    }

    std::shared_ptr<SkeletalAnimation> AnimationSerialization::DeserializeSkeletalAnimation(const std::string& jsonData) {
        try {
            nlohmann::json json = nlohmann::json::parse(jsonData);
            return JsonToSkeletalAnimation(json);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to deserialize skeletal animation: " + std::string(e.what()));
            return nullptr;
        }
    }

    // State Machine serialization
    std::string AnimationSerialization::SerializeStateMachine(const AnimationStateMachine& stateMachine) {
        try {
            nlohmann::json json = StateMachineToJson(stateMachine);
            return json.dump(4);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to serialize state machine: " + std::string(e.what()));
            return "";
        }
    }

    std::shared_ptr<AnimationStateMachine> AnimationSerialization::DeserializeStateMachine(const std::string& jsonData) {
        try {
            nlohmann::json json = nlohmann::json::parse(jsonData);
            return JsonToStateMachine(json);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to deserialize state machine: " + std::string(e.what()));
            return nullptr;
        }
    }

    // Blend Tree serialization
    std::string AnimationSerialization::SerializeBlendTree(const BlendTree& blendTree) {
        try {
            nlohmann::json json = BlendTreeToJson(blendTree);
            return json.dump(4);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to serialize blend tree: " + std::string(e.what()));
            return "";
        }
    }

    std::shared_ptr<BlendTree> AnimationSerialization::DeserializeBlendTree(const std::string& jsonData) {
        try {
            nlohmann::json json = nlohmann::json::parse(jsonData);
            return JsonToBlendTree(json);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to deserialize blend tree: " + std::string(e.what()));
            return nullptr;
        }
    }

    // Animation Transition serialization
    std::string AnimationSerialization::SerializeTransition(const AnimationTransition& transition) {
        try {
            nlohmann::json json = TransitionToJson(transition);
            return json.dump(4);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to serialize transition: " + std::string(e.what()));
            return "";
        }
    }

    std::shared_ptr<AnimationTransition> AnimationSerialization::DeserializeTransition(const std::string& jsonData) {
        try {
            nlohmann::json json = nlohmann::json::parse(jsonData);
            return JsonToTransition(json);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to deserialize transition: " + std::string(e.what()));
            return nullptr;
        }
    }

    // File I/O operations
    bool AnimationSerialization::SaveAnimationToFile(const SkeletalAnimation& animation, const std::string& filepath) {
        try {
            nlohmann::json json = SkeletalAnimationToJson(animation);
            return WriteJsonToFile(json, filepath);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save animation to file: " + std::string(e.what()));
            return false;
        }
    }

    std::shared_ptr<SkeletalAnimation> AnimationSerialization::LoadAnimationFromFile(const std::string& filepath) {
        try {
            nlohmann::json json = ReadJsonFromFile(filepath);
            if (json.empty()) {
                return nullptr;
            }
            return JsonToSkeletalAnimation(json);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load animation from file: " + std::string(e.what()));
            return nullptr;
        }
    }

    bool AnimationSerialization::SaveStateMachineToFile(const AnimationStateMachine& stateMachine, const std::string& filepath) {
        try {
            nlohmann::json json = StateMachineToJson(stateMachine);
            return WriteJsonToFile(json, filepath);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save state machine to file: " + std::string(e.what()));
            return false;
        }
    }

    std::shared_ptr<AnimationStateMachine> AnimationSerialization::LoadStateMachineFromFile(const std::string& filepath) {
        try {
            nlohmann::json json = ReadJsonFromFile(filepath);
            if (json.empty()) {
                return nullptr;
            }
            return JsonToStateMachine(json);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load state machine from file: " + std::string(e.what()));
            return nullptr;
        }
    }

    bool AnimationSerialization::SaveBlendTreeToFile(const BlendTree& blendTree, const std::string& filepath) {
        try {
            nlohmann::json json = BlendTreeToJson(blendTree);
            return WriteJsonToFile(json, filepath);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save blend tree to file: " + std::string(e.what()));
            return false;
        }
    }

    std::shared_ptr<BlendTree> AnimationSerialization::LoadBlendTreeFromFile(const std::string& filepath) {
        try {
            nlohmann::json json = ReadJsonFromFile(filepath);
            if (json.empty()) {
                return nullptr;
            }
            return JsonToBlendTree(json);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load blend tree from file: " + std::string(e.what()));
            return nullptr;
        }
    }

    // Asset pipeline integration
    std::string AnimationSerialization::SerializeAnimationAsset(const AnimationAsset& asset) {
        try {
            nlohmann::json json;
            json["name"] = asset.name;
            json["type"] = asset.type;
            json["version"] = asset.version;
            json["sourceFile"] = asset.sourceFile;
            json["data"] = asset.data;
            json["timestamp"] = asset.timestamp;
            json["dataSize"] = asset.dataSize;
            
            return json.dump(4);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to serialize animation asset: " + std::string(e.what()));
            return "";
        }
    }

    AnimationSerialization::AnimationAsset AnimationSerialization::DeserializeAnimationAsset(const std::string& jsonData) {
        AnimationAsset asset;
        try {
            nlohmann::json json = nlohmann::json::parse(jsonData);
            
            asset.name = json.value("name", "");
            asset.type = json.value("type", "");
            asset.version = json.value("version", "");
            asset.sourceFile = json.value("sourceFile", "");
            asset.data = json.value("data", "");
            asset.timestamp = json.value("timestamp", 0ULL);
            asset.dataSize = json.value("dataSize", 0U);
            
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to deserialize animation asset: " + std::string(e.what()));
        }
        return asset;
    }

    bool AnimationSerialization::SaveAnimationAsset(const AnimationAsset& asset, const std::string& filepath) {
        try {
            std::string jsonData = SerializeAnimationAsset(asset);
            if (jsonData.empty()) {
                return false;
            }
            
            std::ofstream file(filepath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file for writing: " + filepath);
                return false;
            }
            
            file << jsonData;
            return file.good();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save animation asset: " + std::string(e.what()));
            return false;
        }
    }

    AnimationSerialization::AnimationAsset AnimationSerialization::LoadAnimationAsset(const std::string& filepath) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file for reading: " + filepath);
                return AnimationAsset{};
            }
            
            std::string jsonData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            return DeserializeAnimationAsset(jsonData);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load animation asset: " + std::string(e.what()));
            return AnimationAsset{};
        }
    }

    // Batch operations
    std::string AnimationSerialization::SerializeAnimationCollection(const AnimationCollection& collection) {
        try {
            nlohmann::json json;
            json["name"] = collection.name;
            json["version"] = collection.version;
            
            json["animations"] = nlohmann::json::array();
            for (const auto& asset : collection.animations) {
                nlohmann::json assetJson = nlohmann::json::parse(SerializeAnimationAsset(asset));
                json["animations"].push_back(assetJson);
            }
            
            json["stateMachines"] = nlohmann::json::array();
            for (const auto& asset : collection.stateMachines) {
                nlohmann::json assetJson = nlohmann::json::parse(SerializeAnimationAsset(asset));
                json["stateMachines"].push_back(assetJson);
            }
            
            json["blendTrees"] = nlohmann::json::array();
            for (const auto& asset : collection.blendTrees) {
                nlohmann::json assetJson = nlohmann::json::parse(SerializeAnimationAsset(asset));
                json["blendTrees"].push_back(assetJson);
            }
            
            return json.dump(4);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to serialize animation collection: " + std::string(e.what()));
            return "";
        }
    }

    AnimationSerialization::AnimationCollection AnimationSerialization::DeserializeAnimationCollection(const std::string& jsonData) {
        AnimationCollection collection;
        try {
            nlohmann::json json = nlohmann::json::parse(jsonData);
            
            collection.name = json.value("name", "");
            collection.version = json.value("version", "");
            
            if (json.contains("animations") && json["animations"].is_array()) {
                for (const auto& assetJson : json["animations"]) {
                    AnimationAsset asset = DeserializeAnimationAsset(assetJson.dump());
                    collection.animations.push_back(asset);
                }
            }
            
            if (json.contains("stateMachines") && json["stateMachines"].is_array()) {
                for (const auto& assetJson : json["stateMachines"]) {
                    AnimationAsset asset = DeserializeAnimationAsset(assetJson.dump());
                    collection.stateMachines.push_back(asset);
                }
            }
            
            if (json.contains("blendTrees") && json["blendTrees"].is_array()) {
                for (const auto& assetJson : json["blendTrees"]) {
                    AnimationAsset asset = DeserializeAnimationAsset(assetJson.dump());
                    collection.blendTrees.push_back(asset);
                }
            }
            
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to deserialize animation collection: " + std::string(e.what()));
        }
        return collection;
    }

    bool AnimationSerialization::SaveAnimationCollection(const AnimationCollection& collection, const std::string& filepath) {
        try {
            std::string jsonData = SerializeAnimationCollection(collection);
            if (jsonData.empty()) {
                return false;
            }
            
            std::ofstream file(filepath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file for writing: " + filepath);
                return false;
            }
            
            file << jsonData;
            return file.good();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save animation collection: " + std::string(e.what()));
            return false;
        }
    }

    AnimationSerialization::AnimationCollection AnimationSerialization::LoadAnimationCollection(const std::string& filepath) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file for reading: " + filepath);
                return AnimationCollection{};
            }
            
            std::string jsonData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            return DeserializeAnimationCollection(jsonData);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load animation collection: " + std::string(e.what()));
            return AnimationCollection{};
        }
    }

    // Validation and versioning
    bool AnimationSerialization::ValidateAnimationData(const std::string& jsonData, const std::string& expectedType) {
        try {
            nlohmann::json json = nlohmann::json::parse(jsonData);
            
            // Check for required fields
            if (!json.contains("version") || !json.contains("type")) {
                LOG_ERROR("Animation data missing required version or type fields");
                return false;
            }
            
            std::string version = json["version"];
            std::string type = json["type"];
            
            // Validate version compatibility
            if (!IsVersionCompatible(version)) {
                LOG_ERROR("Incompatible animation data version: " + version);
                return false;
            }
            
            // Validate type if specified
            if (!expectedType.empty() && type != expectedType) {
                LOG_ERROR("Animation data type mismatch. Expected: " + expectedType + ", Got: " + type);
                return false;
            }
            
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to validate animation data: " + std::string(e.what()));
            return false;
        }
    }

    // JSON conversion helpers - SkeletalAnimation
    nlohmann::json AnimationSerialization::SkeletalAnimationToJson(const SkeletalAnimation& animation) {
        nlohmann::json json;
        
        // Basic properties
        json["type"] = "skeletal_animation";
        json["version"] = GetCurrentVersion();
        json["name"] = animation.GetName();
        json["duration"] = animation.GetDuration();
        json["frameRate"] = animation.GetFrameRate();
        json["loopMode"] = static_cast<int>(animation.GetLoopMode());
        
        // Bone animations
        json["boneAnimations"] = nlohmann::json::array();
        const auto& boneAnimations = animation.GetBoneAnimations();
        
        for (const auto& [boneName, boneAnim] : boneAnimations) {
            nlohmann::json boneJson;
            boneJson["boneName"] = boneName;
            
            // Position track
            if (boneAnim->HasPositionTrack()) {
                const auto* posTrack = boneAnim->positionTrack.get();
                boneJson["positionKeyframes"] = nlohmann::json::array();
                
                for (const auto& keyframe : posTrack->GetKeyframes()) {
                    nlohmann::json keyJson = PositionKeyframeToJson(keyframe);
                    boneJson["positionKeyframes"].push_back(keyJson);
                }
            }
            
            // Rotation track
            if (boneAnim->HasRotationTrack()) {
                const auto* rotTrack = boneAnim->rotationTrack.get();
                boneJson["rotationKeyframes"] = nlohmann::json::array();
                
                for (const auto& keyframe : rotTrack->GetKeyframes()) {
                    nlohmann::json keyJson = RotationKeyframeToJson(keyframe);
                    boneJson["rotationKeyframes"].push_back(keyJson);
                }
            }
            
            // Scale track
            if (boneAnim->HasScaleTrack()) {
                const auto* scaleTrack = boneAnim->scaleTrack.get();
                boneJson["scaleKeyframes"] = nlohmann::json::array();
                
                for (const auto& keyframe : scaleTrack->GetKeyframes()) {
                    nlohmann::json keyJson = ScaleKeyframeToJson(keyframe);
                    boneJson["scaleKeyframes"].push_back(keyJson);
                }
            }
            
            json["boneAnimations"].push_back(boneJson);
        }
        
        // Events
        json["events"] = nlohmann::json::array();
        const auto events = animation.GetEvents();
        for (const auto& event : events) {
            nlohmann::json eventJson;
            eventJson["name"] = event.name;
            eventJson["time"] = event.time;
            eventJson["type"] = static_cast<int>(event.type);
            eventJson["stringParameter"] = event.stringParameter;
            eventJson["floatParameter"] = event.floatParameter;
            eventJson["intParameter"] = event.intParameter;
            eventJson["boolParameter"] = event.boolParameter;
            json["events"].push_back(eventJson);
        }
        
        return json;
    }

    std::shared_ptr<SkeletalAnimation> AnimationSerialization::JsonToSkeletalAnimation(const nlohmann::json& json) {
        if (!json.contains("name")) {
            LOG_ERROR("SkeletalAnimation JSON missing name field");
            return nullptr;
        }
        
        auto animation = std::make_shared<SkeletalAnimation>(json["name"]);
        
        // Basic properties
        if (json.contains("duration")) {
            animation->SetDuration(json["duration"]);
        }
        if (json.contains("frameRate")) {
            animation->SetFrameRate(json["frameRate"]);
        }
        if (json.contains("loopMode")) {
            animation->SetLoopMode(static_cast<LoopMode>(json["loopMode"]));
        }
        
        // Bone animations
        if (json.contains("boneAnimations") && json["boneAnimations"].is_array()) {
            for (const auto& boneJson : json["boneAnimations"]) {
                if (!boneJson.contains("boneName")) continue;
                
                std::string boneName = boneJson["boneName"];
                
                // Position keyframes
                if (boneJson.contains("positionKeyframes") && boneJson["positionKeyframes"].is_array()) {
                    for (const auto& keyJson : boneJson["positionKeyframes"]) {
                        PositionKeyframe keyframe = JsonToPositionKeyframe(keyJson);
                        animation->AddPositionKeyframe(boneName, keyframe.time, keyframe.value);
                    }
                }
                
                // Rotation keyframes
                if (boneJson.contains("rotationKeyframes") && boneJson["rotationKeyframes"].is_array()) {
                    for (const auto& keyJson : boneJson["rotationKeyframes"]) {
                        RotationKeyframe keyframe = JsonToRotationKeyframe(keyJson);
                        animation->AddRotationKeyframe(boneName, keyframe.time, keyframe.value);
                    }
                }
                
                // Scale keyframes
                if (boneJson.contains("scaleKeyframes") && boneJson["scaleKeyframes"].is_array()) {
                    for (const auto& keyJson : boneJson["scaleKeyframes"]) {
                        ScaleKeyframe keyframe = JsonToScaleKeyframe(keyJson);
                        animation->AddScaleKeyframe(boneName, keyframe.time, keyframe.value);
                    }
                }
            }
        }
        
        // Events
        if (json.contains("events") && json["events"].is_array()) {
            for (const auto& eventJson : json["events"]) {
                AnimationEvent event;
                event.name = eventJson.value("name", "");
                event.time = eventJson.value("time", 0.0f);
                event.type = static_cast<AnimationEventType>(eventJson.value("type", 0));
                event.stringParameter = eventJson.value("stringParameter", "");
                event.floatParameter = eventJson.value("floatParameter", 0.0f);
                event.intParameter = eventJson.value("intParameter", 0);
                event.boolParameter = eventJson.value("boolParameter", false);
                animation->AddEvent(event);
            }
        }
        
        return animation;
    }

    // JSON conversion helpers - StateMachine
    nlohmann::json AnimationSerialization::StateMachineToJson(const AnimationStateMachine& stateMachine) {
        nlohmann::json json;
        
        json["type"] = "state_machine";
        json["version"] = GetCurrentVersion();
        json["entryState"] = stateMachine.GetEntryState();
        json["defaultState"] = stateMachine.GetDefaultState();
        
        // States
        json["states"] = nlohmann::json::array();
        const auto states = stateMachine.GetAllStates();
        for (const auto& state : states) {
            nlohmann::json stateJson;
            stateJson["name"] = state->GetName();
            stateJson["type"] = static_cast<int>(state->GetType());
            stateJson["speed"] = state->GetSpeed();
            stateJson["looping"] = state->IsLooping();
            
            // State content based on type
            switch (state->GetType()) {
                case AnimationState::Type::Single:
                    if (auto anim = state->GetAnimation()) {
                        stateJson["animation"] = SkeletalAnimationToJson(*anim);
                    }
                    break;
                case AnimationState::Type::BlendTree:
                    if (auto blendTree = state->GetBlendTree()) {
                        stateJson["blendTree"] = BlendTreeToJson(*blendTree);
                    }
                    break;
                case AnimationState::Type::SubStateMachine:
                    if (auto subStateMachine = state->GetSubStateMachine()) {
                        stateJson["subStateMachine"] = StateMachineToJson(*subStateMachine);
                    }
                    break;
            }
            
            json["states"].push_back(stateJson);
        }
        
        // Transitions
        json["transitions"] = nlohmann::json::array();
        const auto stateNames = stateMachine.GetStateNames();
        for (const auto& fromState : stateNames) {
            const auto transitions = stateMachine.GetTransitions(fromState);
            for (const auto& transition : transitions) {
                nlohmann::json transitionJson = TransitionToJson(*transition);
                transitionJson["fromState"] = fromState;
                json["transitions"].push_back(transitionJson);
            }
        }
        
        return json;
    }

    std::shared_ptr<AnimationStateMachine> AnimationSerialization::JsonToStateMachine(const nlohmann::json& json) {
        auto stateMachine = std::make_shared<AnimationStateMachine>();
        
        // Basic properties
        if (json.contains("entryState")) {
            stateMachine->SetEntryState(json["entryState"]);
        }
        if (json.contains("defaultState")) {
            stateMachine->SetDefaultState(json["defaultState"]);
        }
        
        // States
        if (json.contains("states") && json["states"].is_array()) {
            for (const auto& stateJson : json["states"]) {
                if (!stateJson.contains("name")) continue;
                
                std::string name = stateJson["name"];
                auto stateType = static_cast<AnimationState::Type>(stateJson.value("type", 0));
                
                auto state = std::make_shared<AnimationState>(name, stateType);
                state->SetSpeed(stateJson.value("speed", 1.0f));
                state->SetLooping(stateJson.value("looping", true));
                
                // State content
                switch (stateType) {
                    case AnimationState::Type::Single:
                        if (stateJson.contains("animation")) {
                            auto animation = JsonToSkeletalAnimation(stateJson["animation"]);
                            state->SetAnimation(animation);
                        }
                        break;
                    case AnimationState::Type::BlendTree:
                        if (stateJson.contains("blendTree")) {
                            auto blendTree = JsonToBlendTree(stateJson["blendTree"]);
                            state->SetBlendTree(blendTree);
                        }
                        break;
                    case AnimationState::Type::SubStateMachine:
                        if (stateJson.contains("subStateMachine")) {
                            auto subStateMachine = JsonToStateMachine(stateJson["subStateMachine"]);
                            state->SetSubStateMachine(subStateMachine);
                        }
                        break;
                }
                
                stateMachine->AddState(state);
            }
        }
        
        // Transitions
        if (json.contains("transitions") && json["transitions"].is_array()) {
            for (const auto& transitionJson : json["transitions"]) {
                if (!transitionJson.contains("fromState") || !transitionJson.contains("toState")) {
                    continue;
                }
                
                std::string fromState = transitionJson["fromState"];
                std::string toState = transitionJson["toState"];
                auto transition = JsonToTransition(transitionJson);
                
                if (transition) {
                    stateMachine->AddTransition(fromState, toState, transition);
                }
            }
        }
        
        return stateMachine;
    }

    // JSON conversion helpers - BlendTree
    nlohmann::json AnimationSerialization::BlendTreeToJson(const BlendTree& blendTree) {
        nlohmann::json json;
        
        json["type"] = "blend_tree";
        json["version"] = GetCurrentVersion();
        json["blendType"] = static_cast<int>(blendTree.GetType());
        json["parameterX"] = blendTree.GetParameterX();
        json["parameterY"] = blendTree.GetParameterY();
        
        // Serialize blend tree nodes
        json["nodes"] = nlohmann::json::array();
        const auto& nodes = blendTree.GetNodes();
        for (const auto& node : nodes) {
            nlohmann::json nodeJson;
            nodeJson["name"] = node.name;
            nodeJson["threshold"] = node.threshold;
            nodeJson["position"] = nlohmann::json::array({node.position.x, node.position.y});
            nodeJson["weight"] = node.weight;
            
            if (node.IsAnimation() && node.animation) {
                nodeJson["nodeType"] = "animation";
                nodeJson["animation"] = SkeletalAnimationToJson(*node.animation);
            } else if (node.IsChildTree() && node.childTree) {
                nodeJson["nodeType"] = "childTree";
                nodeJson["childTree"] = BlendTreeToJson(*node.childTree);
            }
            
            json["nodes"].push_back(nodeJson);
        }
        
        return json;
    }

    std::shared_ptr<BlendTree> AnimationSerialization::JsonToBlendTree(const nlohmann::json& json) {
        auto blendType = static_cast<BlendTree::Type>(json.value("blendType", 0));
        auto blendTree = std::make_shared<BlendTree>(blendType);
        
        if (json.contains("parameterX")) {
            std::string paramX = json["parameterX"];
            if (json.contains("parameterY")) {
                std::string paramY = json["parameterY"];
                blendTree->SetParameters(paramX, paramY);
            } else {
                blendTree->SetParameter(paramX);
            }
        }
        
        // Deserialize nodes
        if (json.contains("nodes") && json["nodes"].is_array()) {
            for (const auto& nodeJson : json["nodes"]) {
                std::string name = nodeJson.value("name", "");
                float threshold = nodeJson.value("threshold", 0.0f);
                Math::Vec2 position(0.0f);
                if (nodeJson.contains("position") && nodeJson["position"].is_array() && nodeJson["position"].size() >= 2) {
                    position.x = nodeJson["position"][0];
                    position.y = nodeJson["position"][1];
                }
                
                std::string nodeType = nodeJson.value("nodeType", "");
                if (nodeType == "animation" && nodeJson.contains("animation")) {
                    auto animation = JsonToSkeletalAnimation(nodeJson["animation"]);
                    if (animation) {
                        if (blendTree->GetType() == BlendTree::Type::Simple1D) {
                            blendTree->AddMotion(animation, threshold, name);
                        } else {
                            blendTree->AddMotion(animation, position, name);
                        }
                    }
                } else if (nodeType == "childTree" && nodeJson.contains("childTree")) {
                    auto childTree = JsonToBlendTree(nodeJson["childTree"]);
                    if (childTree) {
                        if (blendTree->GetType() == BlendTree::Type::Simple1D) {
                            blendTree->AddChildBlendTree(childTree, threshold, name);
                        } else {
                            blendTree->AddChildBlendTree(childTree, position, name);
                        }
                    }
                }
            }
        }
        
        return blendTree;
    }

    // JSON conversion helpers - Transition
    nlohmann::json AnimationSerialization::TransitionToJson(const AnimationTransition& transition) {
        nlohmann::json json;
        
        json["type"] = "transition";
        json["version"] = GetCurrentVersion();
        json["toState"] = transition.GetToState();
        json["duration"] = transition.GetDuration();
        json["offset"] = transition.GetOffset();
        json["exitTime"] = transition.GetExitTime();
        json["hasExitTime"] = transition.HasExitTime();
        json["interruptionSource"] = static_cast<int>(transition.GetInterruptSource());
        json["blendMode"] = static_cast<int>(transition.GetBlendMode());
        json["logicOperator"] = static_cast<int>(transition.GetLogicOperator());
        
        // Conditions
        json["conditions"] = nlohmann::json::array();
        const auto& conditions = transition.GetConditions();
        for (const auto& condition : conditions) {
            nlohmann::json conditionJson;
            conditionJson["type"] = static_cast<int>(condition.type);
            conditionJson["parameterName"] = condition.parameterName;
            conditionJson["floatValue"] = condition.floatValue;
            conditionJson["intValue"] = condition.intValue;
            conditionJson["boolValue"] = condition.boolValue;
            conditionJson["tolerance"] = condition.tolerance;
            json["conditions"].push_back(conditionJson);
        }
        
        return json;
    }

    std::shared_ptr<AnimationTransition> AnimationSerialization::JsonToTransition(const nlohmann::json& json) {
        if (!json.contains("toState")) {
            LOG_ERROR("Transition JSON missing toState field");
            return nullptr;
        }
        
        // Create transition with placeholder fromState (will be set by caller)
        auto transition = std::make_shared<AnimationTransition>("", json["toState"]);
        
        // Basic properties
        if (json.contains("duration")) {
            transition->SetDuration(json["duration"]);
        }
        if (json.contains("offset")) {
            transition->SetOffset(json["offset"]);
        }
        if (json.contains("exitTime")) {
            transition->SetExitTime(json["exitTime"]);
        }
        if (json.contains("hasExitTime")) {
            transition->SetHasExitTime(json["hasExitTime"]);
        }
        if (json.contains("interruptionSource")) {
            transition->SetInterruptSource(static_cast<TransitionInterruptSource>(json["interruptionSource"]));
        }
        if (json.contains("blendMode")) {
            transition->SetBlendMode(static_cast<AnimationTransition::BlendMode>(json["blendMode"]));
        }
        if (json.contains("logicOperator")) {
            transition->SetLogicOperator(static_cast<TransitionLogicOperator>(json["logicOperator"]));
        }
        
        // Conditions
        if (json.contains("conditions") && json["conditions"].is_array()) {
            for (const auto& conditionJson : json["conditions"]) {
                TransitionCondition condition;
                condition.type = static_cast<TransitionConditionType>(conditionJson.value("type", 0));
                condition.parameterName = conditionJson.value("parameterName", "");
                condition.floatValue = conditionJson.value("floatValue", 0.0f);
                condition.intValue = conditionJson.value("intValue", 0);
                condition.boolValue = conditionJson.value("boolValue", false);
                condition.tolerance = conditionJson.value("tolerance", 0.001f);
                transition->AddCondition(condition);
            }
        }
        
        return transition;
    }

    // Keyframe serialization helpers
    nlohmann::json AnimationSerialization::PositionKeyframeToJson(const PositionKeyframe& keyframe) {
        nlohmann::json json;
        json["time"] = keyframe.time;
        json["value"] = Vec3ToJson(keyframe.value);
        json["interpolation"] = static_cast<int>(keyframe.interpolation);
        return json;
    }

    PositionKeyframe AnimationSerialization::JsonToPositionKeyframe(const nlohmann::json& json) {
        PositionKeyframe keyframe;
        keyframe.time = json.value("time", 0.0f);
        keyframe.value = JsonToVec3(json["value"]);
        keyframe.interpolation = static_cast<InterpolationType>(json.value("interpolation", 0));
        return keyframe;
    }

    nlohmann::json AnimationSerialization::RotationKeyframeToJson(const RotationKeyframe& keyframe) {
        nlohmann::json json;
        json["time"] = keyframe.time;
        json["value"] = QuatToJson(keyframe.value);
        json["interpolation"] = static_cast<int>(keyframe.interpolation);
        return json;
    }

    RotationKeyframe AnimationSerialization::JsonToRotationKeyframe(const nlohmann::json& json) {
        RotationKeyframe keyframe;
        keyframe.time = json.value("time", 0.0f);
        keyframe.value = JsonToQuat(json["value"]);
        keyframe.interpolation = static_cast<InterpolationType>(json.value("interpolation", 0));
        return keyframe;
    }

    nlohmann::json AnimationSerialization::ScaleKeyframeToJson(const ScaleKeyframe& keyframe) {
        nlohmann::json json;
        json["time"] = keyframe.time;
        json["value"] = Vec3ToJson(keyframe.value);
        json["interpolation"] = static_cast<int>(keyframe.interpolation);
        return json;
    }

    ScaleKeyframe AnimationSerialization::JsonToScaleKeyframe(const nlohmann::json& json) {
        ScaleKeyframe keyframe;
        keyframe.time = json.value("time", 0.0f);
        keyframe.value = JsonToVec3(json["value"]);
        keyframe.interpolation = static_cast<InterpolationType>(json.value("interpolation", 0));
        return keyframe;
    }

    // Math type serialization helpers
    nlohmann::json AnimationSerialization::Vec3ToJson(const Math::Vec3& vec) {
        return nlohmann::json::array({vec.x, vec.y, vec.z});
    }

    Math::Vec3 AnimationSerialization::JsonToVec3(const nlohmann::json& json) {
        if (json.is_array() && json.size() >= 3) {
            return Math::Vec3(json[0], json[1], json[2]);
        }
        return Math::Vec3(0.0f);
    }

    nlohmann::json AnimationSerialization::QuatToJson(const Math::Quat& quat) {
        return nlohmann::json::array({quat.x, quat.y, quat.z, quat.w});
    }

    Math::Quat AnimationSerialization::JsonToQuat(const nlohmann::json& json) {
        if (json.is_array() && json.size() >= 4) {
            return Math::Quat(json[3], json[0], json[1], json[2]); // w, x, y, z
        }
        return Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
    }

    // Utility functions
    bool AnimationSerialization::WriteJsonToFile(const nlohmann::json& json, const std::string& filepath) {
        try {
            std::ofstream file(filepath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file for writing: " + filepath);
                return false;
            }
            
            file << json.dump(4);
            return file.good();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to write JSON to file: " + std::string(e.what()));
            return false;
        }
    }

    nlohmann::json AnimationSerialization::ReadJsonFromFile(const std::string& filepath) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file for reading: " + filepath);
                return nlohmann::json{};
            }
            
            nlohmann::json json;
            file >> json;
            return json;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to read JSON from file: " + std::string(e.what()));
            return nlohmann::json{};
        }
    }

    uint64_t AnimationSerialization::GetCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

#else
    // Fallback implementations when JSON is not available
    std::string AnimationSerialization::SerializeSkeletalAnimation(const SkeletalAnimation& animation) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return "";
    }

    std::shared_ptr<SkeletalAnimation> AnimationSerialization::DeserializeSkeletalAnimation(const std::string& jsonData) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return nullptr;
    }

    std::string AnimationSerialization::SerializeStateMachine(const AnimationStateMachine& stateMachine) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return "";
    }

    std::shared_ptr<AnimationStateMachine> AnimationSerialization::DeserializeStateMachine(const std::string& jsonData) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return nullptr;
    }

    std::string AnimationSerialization::SerializeBlendTree(const BlendTree& blendTree) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return "";
    }

    std::shared_ptr<BlendTree> AnimationSerialization::DeserializeBlendTree(const std::string& jsonData) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return nullptr;
    }

    std::string AnimationSerialization::SerializeTransition(const AnimationTransition& transition) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return "";
    }

    std::shared_ptr<AnimationTransition> AnimationSerialization::DeserializeTransition(const std::string& jsonData) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return nullptr;
    }

    bool AnimationSerialization::SaveAnimationToFile(const SkeletalAnimation& animation, const std::string& filepath) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return false;
    }

    std::shared_ptr<SkeletalAnimation> AnimationSerialization::LoadAnimationFromFile(const std::string& filepath) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return nullptr;
    }

    bool AnimationSerialization::SaveStateMachineToFile(const AnimationStateMachine& stateMachine, const std::string& filepath) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return false;
    }

    std::shared_ptr<AnimationStateMachine> AnimationSerialization::LoadStateMachineFromFile(const std::string& filepath) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return nullptr;
    }

    bool AnimationSerialization::SaveBlendTreeToFile(const BlendTree& blendTree, const std::string& filepath) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return false;
    }

    std::shared_ptr<BlendTree> AnimationSerialization::LoadBlendTreeFromFile(const std::string& filepath) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return nullptr;
    }

    std::string AnimationSerialization::SerializeAnimationAsset(const AnimationAsset& asset) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return "";
    }

    AnimationSerialization::AnimationAsset AnimationSerialization::DeserializeAnimationAsset(const std::string& jsonData) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return AnimationAsset{};
    }

    bool AnimationSerialization::SaveAnimationAsset(const AnimationAsset& asset, const std::string& filepath) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return false;
    }

    AnimationSerialization::AnimationAsset AnimationSerialization::LoadAnimationAsset(const std::string& filepath) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return AnimationAsset{};
    }

    std::string AnimationSerialization::SerializeAnimationCollection(const AnimationCollection& collection) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return "";
    }

    AnimationSerialization::AnimationCollection AnimationSerialization::DeserializeAnimationCollection(const std::string& jsonData) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return AnimationCollection{};
    }

    bool AnimationSerialization::SaveAnimationCollection(const AnimationCollection& collection, const std::string& filepath) {
        LOG_ERROR("JSON serialization not available - nlohmann/json not found");
        return false;
    }

    AnimationSerialization::AnimationCollection AnimationSerialization::LoadAnimationCollection(const std::string& filepath) {
        LOG_ERROR("JSON deserialization not available - nlohmann/json not found");
        return AnimationCollection{};
    }

    bool AnimationSerialization::ValidateAnimationData(const std::string& jsonData, const std::string& expectedType) {
        LOG_ERROR("JSON validation not available - nlohmann/json not found");
        return false;
    }

#endif

} // namespace Animation
} // namespace GameEngine