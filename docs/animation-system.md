# Animation System

Game Engine Kiro v1.1 introduces a comprehensive animation system supporting skeletal animation, blend trees, state machines, and advanced animation techniques for creating lifelike character movement and object animations.

## 🔄 Important: Recent Namespace Changes

**Note**: The animation system classes have been renamed to resolve namespace conflicts:

- `GameEngine::Animation::Animation` → `GameEngine::Animation::SkeletalAnimation`
- `GameEngine::Animation::Skeleton` → `GameEngine::Animation::AnimationSkeleton`

All code examples in this document use the new class names. For migration information, see the [Migration Guide](namespace-class-conflict-resolution-migration-guide.md).

## 🎯 Overview

The Animation System provides a complete solution for animating 3D models, characters, and objects with support for skeletal animation, morph targets, procedural animation, and complex animation blending and state management.

### Key Features

- **Skeletal Animation**: Bone-based character animation
- **Animation Blending**: Smooth transitions between animations
- **State Machines**: Complex animation logic and transitions
- **Blend Trees**: Multi-dimensional animation blending
- **Morph Targets**: Facial animation and shape blending
- **Procedural Animation**: Runtime-generated animations
- **Animation Events**: Callback system for animation-driven events

## 🏗️ Architecture Overview

### Animation System Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                   Animation Controller                      │
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
└─────────────────────────────────────────────────────────────┘
```

### Core Components

**AnimationController**

```cpp
class AnimationController {
public:
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

    // Animation control
    void Play(const std::string& animationName, float fadeTime = 0.3f);
    void Stop(const std::string& animationName, float fadeTime = 0.3f);
    void Pause();
    void Resume();

    // Update
    void Update(float deltaTime);

    // Events
    void SetEventCallback(std::function<void(const AnimationEvent&)> callback);

private:
    std::shared_ptr<AnimationStateMachine> m_stateMachine;
    std::shared_ptr<AnimationPlayer> m_player;
    std::unordered_map<std::string, AnimationParameter> m_parameters;
    std::function<void(const AnimationEvent&)> m_eventCallback;
};
```

**AnimationPlayer**

```cpp
class AnimationPlayer {
public:
    // Animation playback
    void PlayAnimation(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation, float weight = 1.0f, float fadeTime = 0.0f);
    void StopAnimation(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation, float fadeTime = 0.3f);
    void CrossFade(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> from, std::shared_ptr<GameEngine::Animation::SkeletalAnimation> to, float duration);

    // Blending
    void SetAnimationWeight(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation, float weight);
    float GetAnimationWeight(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation) const;

    // Playback control
    void SetPlaybackSpeed(float speed);
    void SetTime(float time);
    float GetTime() const;
    float GetNormalizedTime() const;

    // Skeleton binding
    void SetSkeleton(std::shared_ptr<GameEngine::Animation::AnimationSkeleton> skeleton);
    std::shared_ptr<GameEngine::Animation::AnimationSkeleton> GetSkeleton() const;

    // Update and evaluation
    void Update(float deltaTime);
    void Evaluate(float time, std::vector<Math::Mat4>& boneMatrices);

private:
    struct PlayingAnimation {
        std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation;
        float time = 0.0f;
        float weight = 1.0f;
        float fadeTarget = 1.0f;
        float fadeSpeed = 0.0f;
        bool isPlaying = true;
    };

    std::vector<PlayingAnimation> m_playingAnimations;
    std::shared_ptr<GameEngine::Animation::AnimationSkeleton> m_skeleton;
    float m_playbackSpeed = 1.0f;
};
```

## 🦴 Skeletal Animation System

### AnimationSkeleton Structure

```cpp
class GameEngine::Animation::AnimationSkeleton {
public:
    struct Bone {
        std::string name;
        int parentIndex = -1;
        Math::Mat4 bindPose;
        Math::Mat4 inverseBindPose;
        std::vector<int> childIndices;
    };

    // Bone management
    int AddBone(const std::string& name, int parentIndex, const Math::Mat4& bindPose);
    const Bone& GetBone(int index) const;
    int GetBoneIndex(const std::string& name) const;
    int GetBoneCount() const;

