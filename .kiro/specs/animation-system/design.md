# Design Document - Animation System

## Overview

This design document outlines the implementation of a comprehensive animation system for Game Engine Kiro v1.1. The system provides skeletal animation, state machines, blend trees, inverse kinematics, morph targets, and advanced animation techniques for creating lifelike character movement and object animations.

## Architecture

### Modular Animation System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Project Layer                            │
│              (projects/GameExample/src/)                    │
├─────────────────────────────────────────────────────────────┤
│  XBotCharacter  │  PlayerCharacter  │  NPCCharacter       │
│  (GameExample)  │  (UserProject)    │  (UserProject)      │
├─────────────────────────────────────────────────────────────┤
│                   Base Character                            │
│              (include/Game/Character.h)                     │
│              (Generic Animation Interface)                  │
├─────────────────────────────────────────────────────────────┤
│                   Animation Controller                      │
│              (include/Animation/)                           │
├─────────────────────────────────────────────────────────────┤
│ State Machine │ Blend Trees │ Parameters │ Event System    │
├─────────────────────────────────────────────────────────────┤
│                    Animation Player                         │
├─────────────────────────────────────────────────────────────┤
│  Skeleton  │  Animation  │  Blending  │  Interpolation     │
├─────────────────────────────────────────────────────────────┤
│                    Animation Data                           │
├─────────────────────────────────────────────────────────────┤
│   Bones    │   Keyframes │   Curves   │   Morph Targets    │
├─────────────────────────────────────────────────────────────┤
│                  Procedural Animation                       │
├─────────────────────────────────────────────────────────────┤
│     IK     │  Look-At   │  Constraints │  Physics Blend    │
├─────────────────────────────────────────────────────────────┤
│                   Engine Integration                        │
└─────────────────────────────────────────────────────────────┘
│ Graphics │ Physics │ Audio │ Resource │ Model Loading      │
└─────────────────────────────────────────────────────────────┘
```

### Modular Design Principles

1. **Base Character Class**: Provides generic animation interface without specific asset knowledge (engine/include/Game/Character.h)
2. **Project-Specific Characters**: Extend base Character with custom animation logic and assets (projects/[ProjectName]/src/)
3. **Asset Isolation**: Each project manages its own animation assets in projects/[ProjectName]/assets/
4. **Engine Agnostic**: Base engine remains completely unaware of specific character implementations
5. **Clear Separation**: Engine code in include/src/, project code in projects/[ProjectName]/
6. **Modular Structure**: Projects can be developed independently without affecting engine or other projects

## Components and Interfaces

### 1. AnimationController

```cpp
class AnimationController {
public:
    // Lifecycle
    AnimationController();
    ~AnimationController();
    bool Initialize(std::shared_ptr<Skeleton> skeleton);
    void Shutdown();

    // State machine management
    void SetStateMachine(std::shared_ptr<AnimationStateMachine> stateMachine);
    std::shared_ptr<AnimationStateMachine> GetStateMachine() const;

    // Parameter system
    void SetFloat(const std::string& name, float value);
    void SetInt(const std::string& name, int value);
    void SetBool(const std::string& name, bool value);
    void SetTrigger(const std::string& name);

    float GetFloat(const std::string& name) const;
    int GetInt(const std::string& name) const;
    bool GetBool(const std::string& name) const;
    bool GetTrigger(const std::string& name) const;

    // Animation control
    void Play(const std::string& animationName, float fadeTime = 0.3f);
    void Stop(const std::string& animationName, float fadeTime = 0.3f);
    void Pause();
    void Resume();
    void SetPlaybackSpeed(float speed);

    // Update and evaluation
    void Update(float deltaTime);
    void Evaluate(std::vector<Math::Mat4>& boneMatrices);

    // Events
    void SetEventCallback(std::function<void(const AnimationEvent&)> callback);

