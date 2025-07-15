# Game Engine Kiro - Setup Guide

Este guia irá ajudá-lo a configurar e compilar o Game Engine Kiro em diferentes plataformas.

## Requisitos Mínimos

- **CMake 3.16+**
- **C++20 compatible compiler**
  - Windows: Visual Studio 2019+ ou MinGW-w64
  - Linux: GCC 10+ ou Clang 10+
  - macOS: Xcode 12+ ou Clang 10+
- **Git** (para baixar dependências)

## Setup Automático (Recomendado)

### Windows

1. **Clone o repositório:**

   ```cmd
   git clone https://github.com/yourusername/GameEngineKiro.git
   cd GameEngineKiro
   ```

2. **Execute o setup automático:**
   ```cmd
   setup_dependencies.bat
   ```
3. **Compile o projeto:**
   ```cmd
   build.bat
   ```

### Linux/macOS

1. **Clone o repositório:**

   ```bash
   git clone https://github.com/yourusername/GameEngineKiro.git
   cd GameEngineKiro
   ```

2. **Execute o setup automático:**
   ```bash
   chmod +x setup_dependencies.sh build.sh
   ./setup_dependencies.sh
   ```
3. **Compile o projeto:**
   ```bash
   ./build.sh
   ```

## Setup Manual

Se preferir instalar as dependências manualmente:

### Windows (vcpkg)

```cmd
# Instalar vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

# Instalar dependências
vcpkg install glfw3:x64-windows glm:x64-windows glad:x64-windows
vcpkg install assimp:x64-windows openal-soft:x64-windows bullet3:x64-windows
vcpkg install lua:x64-windows nlohmann-json:x64-windows fmt:x64-windows

# Voltar ao diretório do projeto
cd ..

# Compilar
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Ubuntu/Debian

```bash
# Instalar dependências
sudo apt-get update
sudo apt-get install -y build-essential cmake git
sudo apt-get install -y libglfw3-dev libglm-dev libgl1-mesa-dev
sudo apt-get install -y libassimp-dev libopenal-dev libbullet-dev
sudo apt-get install -y liblua5.4-dev nlohmann-json3-dev libfmt-dev

# Compilar
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### macOS (Homebrew)

```bash
# Instalar Homebrew (se não tiver)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Instalar dependências
brew install cmake glfw glm assimp openal-soft bullet lua nlohmann-json fmt

# Compilar
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

## Dependências

### Obrigatórias

- **GLFW3**: Gerenciamento de janelas e input
- **GLM**: Biblioteca matemática para OpenGL
- **OpenGL**: API gráfica
- **GLAD**: Carregador de extensões OpenGL

### Opcionais

- **Assimp**: Carregamento de modelos 3D
- **OpenAL**: Sistema de áudio 3D
- **Bullet Physics**: Motor de física
- **Lua**: Sistema de scripting
- **nlohmann/json**: Parsing de JSON
- **fmt**: Formatação de strings

## Verificação da Instalação

Após a compilação bem-sucedida, você deve ter:

```
build/
├── GameExample(.exe)     # Executável principal
├── assets/               # Assets copiados
│   └── shaders/
└── libGameEngineKiro.a   # Biblioteca do engine
```

## Executando o Exemplo

### Windows

```cmd
cd build\Release
GameExample.exe
```

### Linux/macOS

```bash
cd build
./GameExample
```

## Opções de Compilação

Você pode personalizar a compilação com estas opções:

```bash
# Habilitar Vulkan (experimental)
cmake .. -DENABLE_VULKAN=ON

# Desabilitar OpenGL
cmake .. -DENABLE_OPENGL=OFF

# Usar bibliotecas do sistema (Linux/macOS)
cmake .. -DUSE_VCPKG=OFF

# Build de Debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## Solução de Problemas

### Erro: "GLFW3 not found"

- **Windows**: Execute `setup_dependencies.bat` ou instale via vcpkg
- **Linux**: `sudo apt-get install libglfw3-dev`
- **macOS**: `brew install glfw`

### Erro: "OpenGL not found"

- **Windows**: Instale drivers gráficos atualizados
- **Linux**: `sudo apt-get install libgl1-mesa-dev`
- **macOS**: OpenGL já está incluído no sistema

### Erro de compilação C++20

- Certifique-se de usar um compilador compatível com C++20
- **Windows**: Visual Studio 2019+ ou GCC 10+
- **Linux**: GCC 10+ ou Clang 10+
- **macOS**: Xcode 12+ ou Clang 10+

### Problemas com vcpkg

- Certifique-se de ter Git instalado
- Execute `git clean -fdx` no diretório vcpkg se houver problemas
- Tente executar `vcpkg integrate install`

## Próximos Passos

Após a instalação bem-sucedida:

1. Leia o [README.md](README.md) para entender a arquitetura
2. Explore os exemplos em `examples/`
3. Consulte a documentação da API em `docs/` (quando disponível)
4. Comece a desenvolver seu jogo!

## Suporte

Se encontrar problemas:

1. Verifique se todas as dependências estão instaladas
2. Consulte os logs de erro detalhados
3. Abra uma issue no GitHub com informações do sistema e logs
4. Consulte a documentação das dependências individuais

---

**Boa sorte com o desenvolvimento do seu jogo! 🎮**
