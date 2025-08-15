# Animation Development Workflow - Game Engine Kiro

## Overview

This guide provides a comprehensive workflow for implementing character animations in Game Engine Kiro, using the Xbot character as a reference implementation. The workflow follows industry standards for modular animation systems and demonstrates best practices for game animation development.

## Animation System Architecture

### Core Components

```
Character (Game Layer)
    ↓
AnimationController (Animation System)
    ↓
AnimationStateMachine (State Management)
    ↓
BlendTree (Animation Blending)
    ↓
SkeletalAnimation (Animation Data)
    ↓
AnimationSkeleton (Bone Hierarchy)
```

### Key Principles

1. **Modular Design**: Each animation is a separate asset that can be loaded independently
2. **State-Driven**: Animations are controlled through state machines with clear transitions
3. **Parameter-Based**: Animation behavior is controlled through parameters (speed, direction, etc.)
4. **Event-Driven**: Animation events trigger game logic (footsteps, combat hits, etc.)
5. **Performance-Optimized**: LOD system and efficient blending for multiple characters

## Available Xbot Animations

The following animations are available in `assets/meshes/` for the Xbot character:

### Essential Animations

- **Idle.fbx** - Default standing pose
- **Walking.fbx** - Basic walking cycle
- **Running.fbx** - High-speed movement
- **Jump.fbx** - Jump animation

### Combat Animations

- **Attack.fbx** - Basic attack animation
- **Block.fbx** - Defensive blocking pose
- **Hit.fbx** - Damage reaction animation
- **Dying.fbx** - Death sequence

### Additional Animations

- **Celebrate.fbx** - Victory/achievement animation
- **Left Turn.fbx** - Turning left animation
- **Right Turn.fbx** - Turning right animation
- **Crouched Walking.fbx** - Stealth/crouch movement

## Step-by-Step Implementation Guide

### Step 1: Character Animation Integration

```cpp
// In Character class (Game/Character.h)
class Character {
private:
    std::unique_ptr<Animation::AnimationController> m_animationController;
    std::shared_ptr<Animation::AnimationSkeleton> m_skeleton;

public:
    bool LoadAnimationSystem(const std::string& modelPath);
    void UpdateAnimations(float deltaTime);
    void SetAnimationParameter(const std::string& name, float value);
};
```

### Step 2: Animation Loading System

```cpp
bool Character::LoadAnimationSystem(const std::string& modelPath) {
    // Load the main character model (XBot.fbx)
    if (!LoadFBXModel(modelPath)) {
        LOG_ERROR("Failed to load character model: " + modelPath);
        return false;
    }

    // Extract skeleton from the loaded model
    m_skeleton = ExtractSkeletonFromModel();
    if (!m_skeleton) {
        LOG_ERROR("Failed to extract skeleton from model");
        return false;
    }

    // Initialize animation controller
    m_animationController = std::make_unique<Animation::AnimationController>();
    if (!m_animationController->Initialize(m_skeleton)) {
        LOG_ERROR("Failed to initialize animation controller");
        return false;
    }

    // Load individual animation files
    LoadAnimation("idle", "assets/meshes/Idle.fbx");
    LoadAnimation("walk", "assets/meshes/Walking.fbx");
    LoadAnimation("run", "assets/meshes/Running.fbx");
    LoadAnimation("jump", "assets/meshes/Jump.fbx");
    // ... load other animations

    return true;
}
```

### Step 3: State Machine Configuration

```cpp
void Character::SetupAnimationStateMachine() {
    auto stateMachine = std::make_shared<Animation::AnimationStateMachine>();

    // Create states
    auto idleState = std::make_shared<Animation::AnimationState>("Idle");
    idleState->SetAnimation(m_animationController->GetAnimation("idle"));

    auto walkState = std::make_shared<Animation::AnimationState>("Walk");
    walkState->SetAnimation(m_animationController->GetAnimation("walk"));

    auto runState = std::make_shared<Animation::AnimationState>("Run");
    runState->SetAnimation(m_animationController->GetAnimation("run"));

    // Add states to state machine
    stateMachine->AddState(idleState);
    stateMachine->AddState(walkState);
    stateMachine->AddState(runState);
    stateMachine->SetEntryState("Idle");

    // Create transitions
    CreateMovementTransitions(stateMachine);

    m_animationController->SetStateMachine(stateMachine);
}
```