    // Debugging
    AnimationControllerDebugInfo GetDebugInfo() const;
    void SetDebugVisualization(bool enabled);

private:
    std::shared_ptr<AnimationStateMachine> m_stateMachine;
    std::shared_ptr<AnimationPlayer> m_player;
    std::shared_ptr<Skeleton> m_skeleton;

    std::unordered_map<std::string, AnimationParameter> m_parameters;
    std::function<void(const AnimationEvent&)> m_eventCallback;

    float m_playbackSpeed = 1.0f;
    bool m_isPaused = false;
    bool m_debugVisualization = false;
};
```

### 2. Skeleton System

```cpp
class Skeleton {
public:
    struct Bone {
        std::string name;
        int parentIndex = -1;
        Math::Mat4 bindPose;
        Math::Mat4 inverseBindPose;
        std::vector<int> childIndices;

        // Runtime data
        Math::Mat4 localTransform = Math::Mat4(1.0f);
        Math::Mat4 worldTransform = Math::Mat4(1.0f);
        Math::Mat4 skinningMatrix = Math::Mat4(1.0f);
    };

    // Lifecycle
    Skeleton();
    ~Skeleton();

    // Bone management
    int AddBone(const std::string& name, int parentIndex, const Math::Mat4& bindPose);
    const Bone& GetBone(int index) const;
    Bone& GetBone(int index);
    int GetBoneIndex(const std::string& name) const;
    int GetBoneCount() const;

    // Hierarchy queries
    std::vector<int> GetRootBones() const;
    std::vector<int> GetChildBones(int boneIndex) const;
    bool IsAncestor(int ancestorIndex, int descendantIndex) const;
    int GetParent(int boneIndex) const;

    // Transform calculations
    void UpdateBoneTransforms();
    void CalculateSkinningMatrices();
    void SetBoneLocalTransform(int boneIndex, const Math::Mat4& transform);
    Math::Mat4 GetBoneWorldTransform(int boneIndex) const;

    // Pose management
    void SetBindPose();
    void ApplyPose(const AnimationPose& pose);
    AnimationPose GetCurrentPose() const;

    // Validation and debugging
    bool Validate() const;
    void PrintHierarchy() const;
    SkeletonStats GetStats() const;

private:
    std::vector<Bone> m_bones;
    std::unordered_map<std::string, int> m_boneNameToIndex;
    std::vector<Math::Mat4> m_skinningMatrices;

    void UpdateBoneWorldTransform(int boneIndex, const Math::Mat4& parentTransform = Math::Mat4(1.0f));
    void ValidateBoneHierarchy() const;
};
```

### 3. Animation Data Structures

```cpp
class Animation {
public:
    struct BoneTrack {
        int boneIndex;
        std::vector<PositionKey> positionKeys;
        std::vector<RotationKey> rotationKeys;
        std::vector<ScaleKey> scaleKeys;

        // Optimization
        bool hasPosition = true;
        bool hasRotation = true;
        bool hasScale = true;
    };

    // Lifecycle
    Animation(const std::string& name);
    ~Animation();

    // Properties
    void SetName(const std::string& name);
    void SetDuration(float duration);
    void SetFrameRate(float frameRate);
    void SetLooping(bool looping);

    const std::string& GetName() const;
    float GetDuration() const;
    float GetFrameRate() const;
    bool IsLooping() const;

    // Track management
    void AddBoneTrack(const BoneTrack& track);
    const BoneTrack* GetBoneTrack(int boneIndex) const;
    std::vector<BoneTrack> GetBoneTracks() const;
    void RemoveBoneTrack(int boneIndex);

    // Sampling
    Math::Mat4 SampleBoneTransform(int boneIndex, float time) const;
    void SamplePose(float time, AnimationPose& pose) const;
    AnimationPose SamplePose(float time) const;

    // Events
    void AddEvent(const AnimationEvent& event);
    void RemoveEvent(const std::string& eventName, float time);
    std::vector<AnimationEvent> GetEventsInRange(float startTime, float endTime) const;
    std::vector<AnimationEvent> GetAllEvents() const;