    // Hierarchy
    std::vector<int> GetRootBones() const;
    std::vector<int> GetChildBones(int boneIndex) const;
    bool IsAncestor(int ancestorIndex, int descendantIndex) const;

    // Pose calculation
    void CalculateBoneMatrices(const std::vector<Math::Mat4>& localTransforms, std::vector<Math::Mat4>& boneMatrices) const;
    Math::Mat4 GetBoneWorldMatrix(int boneIndex, const std::vector<Math::Mat4>& localTransforms) const;

private:
    std::vector<Bone> m_bones;
    std::unordered_map<std::string, int> m_boneNameToIndex;
};
```

### Animation Data

```cpp
class GameEngine::Animation::SkeletalAnimation {
public:
    struct BoneTrack {
        int boneIndex;
        std::vector<PositionKey> positionKeys;
        std::vector<RotationKey> rotationKeys;
        std::vector<ScaleKey> scaleKeys;
    };

    // Animation properties
    void SetName(const std::string& name);
    void SetDuration(float duration);
    void SetFrameRate(float frameRate);
    void SetLooping(bool looping);

    // Track management
    void AddBoneTrack(const BoneTrack& track);
    const BoneTrack* GetBoneTrack(int boneIndex) const;
    std::vector<BoneTrack> GetBoneTracks() const;

    // Sampling
    Math::Mat4 SampleBoneTransform(int boneIndex, float time) const;
    void SamplePose(float time, std::vector<Math::Mat4>& localTransforms) const;

    // Events
    void AddEvent(const AnimationEvent& event);
    std::vector<AnimationEvent> GetEventsInRange(float startTime, float endTime) const;

    // Optimization
    void OptimizeKeyframes(float tolerance = 0.001f);
    void CompressAnimation();

private:
    std::string m_name;
    float m_duration = 0.0f;
    float m_frameRate = 30.0f;
    bool m_looping = true;
    std::vector<BoneTrack> m_boneTracks;
    std::vector<AnimationEvent> m_events;
};

struct PositionKey {
    float time;
    Math::Vec3 position;
};

struct RotationKey {
    float time;
    Math::Quat rotation;
};

struct ScaleKey {
    float time;
    Math::Vec3 scale;
};
```

## 🎭 Animation State Machine

### State Machine Structure

```cpp
class AnimationStateMachine {
public:
    // State management
    void AddState(std::shared_ptr<AnimationState> state);
    void RemoveState(const std::string& name);
    std::shared_ptr<AnimationState> GetState(const std::string& name) const;

    // Transitions
    void AddTransition(const std::string& fromState, const std::string& toState, std::shared_ptr<AnimationTransition> transition);
    void RemoveTransition(const std::string& fromState, const std::string& toState);

    // Entry and default states
    void SetEntryState(const std::string& name);
    void SetDefaultState(const std::string& name);

    // Execution
    void Start();
    void Update(float deltaTime, AnimationController* controller);

    // Current state
    std::shared_ptr<AnimationState> GetCurrentState() const;
    std::string GetCurrentStateName() const;

private:
    std::unordered_map<std::string, std::shared_ptr<AnimationState>> m_states;
    std::unordered_map<std::string, std::vector<std::shared_ptr<AnimationTransition>>> m_transitions;
    std::shared_ptr<AnimationState> m_currentState;
    std::string m_entryState;
    std::string m_defaultState;
};
```

### Animation States

```cpp
class AnimationState {
public:
    enum class Type { Single, BlendTree, SubStateMachine };

    // State properties
    void SetName(const std::string& name);
    void SetType(Type type);
    void SetSpeed(float speed);
    void SetLooping(bool looping);

    // Single animation state
    void SetAnimation(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation);
    std::shared_ptr<GameEngine::Animation::SkeletalAnimation> GetAnimation() const;

    // Blend tree state
    void SetBlendTree(std::shared_ptr<BlendTree> blendTree);
    std::shared_ptr<BlendTree> GetBlendTree() const;

    // Sub-state machine
    void SetSubStateMachine(std::shared_ptr<AnimationStateMachine> subStateMachine);
    std::shared_ptr<AnimationStateMachine> GetSubStateMachine() const;

    // Events
    void AddEvent(const AnimationEvent& event);
    std::vector<AnimationEvent> GetEvents() const;