### Step 4: Movement-Based Animation Transitions

```cpp
void Character::CreateMovementTransitions(std::shared_ptr<Animation::AnimationStateMachine> stateMachine) {
    // Idle to Walk transition (when speed > 0.1)
    auto idleToWalk = std::make_shared<Animation::AnimationTransition>();
    idleToWalk->AddCondition("Speed", Animation::ComparisonType::Greater, 0.1f);
    idleToWalk->SetTransitionTime(0.2f);
    stateMachine->AddTransition("Idle", "Walk", idleToWalk);

    // Walk to Idle transition (when speed <= 0.1)
    auto walkToIdle = std::make_shared<Animation::AnimationTransition>();
    walkToIdle->AddCondition("Speed", Animation::ComparisonType::LessEqual, 0.1f);
    walkToIdle->SetTransitionTime(0.2f);
    stateMachine->AddTransition("Walk", "Idle", walkToIdle);

    // Walk to Run transition (when speed > 3.0)
    auto walkToRun = std::make_shared<Animation::AnimationTransition>();
    walkToRun->AddCondition("Speed", Animation::ComparisonType::Greater, 3.0f);
    walkToRun->SetTransitionTime(0.3f);
    stateMachine->AddTransition("Walk", "Run", walkToRun);

    // Run to Walk transition (when speed <= 3.0)
    auto runToWalk = std::make_shared<Animation::AnimationTransition>();
    runToWalk->AddCondition("Speed", Animation::ComparisonType::LessEqual, 3.0f);
    runToWalk->SetTransitionTime(0.3f);
    stateMachine->AddTransition("Run", "Walk", runToWalk);
}
```

### Step 5: Blend Tree for Smooth Movement

```cpp
void Character::SetupMovementBlendTree() {
    // Create 1D blend tree for movement speed
    auto movementBlendTree = std::make_shared<Animation::BlendTree>(Animation::BlendTree::Type::Simple1D);
    movementBlendTree->SetParameter("Speed");

    // Add animations with speed thresholds
    movementBlendTree->AddMotion(m_animationController->GetAnimation("idle"), 0.0f);
    movementBlendTree->AddMotion(m_animationController->GetAnimation("walk"), 2.0f);
    movementBlendTree->AddMotion(m_animationController->GetAnimation("run"), 6.0f);

    // Create blend tree state
    auto movementState = std::make_shared<Animation::AnimationState>("Movement", Animation::AnimationState::Type::BlendTree);
    movementState->SetBlendTree(movementBlendTree);

    // Replace individual movement states with blend tree
    auto stateMachine = m_animationController->GetStateMachine();
    stateMachine->AddState(movementState);
    stateMachine->SetEntryState("Movement");
}
```

### Step 6: Animation Events for Game Logic

```cpp
void Character::SetupAnimationEvents() {
    // Set up event callback
    m_animationController->SetEventCallback([this](const Animation::AnimationEvent& event) {
        HandleAnimationEvent(event);
    });

    // Add footstep events to walking animation
    auto walkAnimation = m_animationController->GetAnimation("walk");
    if (walkAnimation) {
        Animation::AnimationEvent leftFootstep;
        leftFootstep.name = "footstep";
        leftFootstep.time = 0.2f; // 20% through the animation
        leftFootstep.stringParameter = "left";
        walkAnimation->AddEvent(leftFootstep);

        Animation::AnimationEvent rightFootstep;
        rightFootstep.name = "footstep";
        rightFootstep.time = 0.7f; // 70% through the animation
        rightFootstep.stringParameter = "right";
        walkAnimation->AddEvent(rightFootstep);
    }
}

void Character::HandleAnimationEvent(const Animation::AnimationEvent& event) {
    if (event.name == "footstep") {
        // Play footstep sound
        if (m_audioManager) {
            m_audioManager->PlayFootstepSound(GetPosition(), event.stringParameter);
        }

        // Create dust particle effect
        if (m_particleSystem) {
            m_particleSystem->CreateFootstepDust(GetPosition());
        }
    }
    else if (event.name == "attack_hit") {
        // Process combat hit detection
        ProcessCombatHit();
    }
}
```