    // Optimization
    void OptimizeKeyframes(float tolerance = 0.001f);
    void CompressAnimation();
    void RemoveRedundantKeys();

    // Statistics
    AnimationStats GetStats() const;
    size_t GetMemoryUsage() const;

private:
    std::string m_name;
    float m_duration = 0.0f;
    float m_frameRate = 30.0f;
    bool m_looping = true;

    std::vector<BoneTrack> m_boneTracks;
    std::unordered_map<int, size_t> m_boneToTrackIndex;
    std::vector<AnimationEvent> m_events;

    // Helper methods
    Math::Vec3 InterpolatePosition(const std::vector<PositionKey>& keys, float time) const;
    Math::Quat InterpolateRotation(const std::vector<RotationKey>& keys, float time) const;
    Math::Vec3 InterpolateScale(const std::vector<ScaleKey>& keys, float time) const;

    template<typename T>
    T InterpolateKeys(const std::vector<T>& keys, float time) const;
};

struct PositionKey {
    float time;
    Math::Vec3 position;

    // Interpolation data
    Math::Vec3 inTangent;
    Math::Vec3 outTangent;
    InterpolationType interpolation = InterpolationType::Linear;
};

struct RotationKey {
    float time;
    Math::Quat rotation;

    InterpolationType interpolation = InterpolationType::Linear;
};

struct ScaleKey {
    float time;
    Math::Vec3 scale;

    InterpolationType interpolation = InterpolationType::Linear;
};
```

### 4. Animation State Machine

```cpp
class AnimationStateMachine {
public:
    // Lifecycle
    AnimationStateMachine();
    ~AnimationStateMachine();

    // State management
    void AddState(std::shared_ptr<AnimationState> state);
    void RemoveState(const std::string& name);
    std::shared_ptr<AnimationState> GetState(const std::string& name) const;
    std::vector<std::shared_ptr<AnimationState>> GetAllStates() const;

    // Transitions
    void AddTransition(const std::string& fromState, const std::string& toState,
                      std::shared_ptr<AnimationTransition> transition);
    void RemoveTransition(const std::string& fromState, const std::string& toState);
    std::vector<std::shared_ptr<AnimationTransition>> GetTransitions(const std::string& fromState) const;

    // Entry and default states
    void SetEntryState(const std::string& name);
    void SetDefaultState(const std::string& name);
    const std::string& GetEntryState() const;

    // Execution
    void Start();
    void Update(float deltaTime, AnimationController* controller);
    void Stop();

    // Current state
    std::shared_ptr<AnimationState> GetCurrentState() const;
    std::string GetCurrentStateName() const;
    float GetCurrentStateTime() const;

    // Parameters
    void SetParameter(const std::string& name, const AnimationParameter& value);
    AnimationParameter GetParameter(const std::string& name) const;

    // Debugging
    StateMachineDebugInfo GetDebugInfo() const;

private:
    std::unordered_map<std::string, std::shared_ptr<AnimationState>> m_states;
    std::unordered_map<std::string, std::vector<std::shared_ptr<AnimationTransition>>> m_transitions;

    std::shared_ptr<AnimationState> m_currentState;
    std::shared_ptr<AnimationTransition> m_activeTransition;

    std::string m_entryState;
    std::string m_defaultState;
    float m_currentStateTime = 0.0f;
    float m_transitionTime = 0.0f;

    std::unordered_map<std::string, AnimationParameter> m_parameters;

    void EvaluateTransitions(AnimationController* controller);
    void ProcessTransition(float deltaTime, AnimationController* controller);
};

class AnimationState {
public:
    enum class Type { Single, BlendTree, SubStateMachine };

    // Lifecycle
    AnimationState(const std::string& name, Type type = Type::Single);
    ~AnimationState();

    // Properties
    void SetName(const std::string& name);
    void SetType(Type type);
    void SetSpeed(float speed);
    void SetLooping(bool looping);