    // Execution
    void OnEnter(AnimationController* controller);
    void OnUpdate(float deltaTime, AnimationController* controller);
    void OnExit(AnimationController* controller);

private:
    std::string m_name;
    Type m_type = Type::Single;
    float m_speed = 1.0f;
    bool m_looping = true;

    std::shared_ptr<GameEngine::Animation::SkeletalAnimation> m_animation;
    std::shared_ptr<BlendTree> m_blendTree;
    std::shared_ptr<AnimationStateMachine> m_subStateMachine;
    std::vector<AnimationEvent> m_events;
};
```

### Transitions

```cpp
class AnimationTransition {
public:
    // Transition properties
    void SetDuration(float duration);
    void SetOffset(float offset);
    void SetInterruptionSource(InterruptionSource source);
    void SetOrderedInterruption(bool ordered);

    // Conditions
    void AddCondition(std::shared_ptr<AnimationCondition> condition);
    void RemoveCondition(std::shared_ptr<AnimationCondition> condition);
    std::vector<std::shared_ptr<AnimationCondition>> GetConditions() const;

    // Evaluation
    bool CanTransition(AnimationController* controller) const;
    float GetTransitionTime(AnimationController* controller) const;

private:
    float m_duration = 0.25f;
    float m_offset = 0.0f;
    InterruptionSource m_interruptionSource = InterruptionSource::None;
    bool m_orderedInterruption = true;
    std::vector<std::shared_ptr<AnimationCondition>> m_conditions;
};

// Transition conditions
class AnimationCondition {
public:
    enum class Type { Greater, Less, Equal, NotEqual, True, False };

    virtual bool Evaluate(AnimationController* controller) const = 0;
};

class FloatCondition : public AnimationCondition {
public:
    FloatCondition(const std::string& parameter, Type type, float threshold);
    bool Evaluate(AnimationController* controller) const override;

private:
    std::string m_parameter;
    Type m_type;
    float m_threshold;
};
```

## 🌳 Blend Trees

### Blend Tree System

```cpp
class BlendTree {
public:
    enum class Type { Simple1D, SimpleDirectional2D, FreeformDirectional2D, FreeformCartesian2D };

    // Blend tree properties
    void SetType(Type type);
    void SetParameter(const std::string& parameter);
    void SetParameters(const std::string& paramX, const std::string& paramY);

    // Motion management
    void AddMotion(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation, float threshold);
    void AddMotion(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation, const Math::Vec2& position);
    void RemoveMotion(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation);

    // Blend tree nodes
    void AddChildBlendTree(std::shared_ptr<BlendTree> childTree, float threshold);
    void AddChildBlendTree(std::shared_ptr<BlendTree> childTree, const Math::Vec2& position);

    // Evaluation
    void Evaluate(AnimationController* controller, std::vector<AnimationSample>& samples) const;

private:
    struct BlendTreeNode {
        std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation;
        std::shared_ptr<BlendTree> childTree;
        float threshold = 0.0f;
        Math::Vec2 position = Math::Vec2(0.0f);
        float weight = 0.0f;
    };

    Type m_type = Type::Simple1D;
    std::string m_parameterX;
    std::string m_parameterY;
    std::vector<BlendTreeNode> m_nodes;
};

struct AnimationSample {
    std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation;
    float weight;
    float time;
};
```

### Blend Tree Types

```cpp
// 1D Blend Tree (e.g., walk/run based on speed)
class Simple1DBlendTree : public BlendTree {
public:
    void AddMotion(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation, float threshold);
    void Evaluate(float parameter, std::vector<AnimationSample>& samples) const;
};

// 2D Directional Blend Tree (e.g., movement directions)
class SimpleDirectional2DBlendTree : public BlendTree {
public:
    void AddMotion(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation, const Math::Vec2& direction);
    void Evaluate(const Math::Vec2& direction, std::vector<AnimationSample>& samples) const;
};