### Step 7: Integration with Character Movement

```cpp
void Character::Update(float deltaTime, InputManager* input, Camera* camera) {
    // Update movement (existing code)
    UpdateMovement(deltaTime, input, camera);

    // Update animation parameters based on movement
    UpdateAnimationParameters();

    // Update animation system
    if (m_animationController) {
        m_animationController->Update(deltaTime);
    }
}

void Character::UpdateAnimationParameters() {
    if (!m_animationController) return;

    // Calculate movement speed
    Math::Vec3 velocity = GetVelocity();
    float speed = glm::length(velocity);

    // Set speed parameter for blend tree
    m_animationController->SetFloat("Speed", speed);

    // Set direction parameters for directional animations
    if (speed > 0.1f) {
        Math::Vec3 forward = GetForward();
        Math::Vec3 normalizedVelocity = glm::normalize(velocity);

        float forwardDot = glm::dot(forward, normalizedVelocity);
        float rightDot = glm::dot(GetRight(), normalizedVelocity);

        m_animationController->SetFloat("Forward", forwardDot);
        m_animationController->SetFloat("Right", rightDot);
    }

    // Set grounded state
    m_animationController->SetBool("IsGrounded", IsGrounded());

    // Set combat state
    m_animationController->SetBool("InCombat", IsInCombat());
}
```

### Step 8: Rendering Integration

```cpp
void Character::Render(PrimitiveRenderer* renderer) {
    if (!m_animationController || !m_skeleton) {
        // Fallback to primitive rendering
        RenderPrimitive(renderer);
        return;
    }

    // Evaluate current animation pose
    std::vector<Math::Mat4> boneMatrices;
    m_animationController->Evaluate(boneMatrices);

    // Render animated mesh with bone matrices
    if (m_mesh && m_mesh->HasSkeleton()) {
        renderer->DrawSkinnedMesh(m_mesh, GetTransform(), boneMatrices);
    }

    // Debug visualization
    if (m_showDebugSkeleton) {
        RenderSkeletonDebug(renderer, boneMatrices);
    }
}
```

## GameExample Integration

### Controls for Animation Testing

```cpp
// In GameExample Update method
void GameApplication::HandleAnimationControls(InputManager* input) {
    if (!m_character || !m_character->HasAnimationController()) return;

    auto animController = m_character->GetAnimationController();

    // Basic animation testing
    if (input->IsKeyPressed(KeyCode::Num1)) {
        animController->Play("idle");
        LOG_INFO("Playing Idle animation");
    }
    if (input->IsKeyPressed(KeyCode::Num2)) {
        animController->Play("walk");
        LOG_INFO("Playing Walk animation");
    }
    if (input->IsKeyPressed(KeyCode::Num3)) {
        animController->Play("run");
        LOG_INFO("Playing Run animation");
    }
    if (input->IsKeyPressed(KeyCode::Num4)) {
        animController->Play("jump");
        LOG_INFO("Playing Jump animation");
    }

    // Combat animations
    if (input->IsKeyPressed(KeyCode::Q)) {
        animController->SetTrigger("Attack");
        LOG_INFO("Triggered Attack animation");
    }
    if (input->IsKeyPressed(KeyCode::E)) {
        animController->SetBool("Blocking", !animController->GetBool("Blocking"));
        LOG_INFO("Toggled Block animation");
    }

    // Special animations
    if (input->IsKeyPressed(KeyCode::C)) {
        animController->Play("celebrate");
        LOG_INFO("Playing Celebrate animation");
    }
    if (input->IsKeyPressed(KeyCode::X)) {
        animController->SetBool("Crouching", !animController->GetBool("Crouching"));
        LOG_INFO("Toggled Crouch animation");
    }
}
```

### Animation Debug UI

