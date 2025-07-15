# Game Engine Kiro - Quick Start

## Para Começar Rapidamente (Windows)

### 1. Teste o Sistema

```cmd
test_setup.bat
```

### 2. Instale Dependências

```cmd
setup_dependencies.bat
```

### 3. Compile o Projeto

```cmd
build.bat
```

### 4. Execute o Jogo

```cmd
cd build\Release
GameExample.exe
```

---

## O que Você Tem Agora

✓ **Engine 3D Completo** com arquitetura modular
✓ **Sistema de Renderização** OpenGL com shaders PBR
✓ **Sistema de Input** (teclado, mouse, gamepad)
✓ **Sistema de Áudio** 3D espacial
✓ **Sistema de Física** (Bullet Physics + PhysX ready)
✓ **Gerenciamento de Recursos** automático
✓ **Sistema de Scripting** (Lua ready)
✓ **Câmera Third-Person** configurada

---

## Estrutura do Projeto

```
GameEngineKiro/
├── include/          # Headers do engine
├── src/             # Código fonte
├── examples/        # Exemplo de jogo
├── assets/          # Shaders e recursos
├── build/           # Arquivos compilados
├── vcpkg/           # Dependências
└── docs/            # Documentação (PHYSICS_STRATEGY.md)
```

---

## Escolha de Backend de Física

### **Atual (v1.0):**

```cpp
// Bullet Physics (implementado)
PhysicsEngine physics;
physics.Initialize();  // Usa Bullet automaticamente
```

### **Futuro (v1.5):**

```cpp
// Seleção de backend
PhysicsConfig config;
config.backend = PhysicsBackend::PhysX;    // Para performance
config.backend = PhysicsBackend::Bullet;   // Para compatibilidade
config.backend = PhysicsBackend::Auto;     // Seleção automática

PhysicsEngine physics;
physics.Initialize(config);
```

---

## Próximos Passos

1. **Explore** o código em `examples/main.cpp`
2. **Modifique** os shaders em `assets/shaders/`
3. **Leia** `PHYSICS_STRATEGY.md` para entender a arquitetura
4. **Implemente** sua lógica de jogo
5. **Consulte** o README.md para detalhes avançados

---

## Comandos Úteis

### Recompilar

```cmd
build.bat
```

### Limpar Build

```cmd
rmdir /s /q build
build.bat
```

### Reinstalar Dependências

```cmd
rmdir /s /q vcpkg
setup_dependencies.bat
```

### Compilar com Features Específicas

```cmd
# Apenas física básica
vcpkg install --feature-flags=physics

# Preparar para PhysX (futuro)
vcpkg install --feature-flags=physx
```

---

## Configurações Recomendadas

### **Para Projetos Indie:**

- Use **Bullet Physics** (atual implementação)
- Foque na jogabilidade e prototipagem rápida
- Upgrade para PhysX quando necessário

### **Para Projetos AAA:**

- Planeje migração para **NVIDIA PhysX** (v1.5)
- Implemente desde o início pensando em performance
- Use GPU acceleration quando disponível

### **Para Máxima Compatibilidade:**

- Mantenha **Bullet Physics** como fallback
- Teste em hardware variado
- Implemente detecção automática de capabilities

---

**Seu engine está pronto para criar jogos 3D com a melhor estratégia de física da indústria!**

_Documentação completa em: `PHYSICS_STRATEGY.md`_