// 2D Freeform Blend Tree (e.g., strafe movement)
class Freeform2DBlendTree : public BlendTree {
public:
    void AddMotion(std::shared_ptr<GameEngine::Animation::SkeletalAnimation> animation, const Math::Vec2& position);
    void Evaluate(const Math::Vec2& position, std::vector<AnimationSample>& samples) const;

private:
    void CalculateWeights(const Math::Vec2& position, std::vector<float>& weights) const;
};
```

## 🎪 Animation Events

### Event System

```cpp
struct AnimationEvent {
    std::string name;
    float time;  // Normalized time (0-1)
    std::string stringParameter;
    float floatParameter = 0.0f;
    int intParameter = 0;
    bool boolParameter = false;

    // Event types
    enum class Type { Generic, Sound, Effect, Footstep, Custom };
    Type type = Type::Generic;
};

class AnimationEventSystem {
public:
    // Event callbacks
    void RegisterEventHandler(const std::string& eventName, std::function<void(const AnimationEvent&)> handler);
    void UnregisterEventHandler(const std::string& eventName);

    // Event processing
    void ProcessEvents(const std::vector<AnimationEvent>& events);
    void TriggerEvent(const AnimationEvent& event);

    // Built-in event types
    void RegisterSoundEventHandler(std::function<void(const std::string&, float)> handler);
    void RegisterEffectEventHandler(std::function<void(const std::string&, const Math::Vec3&)> handler);
    void RegisterFootstepEventHandler(std::function<void(const std::string&)> handler);

private:
    std::unordered_map<std::string, std::function<void(const AnimationEvent&)>> m_eventHandlers;
};
```

## 🎨 Morph Target Animation

### Morph Target System

```cpp
class MorphTarget {
public:
    // Vertex data
    void SetVertexDeltas(const std::vector<Math::Vec3>& positionDeltas);
    void SetNormalDeltas(const std::vector<Math::Vec3>& normalDeltas);
    void SetTangentDeltas(const std::vector<Math::Vec3>& tangentDeltas);

    // Properties
    void SetName(const std::string& name);
    void SetWeight(float weight);
    float GetWeight() const;

    // Application
    void ApplyToMesh(Mesh& mesh, float weight) const;

private:
    std::string m_name;
    float m_weight = 0.0f;
    std::vector<Math::Vec3> m_positionDeltas;
    std::vector<Math::Vec3> m_normalDeltas;
    std::vector<Math::Vec3> m_tangentDeltas;
};

class MorphTargetController {
public:
    // Morph target management
    void AddMorphTarget(std::shared_ptr<MorphTarget> morphTarget);
    void RemoveMorphTarget(const std::string& name);
    std::shared_ptr<MorphTarget> GetMorphTarget(const std::string& name) const;

    // Weight control
    void SetWeight(const std::string& name, float weight);
    float GetWeight(const std::string& name) const;

    // Animation
    void AnimateWeight(const std::string& name, float targetWeight, float duration);
    void Update(float deltaTime);

    // Application
    void ApplyToMesh(Mesh& mesh) const;

private:
    std::unordered_map<std::string, std::shared_ptr<MorphTarget>> m_morphTargets;
    std::unordered_map<std::string, float> m_targetWeights;
    std::unordered_map<std::string, float> m_animationSpeeds;
};
```

## 🔧 Procedural Animation

### Inverse Kinematics (IK)

```cpp
class IKSolver {
public:
    enum class Type { TwoBone, FABRIK, CCD };

    // IK chain setup
    void SetChain(const std::vector<int>& boneIndices);
    void SetTarget(const Math::Vec3& position, const Math::Quat& rotation);
    void SetPoleTarget(const Math::Vec3& position);

    // Constraints
    void SetBoneConstraints(int boneIndex, float minAngle, float maxAngle);
    void SetChainLength(float length);

    // Solving
    bool Solve(GameEngine::Animation::AnimationSkeleton& skeleton, std::vector<Math::Mat4>& boneTransforms);
    void SetIterations(int iterations);
    void SetTolerance(float tolerance);

private:
    Type m_type = Type::TwoBone;
    std::vector<int> m_boneChain;
    Math::Vec3 m_targetPosition;
    Math::Quat m_targetRotation;
    Math::Vec3 m_poleTarget;
    int m_iterations = 10;
    float m_tolerance = 0.01f;
};

// Two-bone IK (e.g., arm, leg)
class TwoBoneIK : public IKSolver {
public:
    void SetUpperBone(int boneIndex);
    void SetLowerBone(int boneIndex);
    void SetEndEffector(int boneIndex);

