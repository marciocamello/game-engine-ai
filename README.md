# Game Engine Kiro

Um engine de jogos 3D moderno desenvolvido em C++ para criação de jogos third-person open world, com suporte para PC e futuras expansões para consoles.

## Características Principais

### Arquitetura Modular

- **Core**: Utilitários básicos, gerenciamento de memória, logging, biblioteca matemática
- **Graphics**: Renderização OpenGL/Vulkan, gerenciamento de cena, pipeline de renderização
- **Resource**: Carregamento e gerenciamento de assets (modelos, texturas, shaders, sons)
- **Physics**: Simulação física (rigid body, soft body, character physics)
- **Audio**: Engine de áudio 3D espacial
- **Input**: Gerenciamento de entrada (teclado, mouse, gamepad)
- **Scripting**: Sistema de scripts (Lua integration)

### Recursos Avançados

- Suporte para OpenGL e Vulkan
- Preparado para NVIDIA DLSS e AMD FSR
- Sistema de câmera third-person
- Pipeline de assets
- Sistema de binding de input
- Logging avançado

## Requisitos

### Dependências

- CMake 3.20+
- C++20 compiler
- OpenGL 4.6+
- GLFW3
- GLM (OpenGL Mathematics)

### Opcional

- Vulkan SDK (para renderer Vulkan)
- Lua (para scripting)

## Compilação

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Opções de Compilação

```bash
# Habilitar Vulkan
cmake -DENABLE_VULKAN=ON ..

# Habilitar DLSS
cmake -DENABLE_DLSS=ON ..

# Habilitar FSR
cmake -DENABLE_FSR=ON ..
```

## Uso Básico

```cpp
#include "Core/Engine.h"
#include "Graphics/Camera.h"

using namespace GameEngine;

int main() {
    Engine engine;

    // Inicializar engine
    if (!engine.Initialize()) {
        return -1;
    }

    // Configurar câmera
    auto camera = std::make_unique<Camera>(CameraType::Perspective);
    camera->SetPosition(Math::Vec3(0.0f, 2.0f, 5.0f));
    camera->SetPerspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);

    engine.GetRenderer()->SetCamera(camera.get());

    // Loop principal
    engine.Run();

    return 0;
}
```

## Sistema de Input

```cpp
// Configurar bindings
auto* input = engine.GetInput();
input->BindAction("move_forward", KeyCode::W);
input->BindAction("jump", KeyCode::Space);
input->BindAction("fire", MouseButton::Left);

// Verificar input no loop de update
if (input->IsActionPressed("jump")) {
    // Player jumped
}

if (input->IsActionDown("move_forward")) {
    // Player is moving forward
}
```

## Sistema de Recursos

```cpp
auto* resourceManager = engine.GetResourceManager();

// Carregar assets
auto texture = resourceManager->Load<Texture>("textures/player.png");
auto model = resourceManager->Load<Model>("models/character.fbx");
auto sound = resourceManager->Load<AudioClip>("sounds/footstep.wav");
```

## Sistema de Física

```cpp
auto* physics = engine.GetPhysics();

// Criar mundo físico
auto world = physics->CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
physics->SetActiveWorld(world);

// Criar rigid body
RigidBody bodyDesc;
bodyDesc.position = Math::Vec3(0.0f, 10.0f, 0.0f);
bodyDesc.mass = 1.0f;

CollisionShape shape;
shape.type = CollisionShape::Box;
shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);

uint32_t bodyId = physics->CreateRigidBody(bodyDesc, shape);
```

## Sistema de Áudio

```cpp
auto* audio = engine.GetAudio();

// Carregar e reproduzir som
auto clip = audio->LoadAudioClip("sounds/background_music.ogg");
uint32_t sourceId = audio->CreateAudioSource();

audio->SetAudioSourcePosition(sourceId, Math::Vec3(0.0f, 0.0f, 0.0f));
audio->PlayAudioSource(sourceId, clip);
```

## Estrutura de Diretórios

```
GameEngineKiro/
├── include/           # Headers do engine
│   ├── Core/         # Sistema core
│   ├── Graphics/     # Sistema gráfico
│   ├── Resource/     # Gerenciamento de recursos
│   ├── Physics/      # Sistema de física
│   ├── Audio/        # Sistema de áudio
│   ├── Input/        # Sistema de input
│   └── Scripting/    # Sistema de scripting
├── src/              # Implementações
├── examples/         # Exemplos de uso
├── assets/           # Assets do jogo
└── CMakeLists.txt    # Configuração CMake
```

## Roadmap

### Versão 1.0 (Atual)

- [x] Arquitetura básica do engine
- [x] Sistema de renderização OpenGL
- [x] Sistema de input
- [x] Sistema de áudio básico
- [x] Sistema de física básico
- [x] Gerenciamento de recursos

### Versão 1.1

- [ ] Implementação completa do renderer Vulkan
- [ ] Sistema de shaders avançado
- [ ] Carregamento de modelos 3D (FBX, OBJ, GLTF)
- [ ] Sistema de animação skeletal

### Versão 1.2

- [ ] Suporte DLSS/FSR
- [ ] Sistema de streaming de mundo aberto
- [ ] Sistema de IA (pathfinding, behavior trees)
- [ ] Integração completa com Lua

### Versão 1.5 (Physics Upgrade)

- [ ] Integração NVIDIA PhysX
- [ ] Sistema de seleção de backend de física (PhysX/Bullet)
- [ ] Comparação de performance e benchmarks
- [ ] Abstração cross-backend para física
- [ ] GPU acceleration com PhysX
- [ ] Fallback automático para Bullet Physics

### Versão 2.0

- [ ] Suporte para consoles
- [ ] Sistema de networking
- [ ] Editor visual
- [ ] Sistema de partículas avançado

## Contribuição

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## Licença

Este projeto está licenciado sob a MIT License - veja o arquivo [LICENSE](LICENSE) para detalhes.

## Contato

Game Engine Kiro - Desenvolvido para criação de jogos 3D modernos

Project Link: [https://github.com/yourusername/GameEngineKiro](https://github.com/yourusername/GameEngineKiro)
