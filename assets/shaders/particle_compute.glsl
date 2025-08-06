#version 460 core

// Particle System Compute Shader
// Demonstrates compute shader capabilities for GPU-based particle simulation
// This shader updates particle positions, velocities, and lifetimes

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

// Particle structure
struct Particle {
    vec3 position;
    float life;
    vec3 velocity;
    float maxLife;
    vec4 color;
    float size;
    float mass;
    vec2 padding; // Align to 16 bytes
};

// Storage buffers
layout(std430, binding = 0) restrict buffer ParticleBuffer {
    Particle particles[];
};

layout(std430, binding = 1) restrict buffer EmitterBuffer {
    vec3 emitterPosition;
    float emissionRate;
    vec3 emitterVelocity;
    float particleLifetime;
    vec4 startColor;
    vec4 endColor;
    float startSize;
    float endSize;
    vec2 padding;
} emitter;

// Uniforms
uniform float u_deltaTime;
uniform float u_time;
uniform vec3 u_gravity;
uniform float u_damping;
uniform int u_maxParticles;

// Random number generation
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 randomVec3(vec3 seed) {
    return vec3(
        random(seed.xy + u_time),
        random(seed.yz + u_time * 1.1),
        random(seed.zx + u_time * 1.3)
    ) * 2.0 - 1.0;
}

void main() {
    uint index = gl_GlobalInvocationID.x;
    
    // Bounds check
    if (index >= u_maxParticles) {
        return;
    }
    
    Particle particle = particles[index];
    
    // Update particle life
    particle.life -= u_deltaTime;
    
    // If particle is dead, respawn it
    if (particle.life <= 0.0) {
        // Reset particle
        particle.position = emitter.emitterPosition + randomVec3(vec3(index, u_time, 0.0)) * 0.5;
        particle.velocity = emitter.emitterVelocity + randomVec3(vec3(index, u_time, 1.0)) * 2.0;
        particle.life = emitter.particleLifetime;
        particle.maxLife = emitter.particleLifetime;
        particle.size = emitter.startSize;
        particle.mass = 1.0 + random(vec2(index, u_time)) * 0.5;
        particle.color = emitter.startColor;
    } else {
        // Update living particle
        
        // Apply gravity
        particle.velocity += u_gravity * u_deltaTime / particle.mass;
        
        // Apply damping
        particle.velocity *= (1.0 - u_damping * u_deltaTime);
        
        // Update position
        particle.position += particle.velocity * u_deltaTime;
        
        // Update visual properties based on life
        float lifeRatio = particle.life / particle.maxLife;
        
        // Interpolate color
        particle.color = mix(emitter.endColor, emitter.startColor, lifeRatio);
        
        // Interpolate size
        particle.size = mix(emitter.endSize, emitter.startSize, lifeRatio);
        
        // Add some turbulence
        vec3 turbulence = randomVec3(particle.position + u_time) * 0.1;
        particle.velocity += turbulence * u_deltaTime;
    }
    
    // Write back to buffer
    particles[index] = particle;
}