    const std::string& GetName() const;
    Type GetType() const;
    float GetSpeed() const;
    bool IsLooping() const;

    // Single animation state
    void SetAnimation(std::shared_ptr<Animation> animation);
    std::shared_ptr<Animation> GetAnimation() const;

    // Blend tree state
    void SetBlendTree(std::shared_ptr<BlendTree> blendTree);
    std::shared_ptr<BlendTree> GetBlendTree() const;

    // Sub-state machine
    void SetSubStateMachine(std::shared_ptr<AnimationStateMachine> subStateMachine);
    std::shared_ptr<AnimationStateMachine> GetSubStateMachine() const;

    // Events
    void AddEvent(const AnimationEvent& event);
    void RemoveEvent(const std::string& eventName);
    std::vector<AnimationEvent> GetEvents() const;

    // Execution
    void OnEnter(AnimationController* controller);
    void OnUpdate(float deltaTime, AnimationController* controller);
    void OnExit(AnimationController* controller);

    // Pose evaluation
    void EvaluatePose(float time, AnimationPose& pose, AnimationController* controller) const;

private:
    std::string m_name;
    Type m_type;
    float m_speed = 1.0f;
    bool m_looping = true;

    std::shared_ptr<Animation> m_animation;
    std::shared_ptr<BlendTree> m_blendTree;
    std::shared_ptr<AnimationStateMachine> m_subStateMachine;

    std::vector<AnimationEvent> m_events;
};
```

### 5. Blend Tree System

```cpp
class BlendTree {
public:
    enum class Type { Simple1D, SimpleDirectional2D, FreeformDirectional2D, FreeformCartesian2D };

    // Lifecycle
    BlendTree(Type type = Type::Simple1D);
    ~BlendTree();

    // Properties
    void SetType(Type type);
    void SetParameter(const std::string& parameter);
    void SetParameters(const std::string& paramX, const std::string& paramY);

    Type GetType() const;
    const std::string& GetParameterX() const;
    const std::string& GetParameterY() const;

    // Motion management
    void AddMotion(std::shared_ptr<Animation> animation, float threshold);
    void AddMotion(std::shared_ptr<Animation> animation, const Math::Vec2& position);
    void AddChildBlendTree(std::shared_ptr<BlendTree> childTree, float threshold);
    void AddChildBlendTree(std::shared_ptr<BlendTree> childTree, const Math::Vec2& position);
    void RemoveMotion(std::shared_ptr<Animation> animation);

    // Evaluation
    void Evaluate(AnimationController* controller, AnimationPose& pose, float time) const;
    std::vector<AnimationSample> GetAnimationSamples(AnimationController* controller, float time) const;

    // Validation
    bool Validate() const;
    std::vector<std::string> GetValidationErrors() const;

private:
    struct BlendTreeNode {
        std::shared_ptr<Animation> animation;
        std::shared_ptr<BlendTree> childTree;
        float threshold = 0.0f;
        Math::Vec2 position = Math::Vec2(0.0f);
        float weight = 0.0f;
        std::string name;
    };

    Type m_type;
    std::string m_parameterX;
    std::string m_parameterY;
    std::vector<BlendTreeNode> m_nodes;

    void CalculateWeights1D(float parameter, std::vector<float>& weights) const;
    void CalculateWeights2D(const Math::Vec2& parameter, std::vector<float>& weights) const;
    void CalculateDirectionalWeights(const Math::Vec2& direction, std::vector<float>& weights) const;
    void CalculateCartesianWeights(const Math::Vec2& position, std::vector<float>& weights) const;
};

struct AnimationSample {
    std::shared_ptr<Animation> animation;
    float weight;
    float time;

    bool IsValid() const { return animation != nullptr && weight > 0.0f; }
};
```

### 6. Inverse Kinematics System

```cpp
class IKSolver {
public:
    enum class Type { TwoBone, FABRIK, CCD };