    bool Solve(GameEngine::Animation::AnimationSkeleton& skeleton, std::vector<Math::Mat4>& boneTransforms) override;
};
```

### Look-At System

```cpp
class LookAtController {
public:
    // Setup
    void SetHeadBone(int boneIndex);
    void SetEyeBones(int leftEye, int rightEye);
    void SetConstraints(float maxAngle, float maxSpeed);

    // Target
    void SetTarget(const Math::Vec3& worldPosition);
    void SetTargetWeight(float weight);

    // Update
    void Update(float deltaTime, GameEngine::Animation::AnimationSkeleton& skeleton, std::vector<Math::Mat4>& boneTransforms);

private:
    int m_headBone = -1;
    int m_leftEyeBone = -1;
    int m_rightEyeBone = -1;
    Math::Vec3 m_target;
    float m_weight = 1.0f;
    float m_maxAngle = 90.0f;
    float m_maxSpeed = 180.0f;  // degrees per second
};
```

## 🎮 Usage Examples

### Basic Character Animation

```cpp
// Load animated model
auto character = modelLoader->LoadModel("models/character.fbx");
auto skeleton = character->GetSkeleton();

// Create animation controller
auto animController = std::make_unique<AnimationController>();
animController->SetSkeleton(skeleton);

// Load animations
auto idleAnim = resourceManager->Load<GameEngine::Animation::SkeletalAnimation>("animations/idle.fbx");
auto walkAnim = resourceManager->Load<GameEngine::Animation::SkeletalAnimation>("animations/walk.fbx");
auto runAnim = resourceManager->Load<GameEngine::Animation::SkeletalAnimation>("animations/run.fbx");

// Create state machine
auto stateMachine = std::make_shared<AnimationStateMachine>();

// Create states
auto idleState = std::make_shared<AnimationState>();
idleState->SetName("Idle");
idleState->SetAnimation(idleAnim);

auto walkState = std::make_shared<AnimationState>();
walkState->SetName("Walk");
walkState->SetAnimation(walkAnim);

auto runState = std::make_shared<AnimationState>();
runState->SetName("Run");
runState->SetAnimation(runAnim);

// Add states to state machine
stateMachine->AddState(idleState);
stateMachine->AddState(walkState);
stateMachine->AddState(runState);
stateMachine->SetEntryState("Idle");

// Create transitions
auto idleToWalk = std::make_shared<AnimationTransition>();
idleToWalk->SetDuration(0.2f);
idleToWalk->AddCondition(std::make_shared<FloatCondition>("Speed", AnimationCondition::Type::Greater, 0.1f));

auto walkToRun = std::make_shared<AnimationTransition>();
walkToRun->SetDuration(0.3f);
walkToRun->AddCondition(std::make_shared<FloatCondition>("Speed", AnimationCondition::Type::Greater, 5.0f));

stateMachine->AddTransition("Idle", "Walk", idleToWalk);
stateMachine->AddTransition("Walk", "Run", walkToRun);

animController->SetStateMachine(stateMachine);
```

### Blend Tree Setup

```cpp
// Create locomotion blend tree
auto locomotionBlendTree = std::make_shared<Simple1DBlendTree>();
locomotionBlendTree->SetParameter("Speed");

// Add animations to blend tree
locomotionBlendTree->AddMotion(idleAnim, 0.0f);
locomotionBlendTree->AddMotion(walkAnim, 2.0f);
locomotionBlendTree->AddMotion(runAnim, 6.0f);

// Create blend tree state
auto locomotionState = std::make_shared<AnimationState>();
locomotionState->SetName("Locomotion");
locomotionState->SetType(AnimationState::Type::BlendTree);
locomotionState->SetBlendTree(locomotionBlendTree);

stateMachine->AddState(locomotionState);
```

### Animation Events

```cpp
// Set up event handling
animController->SetEventCallback([](const AnimationEvent& event) {
    if (event.name == "Footstep") {
        // Play footstep sound
        audioEngine->PlaySound("footstep.wav", event.floatParameter);  // Volume
    } else if (event.name == "Attack") {
        // Trigger attack logic
        character->PerformAttack();
    }
});

// Add events to animation
AnimationEvent footstepEvent;
footstepEvent.name = "Footstep";
footstepEvent.time = 0.3f;  // 30% through animation
footstepEvent.floatParameter = 0.8f;  // Volume

