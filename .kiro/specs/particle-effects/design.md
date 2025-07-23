# Design Document - Particle Effects System

## Overview

This design document outlines the implementation of a comprehensive particle effects system for Game Engine Kiro v1.1. The system provides GPU-accelerated particle simulation, flexible emitters, advanced physics integration, and a wide range of visual effects for creating stunning particle-based visuals.

## Architecture

### Particle System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   Particle System Manager                  │
├─────────────────────────────────────────────────────────────┤
│  Effect Pool  │  Emitter Pool │  Renderer  │  Physics      │
├─────────────────────────────────────────────────────────────┤
│                    Particle Effects                        │
├─────────────────────────────────────────────────────────────┤
│  Emitters  │  Modifiers  │  Renderers  │  Collision       │
├─────────────────────────────────────────────────────────────┤
│                    GPU Simulation                          │
├─────────────────────────────────────────────────────────────┤
│ Compute Shaders │ Buffers │ Synchronization │ Memory Mgmt  │
├─────────────────────────────────────────────────────────────┤
│                    Particle Data                           │
├─────────────────────────────────────────────────────────────┤
│ Position │ Velocity │ Color │ Size │ Life │ Custom Data    │
└─────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### 1. ParticleSystemManager

```cpp
class ParticleSystemManager {
public:
    // Lifecycle
    bool Initialize();
    void Shutdown();
    void Update(float deltaTime);
    void Render(const Camera& camera);

    // Effect management
    std::shared_ptr<ParticleEffect> CreateEffect(const std::string& name);
    std::shared_ptr<ParticleEffect> LoadEffect(const std::string& filepath);
    void DestroyEffect(std::shared_ptr<ParticleEffect> effect);

    // Global settings
    void SetMaxParticles(uint32_t maxParticles);
    void SetSimulationSpace(SimulationSpace space);
    void SetGlobalForces(const std::vector<Math::Vec3>& forces);

    // Performance
    void SetLODEnabled(bool enabled);
    void SetCullingEnabled(bool enabled);
    void SetGPUSimulation(bool enabled);

private:
    std::vector<std::shared_ptr<ParticleEffect>> m_effects;
    std::unique_ptr<ParticleRenderer> m_renderer;
    std::unique_ptr<ParticlePhysics> m_physics;
    std::unique_ptr<ParticlePool> m_particlePool;

    uint32_t m_maxParticles = 100000;
    bool m_gpuSimulation = true;
};
```

### 2. ParticleEffect

```cpp
class ParticleEffect {
public:
    // Effect properties
    void SetName(const std::string& name);
    void SetDuration(float duration);
    void SetLooping(bool looping);
    void SetPrewarm(bool prewarm);

    // Emitter management
    void AddEmitter(std::shared_ptr<ParticleEmitter> emitter);
    void RemoveEmitter(std::shared_ptr<ParticleEmitter> emitter);
    std::vector<std::shared_ptr<ParticleEmitter>> GetEmitters() const;

    // Playback control
    void Play();
    void Stop();
    void Pause();
    void Restart();
    bool IsPlaying() const;

    // Transform
    void SetPosition(const Math::Vec3& position);
    void SetRotation(const Math::Quat& rotation);
    void SetScale(const Math::Vec3& scale);
    Math::Mat4 GetTransform() const;

    // Update and rendering
    void Update(float deltaTime);
    void Render(const Camera& camera, ParticleRenderer* renderer);

    // Serialization
    void SaveToFile(const std::string& filepath) const;
    bool LoadFromFile(const std::string& filepath);

private:
    std::string m_name;
    float m_duration = 5.0f;
    bool m_looping = false;
    bool m_prewarm = false;
    bool m_isPlaying = false;
    float m_time = 0.0f;

    Math::Vec3 m_position = Math::Vec3(0.0f);
    Math::Quat m_rotation = Math::Quat::Identity();
    Math::Vec3 m_scale = Math::Vec3(1.0f);

    std::vector<std::shared_ptr<ParticleEmitter>> m_emitters;
};
```

### 3. GPU Particle Simulation

```cpp
class GPUParticleSimulator {
public:
    // Initialization
    bool Initialize(uint32_t maxParticles);
    void Shutdown();

    // Simulation
    void Update(float deltaTime);
    void EmitParticles(const std::vector<ParticleEmissionData>& emissionData);

    // Buffer management
    void UploadParticleData(const std::vector<Particle>& particles);
    void DownloadParticleData(std::vector<Particle>& particles);

    // Compute shaders
    void SetUpdateShader(std::shared_ptr<ComputeShader> shader);
    void SetEmissionShader(std::shared_ptr<ComputeShader> shader);

    // Performance
    uint32_t GetActiveParticleCount() const;
    uint32_t GetMaxParticleCount() const;

private:
    std::shared_ptr<ComputeShader> m_updateShader;
    std::shared_ptr<ComputeShader> m_emissionShader;

    // GPU buffers
    GLuint m_particleBuffer = 0;
    GLuint m_emissionBuffer = 0;
    GLuint m_counterBuffer = 0;
    GLuint m_indirectBuffer = 0;

    uint32_t m_maxParticles = 0;
    uint32_t m_activeParticles = 0;
};
```

## Testing Strategy

### Unit Testing

```cpp
// Test particle emission
bool TestParticleEmission() {
    TestOutput::PrintTestStart("particle emission");

    ParticleSystemManager manager;
    EXPECT_TRUE(manager.Initialize());

    auto effect = manager.CreateEffect("test_effect");
    EXPECT_NOT_NULL(effect);

    auto emitter = std::make_shared<ContinuousEmitter>();
    emitter->SetEmissionRate(10.0f);
    effect->AddEmitter(emitter);

    effect->Play();
    EXPECT_TRUE(effect->IsPlaying());

    TestOutput::PrintTestPass("particle emission");
    return true;
}
```