    // Lifecycle
    IKSolver(Type type = Type::TwoBone);
    virtual ~IKSolver() = default;

    // Chain setup
    void SetChain(const std::vector<int>& boneIndices);
    void SetTarget(const Math::Vec3& position, const Math::Quat& rotation = Math::Quat::Identity());
    void SetPoleTarget(const Math::Vec3& position);

    // Constraints
    void SetBoneConstraints(int boneIndex, float minAngle, float maxAngle);
    void SetChainLength(float length);
    void SetIterations(int iterations);
    void SetTolerance(float tolerance);

    // Solving
    virtual bool Solve(Skeleton& skeleton) = 0;
    bool IsTargetReachable(const Skeleton& skeleton) const;

    // Properties
    Type GetType() const { return m_type; }
    const std::vector<int>& GetChain() const { return m_boneChain; }
    const Math::Vec3& GetTarget() const { return m_targetPosition; }

protected:
    Type m_type;
    std::vector<int> m_boneChain;
    Math::Vec3 m_targetPosition;
    Math::Quat m_targetRotation = Math::Quat::Identity();
    Math::Vec3 m_poleTarget;

    int m_iterations = 10;
    float m_tolerance = 0.01f;
    float m_chainLength = 0.0f;

    std::unordered_map<int, std::pair<float, float>> m_boneConstraints;

    float CalculateChainLength(const Skeleton& skeleton) const;
    void ApplyBoneConstraints(Skeleton& skeleton, int boneIndex, const Math::Quat& rotation) const;
};

class TwoBoneIK : public IKSolver {
public:
    TwoBoneIK();

    void SetUpperBone(int boneIndex);
    void SetLowerBone(int boneIndex);
    void SetEndEffector(int boneIndex);

    bool Solve(Skeleton& skeleton) override;

private:
    int m_upperBone = -1;
    int m_lowerBone = -1;
    int m_endEffector = -1;

    void SolveTwoBoneIK(Skeleton& skeleton);
};

class FABRIKIK : public IKSolver {
public:
    FABRIKIK();

    bool Solve(Skeleton& skeleton) override;

private:
    void ForwardReach(Skeleton& skeleton);
    void BackwardReach(Skeleton& skeleton);
};
```

### 7. Morph Target System

```cpp
class MorphTarget {
public:
    // Lifecycle
    MorphTarget(const std::string& name);
    ~MorphTarget();

    // Vertex data
    void SetVertexDeltas(const std::vector<Math::Vec3>& positionDeltas);
    void SetNormalDeltas(const std::vector<Math::Vec3>& normalDeltas);
    void SetTangentDeltas(const std::vector<Math::Vec3>& tangentDeltas);

    const std::vector<Math::Vec3>& GetVertexDeltas() const;
    const std::vector<Math::Vec3>& GetNormalDeltas() const;
    const std::vector<Math::Vec3>& GetTangentDeltas() const;

    // Properties
    void SetName(const std::string& name);
    void SetWeight(float weight);
    float GetWeight() const;
    const std::string& GetName() const;

    // Application
    void ApplyToMesh(Mesh& mesh, float weight) const;
    void ApplyToVertices(std::vector<Vertex>& vertices, float weight) const;

    // Optimization
    void Compress(float tolerance = 0.001f);
    size_t GetMemoryUsage() const;

private:
    std::string m_name;
    float m_weight = 0.0f;

    std::vector<Math::Vec3> m_positionDeltas;
    std::vector<Math::Vec3> m_normalDeltas;
    std::vector<Math::Vec3> m_tangentDeltas;

    // Sparse representation for optimization
    std::vector<uint32_t> m_affectedVertices;
    bool m_isCompressed = false;
};

class MorphTargetController {
public:
    // Lifecycle
    MorphTargetController();
    ~MorphTargetController();

