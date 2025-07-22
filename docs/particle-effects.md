# Particle Effects System

Game Engine Kiro v1.1 introduces a powerful particle effects system for creating stunning visual effects including fire, smoke, explosions, magic spells, weather effects, and other dynamic visual elements that bring games to life.

## 🎯 Overview

The Particle Effects System provides a comprehensive solution for creating, managing, and rendering particle-based visual effects with GPU acceleration, advanced physics simulation, and flexible customization options.

### Key Features

- **GPU-Accelerated Simulation**: Compute shader-based particle updates
- **Flexible Emitters**: Multiple emission patterns and shapes
- **Physics Integration**: Collision detection and force simulation
- **Advanced Rendering**: Billboards, meshes, and trails
- **Visual Editor**: Real-time effect creation and editing
- **Performance Optimization**: LOD system and culling
- **Modular Design**: Reusable effect components

## 🏗️ Architecture Overview

### Particle System Hierarchy

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
│                    Particle Data                           │
├─────────────────────────────────────────────────────────────┤
│ Position │ Velocity │ Color │ Size │ Life │ Custom Data    │
└─────────────────────────────────────────────────────────────┘
```

### Core Components

**ParticleSystemManager**

```cpp
class ParticleSystemManager {
public:
    // System management
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

**ParticleEffect**

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

## ✨ Particle Emitters

### Emitter Types

```cpp
class ParticleEmitter {
public:
    enum class Shape { Point, Sphere, Box, Cone, Circle, Mesh };
    enum class EmissionType { Continuous, Burst };

    // Emission properties
    void SetEmissionRate(float rate);
    void SetBurstCount(uint32_t count);
    void SetBurstTime(float time);
    void SetEmissionType(EmissionType type);

    // Shape properties
    void SetShape(Shape shape);
    void SetShapeRadius(float radius);
    void SetShapeSize(const Math::Vec3& size);
    void SetShapeAngle(float angle);
    void SetShapeMesh(std::shared_ptr<Mesh> mesh);

    // Particle properties
    void SetStartLifetime(float min, float max);
    void SetStartSpeed(float min, float max);
    void SetStartSize(float min, float max);
    void SetStartColor(const Math::Vec4& min, const Math::Vec4& max);
    void SetStartRotation(float min, float max);

    // Emission
    virtual void Emit(float deltaTime, std::vector<Particle>& particles) = 0;

    // Transform
    void SetLocalPosition(const Math::Vec3& position);
    void SetLocalRotation(const Math::Quat& rotation);
    Math::Mat4 GetLocalTransform() const;

protected:
    Math::Vec3 GetEmissionPosition() const;
    Math::Vec3 GetEmissionDirection() const;

private:
    Shape m_shape = Shape::Point;
    EmissionType m_emissionType = EmissionType::Continuous;
    float m_emissionRate = 10.0f;
    uint32_t m_burstCount = 30;
    float m_burstTime = 0.0f;

    // Shape parameters
    float m_shapeRadius = 1.0f;
    Math::Vec3 m_shapeSize = Math::Vec3(1.0f);
    float m_shapeAngle = 25.0f;
    std::shared_ptr<Mesh> m_shapeMesh;

    // Particle spawn parameters
    Math::Vec2 m_startLifetime = Math::Vec2(1.0f, 5.0f);
    Math::Vec2 m_startSpeed = Math::Vec2(1.0f, 5.0f);
    Math::Vec2 m_startSize = Math::Vec2(0.1f, 1.0f);
    Math::Vec4 m_startColorMin = Math::Vec4(1.0f);
    Math::Vec4 m_startColorMax = Math::Vec4(1.0f);
    Math::Vec2 m_startRotation = Math::Vec2(0.0f, 360.0f);

    Math::Vec3 m_localPosition = Math::Vec3(0.0f);
    Math::Quat m_localRotation = Math::Quat::Identity();
};
```

### Specialized Emitters

```cpp
// Continuous emission
class ContinuousEmitter : public ParticleEmitter {
public:
    void Emit(float deltaTime, std::vector<Particle>& particles) override;

private:
    float m_emissionAccumulator = 0.0f;
};

// Burst emission
class BurstEmitter : public ParticleEmitter {
public:
    void SetBurstInterval(float interval);
    void Emit(float deltaTime, std::vector<Particle>& particles) override;

private:
    float m_burstInterval = 1.0f;
    float m_burstTimer = 0.0f;
};

// Trail emitter (follows moving object)
class TrailEmitter : public ParticleEmitter {
public:
    void SetTrailLength(float length);
    void SetTrailWidth(float width);
    void UpdateTrail(const Math::Vec3& position);
    void Emit(float deltaTime, std::vector<Particle>& particles) override;

private:
    float m_trailLength = 5.0f;
    float m_trailWidth = 0.1f;
    std::vector<Math::Vec3> m_trailPoints;
};
```

## 🎨 Particle Data and Modifiers

### Particle Structure

```cpp
struct Particle {
    // Core properties
    Math::Vec3 position = Math::Vec3(0.0f);
    Math::Vec3 velocity = Math::Vec3(0.0f);
    Math::Vec3 acceleration = Math::Vec3(0.0f);

    // Visual properties
    Math::Vec4 color = Math::Vec4(1.0f);
    float size = 1.0f;
    float rotation = 0.0f;
    float angularVelocity = 0.0f;

    // Lifecycle
    float life = 1.0f;
    float maxLife = 1.0f;
    float age = 0.0f;

    // Custom data
    Math::Vec4 customData1 = Math::Vec4(0.0f);
    Math::Vec4 customData2 = Math::Vec4(0.0f);

    // State
    bool isAlive = true;
    uint32_t emitterIndex = 0;

    // Helper methods
    float GetNormalizedAge() const { return age / maxLife; }
    bool ShouldDie() const { return age >= maxLife; }
};
```

### Particle Modifiers

```cpp
class ParticleModifier {
public:
    enum class Type {
        VelocityOverLifetime,
        ColorOverLifetime,
        SizeOverLifetime,
        RotationOverLifetime,
        ForceOverLifetime,
        Gravity,
        Drag,
        Turbulence,
        Collision
    };

    virtual void Apply(std::vector<Particle>& particles, float deltaTime) = 0;
    virtual Type GetType() const = 0;

    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }

protected:
    bool m_enabled = true;
};

// Velocity over lifetime
class VelocityOverLifetimeModifier : public ParticleModifier {
public:
    void SetVelocityCurve(const AnimationCurve& curve);
    void Apply(std::vector<Particle>& particles, float deltaTime) override;
    Type GetType() const override { return Type::VelocityOverLifetime; }

private:
    AnimationCurve m_velocityCurve;
};

// Color over lifetime
class ColorOverLifetimeModifier : public ParticleModifier {
public:
    void SetColorGradient(const ColorGradient& gradient);
    void Apply(std::vector<Particle>& particles, float deltaTime) override;
    Type GetType() const override { return Type::ColorOverLifetime; }

private:
    ColorGradient m_colorGradient;
};

// Size over lifetime
class SizeOverLifetimeModifier : public ParticleModifier {
public:
    void SetSizeCurve(const AnimationCurve& curve);
    void Apply(std::vector<Particle>& particles, float deltaTime) override;
    Type GetType() const override { return Type::SizeOverLifetime; }

private:
    AnimationCurve m_sizeCurve;
};
```

### Animation Curves and Gradients

```cpp
class AnimationCurve {
public:
    struct Keyframe {
        float time;
        float value;
        float inTangent = 0.0f;
        float outTangent = 0.0f;
    };

    void AddKeyframe(const Keyframe& keyframe);
    void RemoveKeyframe(int index);
    float Evaluate(float time) const;

private:
    std::vector<Keyframe> m_keyframes;
};

class ColorGradient {
public:
    struct ColorKey {
        float time;
        Math::Vec4 color;
    };

    void AddColorKey(const ColorKey& key);
    void RemoveColorKey(int index);
    Math::Vec4 Evaluate(float time) const;

private:
    std::vector<ColorKey> m_colorKeys;
};
```

## 🖥️ GPU Simulation

### Compute Shader Integration

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

### Compute Shader Example

```glsl
// particle_update.compute
#version 460 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct Particle {
    vec3 position;
    float life;
    vec3 velocity;
    float maxLife;
    vec4 color;
    float size;
    float rotation;
    float angularVelocity;
    float age;
    uint isAlive;
    uint emitterIndex;
    uint padding1;
    vec4 customData1;
    vec4 customData2;
};

layout(std430, binding = 0) restrict buffer ParticleBuffer {
    Particle particles[];
};

uniform float u_deltaTime;
uniform vec3 u_gravity;
uniform float u_drag;

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index >= particles.length()) return;

    Particle p = particles[index];
    if (p.isAlive == 0u) return;

    // Update age
    p.age += u_deltaTime;
    if (p.age >= p.maxLife) {
        p.isAlive = 0u;
        particles[index] = p;
        return;
    }

    // Apply forces
    p.velocity += u_gravity * u_deltaTime;
    p.velocity *= (1.0 - u_drag * u_deltaTime);

    // Update position
    p.position += p.velocity * u_deltaTime;

    // Update rotation
    p.rotation += p.angularVelocity * u_deltaTime;

    // Update color and size based on age
    float normalizedAge = p.age / p.maxLife;
    p.color.a = 1.0 - normalizedAge;  // Fade out over lifetime
    p.size = mix(1.0, 0.1, normalizedAge);  // Shrink over lifetime

    particles[index] = p;
}
```

## 🎨 Particle Rendering

### Rendering Modes

```cpp
class ParticleRenderer {
public:
    enum class RenderMode { Billboard, Mesh, Trail, Ribbon };
    enum class BlendMode { Alpha, Additive, Multiply, Screen };

    // Rendering setup
    void SetRenderMode(RenderMode mode);
    void SetBlendMode(BlendMode mode);
    void SetTexture(std::shared_ptr<Texture> texture);
    void SetMaterial(std::shared_ptr<Material> material);

    // Billboard settings
    void SetBillboardAlignment(BillboardAlignment alignment);
    void SetBillboardSize(const Math::Vec2& size);

    // Mesh settings
    void SetMesh(std::shared_ptr<Mesh> mesh);
    void SetMeshScale(const Math::Vec3& scale);

    // Trail settings
    void SetTrailWidth(float width);
    void SetTrailLength(float length);
    void SetTrailSegments(uint32_t segments);

    // Rendering
    void Render(const std::vector<Particle>& particles, const Camera& camera);
    void RenderInstanced(const std::vector<Particle>& particles, const Camera& camera);

    // Sorting
    void SetSortingEnabled(bool enabled);
    void SetSortingMode(SortingMode mode);

private:
    RenderMode m_renderMode = RenderMode::Billboard;
    BlendMode m_blendMode = BlendMode::Alpha;
    std::shared_ptr<Texture> m_texture;
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Mesh> m_mesh;

    // Rendering resources
    GLuint m_VAO = 0;
    GLuint m_instanceVBO = 0;
    std::shared_ptr<Shader> m_billboardShader;
    std::shared_ptr<Shader> m_meshShader;
    std::shared_ptr<Shader> m_trailShader;
};

enum class BillboardAlignment {
    ViewPlane,      // Face camera
    ViewPoint,      // Face camera position
    WorldUp,        // Face camera but keep world up
    LocalUp,        // Face camera but keep local up
    Velocity        // Align with velocity direction
};
```

### Instanced Rendering

```cpp
class InstancedParticleRenderer : public ParticleRenderer {
public:
    // Instance data structure
    struct InstanceData {
        Math::Mat4 transform;
        Math::Vec4 color;
        Math::Vec2 uvOffset;
        Math::Vec2 uvScale;
    };

    // Batch rendering
    void BeginBatch();
    void AddParticle(const Particle& particle, const Camera& camera);
    void EndBatch();
    void RenderBatch();

    // Performance
    void SetMaxInstances(uint32_t maxInstances);
    uint32_t GetInstanceCount() const;

private:
    std::vector<InstanceData> m_instanceData;
    GLuint m_instanceBuffer = 0;
    uint32_t m_maxInstances = 10000;
    uint32_t m_instanceCount = 0;
};
```

## 🌪️ Physics Integration

### Particle Physics

```cpp
class ParticlePhysics {
public:
    // Force management
    void AddGlobalForce(const Math::Vec3& force);
    void RemoveGlobalForce(const Math::Vec3& force);
    void ClearGlobalForces();

    // Collision detection
    void SetCollisionEnabled(bool enabled);
    void AddCollisionPlane(const Math::Vec4& plane);  // ax + by + cz + d = 0
    void AddCollisionSphere(const Math::Vec3& center, float radius);
    void AddCollisionBox(const Math::Vec3& center, const Math::Vec3& size);

    // Physics simulation
    void UpdatePhysics(std::vector<Particle>& particles, float deltaTime);

    // Collision response
    void SetBounciness(float bounciness);
    void SetFriction(float friction);
    void SetDamping(float damping);

private:
    struct CollisionPlane {
        Math::Vec4 plane;
        float bounciness = 0.5f;
        float friction = 0.1f;
    };

    struct CollisionSphere {
        Math::Vec3 center;
        float radius;
        float bounciness = 0.5f;
        float friction = 0.1f;
    };

    std::vector<Math::Vec3> m_globalForces;
    std::vector<CollisionPlane> m_collisionPlanes;
    std::vector<CollisionSphere> m_collisionSpheres;

    bool m_collisionEnabled = false;
    float m_defaultBounciness = 0.5f;
    float m_defaultFriction = 0.1f;
    float m_damping = 0.99f;
};
```

### Turbulence and Noise

```cpp
class TurbulenceModifier : public ParticleModifier {
public:
    // Turbulence settings
    void SetStrength(float strength);
    void SetFrequency(float frequency);
    void SetOctaves(int octaves);
    void SetLacunarity(float lacunarity);
    void SetPersistence(float persistence);

    // Noise type
    void SetNoiseType(NoiseType type);

    void Apply(std::vector<Particle>& particles, float deltaTime) override;
    Type GetType() const override { return Type::Turbulence; }

private:
    float m_strength = 1.0f;
    float m_frequency = 0.1f;
    int m_octaves = 4;
    float m_lacunarity = 2.0f;
    float m_persistence = 0.5f;
    NoiseType m_noiseType = NoiseType::Perlin;

    std::unique_ptr<NoiseGenerator> m_noiseGenerator;
};

enum class NoiseType {
    Perlin,
    Simplex,
    Worley,
    Ridged
};
```

## 🎮 Usage Examples

### Fire Effect

```cpp
// Create fire particle effect
auto fireEffect = particleManager->CreateEffect("Fire");

// Create main fire emitter
auto fireEmitter = std::make_shared<ContinuousEmitter>();
fireEmitter->SetShape(ParticleEmitter::Shape::Cone);
fireEmitter->SetShapeRadius(0.5f);
fireEmitter->SetShapeAngle(15.0f);
fireEmitter->SetEmissionRate(50.0f);
fireEmitter->SetStartLifetime(1.0f, 2.0f);
fireEmitter->SetStartSpeed(2.0f, 4.0f);
fireEmitter->SetStartSize(0.1f, 0.3f);
fireEmitter->SetStartColor(Math::Vec4(1.0f, 0.8f, 0.2f, 1.0f), Math::Vec4(1.0f, 0.4f, 0.1f, 1.0f));

// Add color over lifetime (fade from yellow to red to transparent)
auto colorModifier = std::make_shared<ColorOverLifetimeModifier>();
ColorGradient fireGradient;
fireGradient.AddColorKey({0.0f, Math::Vec4(1.0f, 1.0f, 0.5f, 1.0f)});  // Bright yellow
fireGradient.AddColorKey({0.3f, Math::Vec4(1.0f, 0.5f, 0.1f, 1.0f)});  // Orange
fireGradient.AddColorKey({0.7f, Math::Vec4(0.8f, 0.2f, 0.1f, 0.8f)});  // Red
fireGradient.AddColorKey({1.0f, Math::Vec4(0.2f, 0.1f, 0.1f, 0.0f)});  // Dark transparent
colorModifier->SetColorGradient(fireGradient);

// Add size over lifetime (grow then shrink)
auto sizeModifier = std::make_shared<SizeOverLifetimeModifier>();
AnimationCurve sizeCurve;
sizeCurve.AddKeyframe({0.0f, 0.5f});
sizeCurve.AddKeyframe({0.3f, 1.0f});
sizeCurve.AddKeyframe({1.0f, 0.2f});
sizeModifier->SetSizeCurve(sizeCurve);

// Add upward velocity
auto velocityModifier = std::make_shared<VelocityOverLifetimeModifier>();
AnimationCurve velocityCurve;
velocityCurve.AddKeyframe({0.0f, 1.0f});
velocityCurve.AddKeyframe({1.0f, 0.5f});
velocityModifier->SetVelocityCurve(velocityCurve);

fireEmitter->AddModifier(colorModifier);
fireEmitter->AddModifier(sizeModifier);
fireEmitter->AddModifier(velocityModifier);

fireEffect->AddEmitter(fireEmitter);

// Create smoke emitter
auto smokeEmitter = std::make_shared<ContinuousEmitter>();
smokeEmitter->SetShape(ParticleEmitter::Shape::Cone);
smokeEmitter->SetShapeRadius(0.3f);
smokeEmitter->SetShapeAngle(30.0f);
smokeEmitter->SetEmissionRate(20.0f);
smokeEmitter->SetStartLifetime(3.0f, 5.0f);
smokeEmitter->SetStartSpeed(1.0f, 2.0f);
smokeEmitter->SetStartSize(0.2f, 0.5f);
smokeEmitter->SetStartColor(Math::Vec4(0.3f, 0.3f, 0.3f, 0.8f), Math::Vec4(0.5f, 0.5f, 0.5f, 0.6f));

fireEffect->AddEmitter(smokeEmitter);

// Play the effect
fireEffect->Play();
```

### Explosion Effect

```cpp
// Create explosion effect
auto explosionEffect = particleManager->CreateEffect("Explosion");

// Main explosion burst
auto explosionEmitter = std::make_shared<BurstEmitter>();
explosionEmitter->SetShape(ParticleEmitter::Shape::Sphere);
explosionEmitter->SetShapeRadius(0.1f);
explosionEmitter->SetBurstCount(100);
explosionEmitter->SetStartLifetime(0.5f, 1.5f);
explosionEmitter->SetStartSpeed(5.0f, 15.0f);
explosionEmitter->SetStartSize(0.05f, 0.2f);
explosionEmitter->SetStartColor(Math::Vec4(1.0f, 1.0f, 0.8f, 1.0f), Math::Vec4(1.0f, 0.6f, 0.2f, 1.0f));

// Add gravity and drag
auto gravityModifier = std::make_shared<GravityModifier>();
gravityModifier->SetGravity(Math::Vec3(0.0f, -9.81f, 0.0f));

auto dragModifier = std::make_shared<DragModifier>();
dragModifier->SetDrag(2.0f);

explosionEmitter->AddModifier(gravityModifier);
explosionEmitter->AddModifier(dragModifier);

explosionEffect->AddEmitter(explosionEmitter);

// Debris particles
auto debrisEmitter = std::make_shared<BurstEmitter>();
debrisEmitter->SetShape(ParticleEmitter::Shape::Sphere);
debrisEmitter->SetBurstCount(50);
debrisEmitter->SetStartLifetime(2.0f, 4.0f);
debrisEmitter->SetStartSpeed(3.0f, 8.0f);
debrisEmitter->SetStartSize(0.02f, 0.1f);
debrisEmitter->SetStartColor(Math::Vec4(0.4f, 0.3f, 0.2f, 1.0f), Math::Vec4(0.6f, 0.5f, 0.3f, 1.0f));

// Add collision with ground
auto collisionModifier = std::make_shared<CollisionModifier>();
collisionModifier->AddCollisionPlane(Math::Vec4(0.0f, 1.0f, 0.0f, 0.0f));  // Ground plane
collisionModifier->SetBounciness(0.3f);
collisionModifier->SetFriction(0.8f);

debrisEmitter->AddModifier(gravityModifier);
debrisEmitter->AddModifier(collisionModifier);

explosionEffect->AddEmitter(debrisEmitter);
```

### Magic Spell Effect

```cpp
// Create magic spell effect
auto spellEffect = particleManager->CreateEffect("MagicSpell");

// Swirling particles around caster
auto swirlEmitter = std::make_shared<ContinuousEmitter>();
swirlEmitter->SetShape(ParticleEmitter::Shape::Circle);
swirlEmitter->SetShapeRadius(1.0f);
swirlEmitter->SetEmissionRate(30.0f);
swirlEmitter->SetStartLifetime(2.0f, 3.0f);
swirlEmitter->SetStartSpeed(0.5f, 1.0f);
swirlEmitter->SetStartSize(0.05f, 0.15f);
swirlEmitter->SetStartColor(Math::Vec4(0.5f, 0.2f, 1.0f, 1.0f), Math::Vec4(0.8f, 0.4f, 1.0f, 1.0f));

// Add orbital motion
auto orbitalModifier = std::make_shared<OrbitalModifier>();
orbitalModifier->SetCenter(Math::Vec3(0.0f, 1.0f, 0.0f));
orbitalModifier->SetRadius(1.5f);
orbitalModifier->SetSpeed(90.0f);  // degrees per second

swirlEmitter->AddModifier(orbitalModifier);
spellEffect->AddEmitter(swirlEmitter);

// Projectile trail
auto trailEmitter = std::make_shared<TrailEmitter>();
trailEmitter->SetTrailLength(2.0f);
trailEmitter->SetTrailWidth(0.1f);
trailEmitter->SetEmissionRate(50.0f);
trailEmitter->SetStartLifetime(0.5f, 1.0f);
trailEmitter->SetStartSize(0.03f, 0.08f);
trailEmitter->SetStartColor(Math::Vec4(0.3f, 0.1f, 0.8f, 1.0f), Math::Vec4(0.6f, 0.3f, 1.0f, 1.0f));

spellEffect->AddEmitter(trailEmitter);
```

### Weather Effects

```cpp
// Rain effect
auto rainEffect = particleManager->CreateEffect("Rain");

auto rainEmitter = std::make_shared<ContinuousEmitter>();
rainEmitter->SetShape(ParticleEmitter::Shape::Box);
rainEmitter->SetShapeSize(Math::Vec3(20.0f, 0.1f, 20.0f));
rainEmitter->SetLocalPosition(Math::Vec3(0.0f, 10.0f, 0.0f));
rainEmitter->SetEmissionRate(1000.0f);
rainEmitter->SetStartLifetime(2.0f, 3.0f);
rainEmitter->SetStartSpeed(8.0f, 12.0f);
rainEmitter->SetStartSize(0.01f, 0.02f);
rainEmitter->SetStartColor(Math::Vec4(0.7f, 0.8f, 1.0f, 0.8f), Math::Vec4(0.8f, 0.9f, 1.0f, 1.0f));

// Make raindrops fall straight down
auto rainVelocity = std::make_shared<VelocityOverLifetimeModifier>();
AnimationCurve downwardCurve;
downwardCurve.AddKeyframe({0.0f, 1.0f});
downwardCurve.AddKeyframe({1.0f, 1.0f});
rainVelocity->SetVelocityCurve(downwardCurve);

rainEmitter->AddModifier(rainVelocity);
rainEffect->AddEmitter(rainEmitter);

// Snow effect
auto snowEffect = particleManager->CreateEffect("Snow");

auto snowEmitter = std::make_shared<ContinuousEmitter>();
snowEmitter->SetShape(ParticleEmitter::Shape::Box);
snowEmitter->SetShapeSize(Math::Vec3(30.0f, 0.1f, 30.0f));
snowEmitter->SetLocalPosition(Math::Vec3(0.0f, 15.0f, 0.0f));
snowEmitter->SetEmissionRate(200.0f);
snowEmitter->SetStartLifetime(8.0f, 12.0f);
snowEmitter->SetStartSpeed(0.5f, 2.0f);
snowEmitter->SetStartSize(0.02f, 0.05f);
snowEmitter->SetStartColor(Math::Vec4(1.0f, 1.0f, 1.0f, 0.9f), Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

// Add gentle swaying motion
auto turbulenceModifier = std::make_shared<TurbulenceModifier>();
turbulenceModifier->SetStrength(0.5f);
turbulenceModifier->SetFrequency(0.1f);

snowEmitter->AddModifier(turbulenceModifier);
snowEffect->AddEmitter(snowEmitter);
```

## 🔮 Future Enhancements

### Planned Features (v1.2+)

- **Volumetric Particles**: 3D particle rendering
- **Fluid Simulation**: Liquid and gas dynamics
- **Mesh Particles**: Complex particle shapes
- **Audio Integration**: Sound-reactive particles
- **VR Optimization**: Efficient VR particle rendering
- **Mobile Optimization**: Reduced complexity for mobile

### Advanced Features

- **Machine Learning**: AI-generated particle behaviors
- **Real-time Raytracing**: Ray-traced particle lighting
- **Temporal Upsampling**: High-quality motion blur
- **Adaptive LOD**: Dynamic quality adjustment
- **Cloud Simulation**: Realistic cloud rendering

## 📚 Best Practices

### Performance Optimization

1. **Use GPU Simulation**: Enable compute shaders for large particle counts
2. **Implement LOD**: Reduce particle count for distant effects
3. **Batch Rendering**: Group similar particles together
4. **Cull Invisible Particles**: Don't update off-screen particles
5. **Optimize Textures**: Use texture atlases and compression

### Visual Quality

1. **Layer Effects**: Combine multiple emitters for complex effects
2. **Use Proper Blending**: Choose appropriate blend modes
3. **Add Variation**: Randomize particle properties
4. **Consider Lighting**: Make particles respond to scene lighting
5. **Optimize Sorting**: Sort particles for proper transparency

---

The Particle Effects System in Game Engine Kiro v1.1 provides comprehensive tools for creating stunning visual effects that enhance the visual appeal and immersion of games, from subtle ambient effects to spectacular explosions and magical spells.

**Game Engine Kiro v1.1** - Bringing visual magic to life.
