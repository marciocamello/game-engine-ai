#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec4 uColor;
uniform bool uHasTexture;
uniform sampler2D uTexture;

// Basic lighting
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

void main()
{
    vec4 baseColor = uColor;
    
    if(uHasTexture)
    {
        baseColor = texture(uTexture, TexCoord) * uColor;
    }
    
    // Simple Phong lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * uLightColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * uLightColor;
    
    vec3 result = (ambient + diffuse + specular) * baseColor.rgb;
    FragColor = vec4(result, baseColor.a);
}