    // Morph target management
    void AddMorphTarget(std::shared_ptr<MorphTarget> morphTarget);
    void RemoveMorphTarget(const std::string& name);
    std::shared_ptr<MorphTarget> GetMorphTarget(const std::string& name) const;
    std::vector<std::shared_ptr<MorphTarget>> GetAllMorphTargets() const;

    // Weight control
    void SetWeight(const std::string& name, float weight);
    float GetWeight(const std::string& name) const;
    void SetAllWeights(const std::unordered_map<std::string, float>& weights);

    // Animation
    void AnimateWeight(const std::string& name, float targetWeight, float duration);
    void Update(float deltaTime);

    // Application
    void ApplyToMesh(Mesh& mesh) const;
    void ApplyToVertices(std::vector<Vertex>& vertices) const;

    // Statistics
    size_t GetMorphTargetCount() const;
    size_t GetMemoryUsage() const;

private:
    std::unordered_map<std::string, std::shared_ptr<MorphTarget>> m_morphTargets;
    std::unordered_map<std::string, float> m_targetWeights;
    std::unordered_map<std::string, float> m_animationSpeeds;
    std::unordered_map<std::string, float> m_animationTargets;
};
```

## Data Models

### Animation Parameters

```cpp
class AnimationParameter {
public:
    enum class Type { Float, Int, Bool, Trigger };

    AnimationParameter() = default;
    AnimationParameter(float value);
    AnimationParameter(int value);
    AnimationParameter(bool value);

    Type GetType() const;

    float AsFloat() const;
    int AsInt() const;
    bool AsBool() const;
    bool IsTrigger() const;

    void SetFloat(float value);
    void SetInt(int value);
    void SetBool(bool value);
    void SetTrigger();
    void ResetTrigger();

private:
    Type m_type = Type::Float;
    std::variant<float, int, bool> m_value;
    bool m_triggerState = false;
};
```

### Animation Events

```cpp
struct AnimationEvent {
    std::string name;
    float time;  // Normalized time (0-1)
    std::string stringParameter;
    float floatParameter = 0.0f;
    int intParameter = 0;
    bool boolParameter = false;

    enum class Type { Generic, Sound, Effect, Footstep, Custom };
    Type type = Type::Generic;