```cpp
void GameApplication::RenderAnimationDebugInfo() {
    if (!m_character || !m_character->HasAnimationController()) return;

    auto debugInfo = m_character->GetAnimationController()->GetDebugInfo();

    LOG_INFO("=== ANIMATION DEBUG INFO ===");
    LOG_INFO("Current State: " + debugInfo.currentStateName);
    LOG_INFO("State Time: " + std::to_string(debugInfo.currentStateTime));
    LOG_INFO("Is Playing: " + std::string(debugInfo.isPlaying ? "Yes" : "No"));
    LOG_INFO("Is Paused: " + std::string(debugInfo.isPaused ? "Yes" : "No"));
    LOG_INFO("Playback Speed: " + std::to_string(debugInfo.playbackSpeed));
    LOG_INFO("Bone Count: " + std::to_string(debugInfo.boneCount));

    LOG_INFO("Parameters:");
    for (const auto& [name, param] : debugInfo.parameters) {
        switch (param.GetType()) {
            case Animation::AnimationParameter::Type::Float:
                LOG_INFO("  " + name + " (Float): " + std::to_string(param.AsFloat()));
                break;
            case Animation::AnimationParameter::Type::Int:
                LOG_INFO("  " + name + " (Int): " + std::to_string(param.AsInt()));
                break;
            case Animation::AnimationParameter::Type::Bool:
                LOG_INFO("  " + name + " (Bool): " + std::string(param.AsBool() ? "true" : "false"));
                break;
            case Animation::AnimationParameter::Type::Trigger:
                LOG_INFO("  " + name + " (Trigger): " + std::string(param.IsTrigger() ? "ACTIVE" : "inactive"));
                break;
        }
    }
    LOG_INFO("============================");
}
```

## Best Practices

### 1. Animation Asset Organization

```
assets/
├── meshes/
│   ├── characters/
│   │   ├── xbot/
│   │   │   ├── XBot.fbx              # Main character model
│   │   │   ├── animations/
│   │   │   │   ├── Idle.fbx
│   │   │   │   ├── Walking.fbx
│   │   │   │   ├── Running.fbx
│   │   │   │   └── ...
│   │   └── other_characters/
│   └── props/
```

### 2. Modular Animation System Design

- **Separate Concerns**: Keep animation logic separate from gameplay logic
- **Parameter-Driven**: Use parameters instead of direct animation calls
- **Event-Based**: Use animation events for game logic triggers
- **State-Machine Driven**: Use state machines for complex animation logic

### 3. Performance Considerations

- **Animation LOD**: Reduce animation quality for distant characters
- **Bone Culling**: Skip bone calculations for off-screen characters
- **Animation Compression**: Use compressed animation data for memory efficiency
- **Pooling**: Reuse animation controllers and data structures

### 4. Testing and Debugging

- **Individual Animation Testing**: Test each animation in isolation
- **State Machine Visualization**: Provide visual debugging for state transitions
- **Parameter Monitoring**: Real-time parameter value display
- **Event Logging**: Log all animation events for debugging

## Common Patterns

### Character State Integration

```cpp
// Synchronize animation with character state
void Character::SynchronizeAnimationWithState() {
    if (!m_animationController) return;

    // Movement state
    m_animationController->SetFloat("Speed", GetMovementSpeed());
    m_animationController->SetBool("IsGrounded", IsGrounded());
    m_animationController->SetBool("IsJumping", IsJumping());

    // Combat state
    m_animationController->SetBool("InCombat", IsInCombat());
    m_animationController->SetBool("IsAttacking", IsAttacking());
    m_animationController->SetBool("IsBlocking", IsBlocking());

    // Health state
    m_animationController->SetFloat("Health", GetHealthPercentage());
    m_animationController->SetBool("IsDead", IsDead());
}
```

### Animation Event Handling

```cpp
// Centralized animation event handling
void Character::ProcessAnimationEvent(const Animation::AnimationEvent& event) {
    // Audio events
    if (event.name == "footstep" || event.name == "land" || event.name == "jump") {
        m_audioManager->PlayAnimationSound(event.name, GetPosition());
    }

    // Combat events
    else if (event.name == "attack_start") {
        StartAttackHitbox();
    }
    else if (event.name == "attack_end") {
        EndAttackHitbox();
    }

    // Movement events
    else if (event.name == "step") {
        CreateFootstepParticles();
    }

    // Custom game events
    else if (event.name.starts_with("game_")) {
        m_gameEventSystem->TriggerEvent(event.name, event);
    }
}
```

This workflow provides a complete foundation for implementing professional character animation systems in Game Engine Kiro, following industry standards and best practices.