walkAnim->AddEvent(footstepEvent);
```

### IK Setup

```cpp
// Set up foot IK for character
auto leftFootIK = std::make_unique<TwoBoneIK>();
leftFootIK->SetUpperBone(skeleton->GetBoneIndex("LeftThigh"));
leftFootIK->SetLowerBone(skeleton->GetBoneIndex("LeftShin"));
leftFootIK->SetEndEffector(skeleton->GetBoneIndex("LeftFoot"));

auto rightFootIK = std::make_unique<TwoBoneIK>();
rightFootIK->SetUpperBone(skeleton->GetBoneIndex("RightThigh"));
rightFootIK->SetLowerBone(skeleton->GetBoneIndex("RightShin"));
rightFootIK->SetEndEffector(skeleton->GetBoneIndex("RightFoot"));

// Update IK in game loop
void Update(float deltaTime) {
    // Update animation
    animController->Update(deltaTime);

    // Get bone transforms
    std::vector<Math::Mat4> boneTransforms;
    animController->GetBoneTransforms(boneTransforms);

    // Apply foot IK
    Math::Vec3 leftFootTarget = GetGroundPosition(character->GetPosition() + leftFootOffset);
    Math::Vec3 rightFootTarget = GetGroundPosition(character->GetPosition() + rightFootOffset);

    leftFootIK->SetTarget(leftFootTarget, Math::Quat::Identity());
    rightFootIK->SetTarget(rightFootTarget, Math::Quat::Identity());

    leftFootIK->Solve(*skeleton, boneTransforms);
    rightFootIK->Solve(*skeleton, boneTransforms);

    // Update skeleton
    skeleton->SetBoneTransforms(boneTransforms);
}
```

### Morph Target Animation

```cpp
// Load model with morph targets
auto faceModel = modelLoader->LoadModel("models/character_face.fbx");
auto morphController = std::make_unique<MorphTargetController>();

// Get morph targets from model
auto morphTargets = faceModel->GetMorphTargets();
for (auto& morphTarget : morphTargets) {
    morphController->AddMorphTarget(morphTarget);
}

// Animate facial expressions
morphController->AnimateWeight("Smile", 1.0f, 0.5f);  // Smile over 0.5 seconds
morphController->AnimateWeight("Blink", 1.0f, 0.1f);  // Quick blink

// Update in game loop
void Update(float deltaTime) {
    morphController->Update(deltaTime);

    // Apply morph targets to mesh
    auto faceMesh = faceModel->GetMesh("Face");
    morphController->ApplyToMesh(*faceMesh);
}
```

## 🔮 Future Enhancements

### Planned Features (v1.2+)

- **Motion Matching**: AI-driven animation selection
- **Ragdoll Physics**: Physics-driven character animation
- **Cloth Animation**: Realistic fabric simulation
- **Crowd Animation**: Efficient animation for many characters
- **Animation Compression**: Reduced memory usage
- **Real-time Retargeting**: Animation adaptation between skeletons

### Advanced Features

- **Machine Learning**: AI-generated animations
- **Motion Capture Integration**: Real-time mocap support
- **Procedural Walk Cycles**: Automatic gait generation
- **Emotion System**: Facial animation driven by emotions
- **Lip Sync**: Automatic mouth animation from audio

## 📚 Best Practices

### Performance Optimization

1. **LOD Animations**: Use simpler animations for distant characters
2. **Bone Culling**: Skip bones outside view frustum
3. **Animation Compression**: Reduce keyframe data
4. **Batch Updates**: Update multiple characters together
5. **GPU Skinning**: Offload bone calculations to GPU

### Animation Quality

1. **Smooth Transitions**: Use appropriate blend times
2. **Root Motion**: Handle character movement properly
3. **Event Timing**: Place events at natural points
4. **IK Constraints**: Set realistic bone limits
5. **Morph Target Limits**: Avoid extreme deformations

---

The Animation System in Game Engine Kiro v1.1 provides comprehensive tools for creating lifelike character animations and complex animation behaviors, enabling developers to bring their characters to life with professional-quality animation techniques.

**Game Engine Kiro v1.1** - Animating the future of game development.