    bool IsValid() const;
    nlohmann::json Serialize() const;
    bool Deserialize(const nlohmann::json& json);
};
```

## Testing Strategy

### Unit Testing

```cpp
// Test skeleton hierarchy
bool TestSkeletonHierarchy() {
    TestOutput::PrintTestStart("skeleton hierarchy");

    Skeleton skeleton;

    // Create simple bone hierarchy
    int rootBone = skeleton.AddBone("root", -1, Math::Mat4(1.0f));
    int childBone = skeleton.AddBone("child", rootBone, Math::Mat4(1.0f));
    int grandChildBone = skeleton.AddBone("grandchild", childBone, Math::Mat4(1.0f));

    EXPECT_EQUAL(skeleton.GetBoneCount(), 3);
    EXPECT_EQUAL(skeleton.GetParent(childBone), rootBone);
    EXPECT_EQUAL(skeleton.GetParent(grandChildBone), childBone);

    auto rootBones = skeleton.GetRootBones();
    EXPECT_EQUAL(rootBones.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(rootBones[0], rootBone);

    TestOutput::PrintTestPass("skeleton hierarchy");
    return true;
}

// Test animation sampling
bool TestAnimationSampling() {
    TestOutput::PrintTestStart("animation sampling");

    Animation animation("test_animation");
    animation.SetDuration(2.0f);

    // Create simple animation track
    Animation::BoneTrack track;
    track.boneIndex = 0;
    track.positionKeys.push_back({0.0f, Math::Vec3(0.0f, 0.0f, 0.0f)});
    track.positionKeys.push_back({1.0f, Math::Vec3(1.0f, 0.0f, 0.0f)});
    track.positionKeys.push_back({2.0f, Math::Vec3(2.0f, 0.0f, 0.0f)});

    animation.AddBoneTrack(track);

    // Test sampling at different times
    Math::Mat4 transform0 = animation.SampleBoneTransform(0, 0.0f);
    Math::Mat4 transform1 = animation.SampleBoneTransform(0, 1.0f);
    Math::Mat4 transform2 = animation.SampleBoneTransform(0, 2.0f);

    Math::Vec3 pos0 = Math::Vec3(transform0[3]);
    Math::Vec3 pos1 = Math::Vec3(transform1[3]);
    Math::Vec3 pos2 = Math::Vec3(transform2[3]);

    EXPECT_NEAR_VEC3(pos0, Math::Vec3(0.0f, 0.0f, 0.0f));
    EXPECT_NEAR_VEC3(pos1, Math::Vec3(1.0f, 0.0f, 0.0f));
    EXPECT_NEAR_VEC3(pos2, Math::Vec3(2.0f, 0.0f, 0.0f));

    TestOutput::PrintTestPass("animation sampling");
    return true;
}

// Test blend tree evaluation
bool TestBlendTreeEvaluation() {
    TestOutput::PrintTestStart("blend tree evaluation");

    auto blendTree = std::make_shared<BlendTree>(BlendTree::Type::Simple1D);
    blendTree->SetParameter("Speed");

    auto idleAnim = std::make_shared<Animation>("idle");
    auto walkAnim = std::make_shared<Animation>("walk");
    auto runAnim = std::make_shared<Animation>("run");

    blendTree->AddMotion(idleAnim, 0.0f);
    blendTree->AddMotion(walkAnim, 2.0f);
    blendTree->AddMotion(runAnim, 6.0f);

    EXPECT_TRUE(blendTree->Validate());

    TestOutput::PrintTestPass("blend tree evaluation");
    return true;
}
```

### Integration Testing

```cpp
// Test animation controller with state machine
bool TestAnimationControllerStateMachine() {
    TestOutput::PrintTestStart("animation controller state machine");

    auto skeleton = std::make_shared<Skeleton>();
    skeleton->AddBone("root", -1, Math::Mat4(1.0f));

    AnimationController controller;
    EXPECT_TRUE(controller.Initialize(skeleton));

    auto stateMachine = std::make_shared<AnimationStateMachine>();

    auto idleState = std::make_shared<AnimationState>("Idle");
    auto walkState = std::make_shared<AnimationState>("Walk");

    stateMachine->AddState(idleState);
    stateMachine->AddState(walkState);
    stateMachine->SetEntryState("Idle");

    controller.SetStateMachine(stateMachine);

    // Test parameter setting
    controller.SetFloat("Speed", 0.0f);
    EXPECT_NEARLY_EQUAL(controller.GetFloat("Speed"), 0.0f);

    TestOutput::PrintTestPass("animation controller state machine");
    return true;
}
```

## Implementation Phases

### Phase 1: Core Animation Foundation

- Skeleton class with bone hierarchy
- Animation class with keyframe data
- Basic animation sampling and interpolation
- AnimationController foundation

### Phase 2: State Machine System

- AnimationStateMachine implementation
- AnimationState with different types
- Transition system with conditions
- Parameter system for state control

### Phase 3: Blend Tree System

- BlendTree implementation with 1D and 2D support
- Animation blending algorithms
- Blend tree evaluation and optimization
- Integration with state machine

### Phase 4: Inverse Kinematics

- IKSolver base class and implementations
- Two-bone IK for arms and legs
- FABRIK implementation for complex chains
- IK/FK blending system

### Phase 5: Morph Target System

- MorphTarget class with vertex deltas
- MorphTargetController for weight management
- Morph target animation and blending
- Integration with mesh rendering

### Phase 6: Advanced Features and Optimization

- Animation compression and optimization
- Event system implementation
- Performance profiling and optimization
- Comprehensive testing and debugging tools

This design provides a comprehensive foundation for character animation in Game Engine Kiro, enabling developers to create sophisticated animation behaviors while maintaining performance and ease of use.
