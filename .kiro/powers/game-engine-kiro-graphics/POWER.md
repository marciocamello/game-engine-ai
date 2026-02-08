---
name: "Game Engine Kiro - Graphics System"
version: "1.0.0"
description: "Specialized knowledge for Game Engine Kiro's graphics and rendering systems, including skeletal animation, shaders, and OpenGL integration"
keywords:
  [
    "graphics",
    "rendering",
    "opengl",
    "shaders",
    "skeletal",
    "animation",
    "mesh",
    "vertex",
    "fragment",
    "skinning",
    "bones",
    "matrices",
  ]
author: "Game Engine Kiro Team"
category: "Game Development"
---

# Game Engine Kiro - Graphics System Power

Este Power fornece conhecimento especializado sobre o sistema gráfico do Game Engine Kiro, incluindo renderização esquelética, shaders, e integração OpenGL.

## Quando Ativar Este Power

Este Power é ativado automaticamente quando você menciona:

- **Renderização**: "rendering", "render", "draw", "graphics"
- **Shaders**: "shader", "vertex", "fragment", "GLSL", "skinning"
- **Animação Esquelética**: "skeletal", "bones", "skinning", "animation"
- **OpenGL**: "opengl", "buffer", "VBO", "VAO", "UBO"
- **Mesh**: "mesh", "vertex", "primitive", "geometry"

## Arquitetura do Sistema Gráfico

### Componentes Principais

1. **PrimitiveRenderer**: Interface principal para renderização
2. **SkinnedMeshRenderer**: Renderizador especializado para meshes esqueléticas
3. **BoneMatrixManager**: Gerenciamento de matrizes de ossos
4. **SkinningShaderManager**: Gerenciamento de shaders de skinning
5. **SkeletalMeshData**: Estrutura de dados para meshes esqueléticas

### Fluxo de Renderização Esquelética

```cpp
// 1. Preparar dados esqueléticos
SkeletalMeshData meshData;
meshData.LoadFromFBX("character.fbx");

// 2. Calcular matrizes de ossos
BoneMatrixManager boneManager;
boneManager.CalculateBoneMatrices(skeleton, animation, time);

// 3. Renderizar mesh esquelética
SkinnedMeshRenderer renderer;
renderer.DrawSkinnedMesh(meshData, boneManager.GetMatrices());
```

## Padrões de Desenvolvimento

### 1. Estrutura de Arquivos

```
include/Graphics/
├── SkeletalMeshData.h          # Dados de mesh esquelética
├── BoneMatrixManager.h         # Gerenciamento de matrizes
├── SkinningShaderManager.h     # Gerenciamento de shaders
└── SkinnedMeshRenderer.h       # Renderizador principal

src/Graphics/
├── SkeletalMeshData.cpp
├── BoneMatrixManager.cpp
├── SkinningShaderManager.cpp
└── SkinnedMeshRenderer.cpp

assets/shaders/
├── skinned.vert               # Vertex shader para skinning
└── skinned.frag               # Fragment shader para skinning
```

### 2. Convenções de Nomenclatura

- **Classes**: PascalCase (ex: `SkeletalMeshData`, `BoneMatrixManager`)
- **Métodos**: PascalCase (ex: `CalculateBoneMatrices()`, `DrawSkinnedMesh()`)
- **Variáveis membro**: m\_ prefix (ex: `m_boneMatrices`, `m_shaderProgram`)
- **Constantes**: UPPER_SNAKE_CASE (ex: `MAX_BONES_PER_SKELETON`)

### 3. Padrões de Shader

#### Vertex Shader (skinned.vert)

```glsl
#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in ivec4 aBoneIndices;
layout (location = 4) in vec4 aBoneWeights;

uniform mat4 uBoneMatrices[128];
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    // Skinning calculation
    mat4 boneTransform =
        aBoneWeights.x * uBoneMatrices[aBoneIndices.x] +
        aBoneWeights.y * uBoneMatrices[aBoneIndices.y] +
        aBoneWeights.z * uBoneMatrices[aBoneIndices.z] +
        aBoneWeights.w * uBoneMatrices[aBoneIndices.w];

    vec4 skinnedPosition = boneTransform * vec4(aPosition, 1.0);
    gl_Position = uProjection * uView * uModel * skinnedPosition;
}
```

## Resolução de Problemas Comuns

### Erro: "Cannot draw skinned mesh - invalid mesh or shader"

**Causa**: Mesh não possui dados esqueléticos ou shader não está carregado
**Solução**:

1. Verificar se `SkeletalMeshData` foi inicializada corretamente
2. Confirmar que shader de skinning foi compilado
3. Validar que matrizes de ossos foram calculadas

### Performance de Renderização Esquelética

**Otimizações**:

1. **UBO para matrizes**: Use Uniform Buffer Objects para matrizes de ossos
2. **Dirty flagging**: Só atualize matrizes quando necessário
3. **Batching**: Agrupe meshes com mesmo esqueleto
4. **GPU skinning**: Mantenha cálculos no vertex shader

### Integração com Sistema Existente

**Compatibilidade**:

- Manter `PrimitiveRenderer` para meshes estáticas
- Estender com `DrawSkinnedMesh()` para meshes esqueléticas
- Preservar sistema de materiais existente
- Integrar com `FBXLoader` para dados esqueléticos

## Testes e Validação

### Testes de Propriedade (Property-Based Testing)

1. **Property 1**: Skinned Mesh Rendering Success
2. **Property 2**: Vertex Skinning Transformation
3. **Property 3**: Bone Matrix Update Cycle
4. **Property 4**: Multi-Mesh Independence

### Testes Unitários

- Compilação de shaders
- Validação de dados esqueléticos
- Cálculo de matrizes de ossos
- Gerenciamento de recursos GPU

## Comandos de Build e Teste

```powershell
# Build individual para desenvolvimento rápido
.\scripts\build_unified.bat --tests SkeletalrenderingTest

# Executar teste específico
.\build\Release\SkeletalrenderingTest.exe

# Build completo e todos os testes
.\scripts\build_unified.bat --tests
.\scripts\run_tests.bat
```

## Recursos Adicionais

- **Documentação**: `docs/3d-model-loading.md`
- **Exemplos**: `examples/character_controller_test.cpp`
- **Assets**: `assets/meshes/XBot.fbx` (modelo de teste)
- **Shaders**: `assets/shaders/skinned.vert`, `assets/shaders/skinned.frag`

---

**Lembre-se**: Este sistema resolve o erro crítico "PrimitiveRenderer: Cannot draw skinned mesh" implementando renderização esquelética completa com suporte a até 128 ossos por esqueleto e 4 influências por vértice.
