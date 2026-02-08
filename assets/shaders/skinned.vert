#version 460 core

// Vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in ivec4 aBoneIds;
layout (location = 5) in vec4 aWeights;

// Constants for skeletal animation
const int MAX_BONES = 128;
const int MAX_BONE_INFLUENCE = 4;

// Transformation matrices
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

// Bone matrices for skeletal animation
uniform mat4 uBoneMatrices[MAX_BONES];

// Output to fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Tangent;
out vec3 Bitangent;
out vec4 WorldPos;

void main()
{
    // Initialize accumulated transformation data
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    vec3 totalTangent = vec3(0.0f);
    
    // Apply bone transformations with weighted blending
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        // Skip invalid bone indices
        if(aBoneIds[i] == -1) 
            continue;
            
        // Validate bone index is within bounds
        if(aBoneIds[i] >= MAX_BONES) 
        {
            // Fallback to original vertex data if bone index is invalid
            totalPosition = vec4(aPos, 1.0f);
            totalNormal = aNormal;
            totalTangent = aTangent;
            break;
        }
        
        // Get bone transformation matrix
        mat4 boneTransform = uBoneMatrices[aBoneIds[i]];
        float weight = aWeights[i];
        
        // Transform position
        vec4 localPosition = boneTransform * vec4(aPos, 1.0f);
        totalPosition += localPosition * weight;
        
        // Transform normal (using upper-left 3x3 matrix)
        mat3 boneNormalMatrix = mat3(boneTransform);
        vec3 localNormal = boneNormalMatrix * aNormal;
        totalNormal += localNormal * weight;
        
        // Transform tangent for normal mapping support
        vec3 localTangent = boneNormalMatrix * aTangent;
        totalTangent += localTangent * weight;
    }
    
    // Calculate world position
    WorldPos = uModel * totalPosition;
    FragPos = WorldPos.xyz;
    
    // Calculate final clip space position
    gl_Position = uProjection * uView * WorldPos;
    
    // Transform normals and tangents to world space
    Normal = normalize(uNormalMatrix * normalize(totalNormal));
    Tangent = normalize(uNormalMatrix * normalize(totalTangent));
    
    // Calculate bitangent for complete TBN matrix
    Bitangent = normalize(cross(Normal, Tangent));
    
    // Pass through texture coordinates
    TexCoord = aTexCoord;
}