# Game Engine Kiro - Windows Setup

## Guia Rápido para Windows

### Pré-requisitos

1. **Git** - [Download aqui](https://git-scm.com/download/win)
2. **CMake** - [Download aqui](https://cmake.org/download/)
3. **Visual Studio 2019+** ou **Build Tools** - [Download aqui](https://visualstudio.microsoft.com/downloads/)

### Instalação em 3 Passos

#### 1. Clone o Repositório

```cmd
git clone https://github.com/yourusername/GameEngineKiro.git
cd GameEngineKiro
```

#### 2. Teste o Sistema (Opcional)

```cmd
test_setup.bat
```

#### 3. Instale Dependências e Compile

```cmd
setup_dependencies.bat
build.bat
```

### Pronto!

O jogo estará em: `build\Release\GameExample.exe`

---

## Detalhes Técnicos

### O que o `setup_dependencies.bat` faz:

1. **Baixa o vcpkg** (gerenciador de pacotes C++)
2. **Instala as dependências**:
   - GLFW3 (janelas e input)
   - GLM (matemática 3D)
   - GLAD (OpenGL)
   - STB (carregamento de imagens)
   - nlohmann-json (JSON)
   - fmt (formatação de strings)
   - Assimp (modelos 3D)
   - OpenAL (áudio 3D)
   - Bullet3 (física)
   - Lua (scripting)

### O que o `build.bat` faz:

1. **Verifica** se as dependências estão instaladas
2. **Configura** o CMake com vcpkg
3. **Compila** o projeto
4. **Copia** os assets para o diretório de build
5. **Oferece** para executar o jogo

### Estrutura após Build:

```
GameEngineKiro/
├── build/
│   └── Release/
│       ├── GameExample.exe    ← Seu jogo!
│       ├── GameEngineKiro.lib ← Biblioteca do engine
│       └── assets/            ← Shaders e recursos
├── vcpkg/                     ← Dependências
└── ...
```

---

## Solução de Problemas

### [ERRO] "Git não encontrado"

- Instale o Git: https://git-scm.com/download/win
- Reinicie o terminal após a instalação

### [ERRO] "CMake não encontrado"

- Instale o CMake: https://cmake.org/download/
- Marque "Add CMake to PATH" durante a instalação

### [ERRO] "Visual Studio não encontrado"

- Instale Visual Studio Community (gratuito)
- Ou instale apenas Build Tools for Visual Studio
- Certifique-se de incluir "C++ build tools"

### [ERRO] "vcpkg falhou ao instalar"

- Verifique sua conexão com a internet
- Execute como administrador se necessário
- Tente deletar a pasta `vcpkg` e executar novamente

### [ERRO] "Build falhou"

- Certifique-se de que todas as dependências foram instaladas
- Tente executar de um "Developer Command Prompt"
- Verifique se há espaços no caminho do projeto

---

## Comandos Úteis

### Limpar e Recompilar

```cmd
rmdir /s /q build
build.bat
```

### Reinstalar Dependências

```cmd
rmdir /s /q vcpkg
setup_dependencies.bat
```

### Build de Debug

```cmd
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug
```

### Executar o Jogo

```cmd
cd build\Release
GameExample.exe
```

---

## Próximos Passos

1. **Explore o código** em `examples/main.cpp`
2. **Modifique os shaders** em `assets/shaders/`
3. **Leia a documentação** completa no `README.md`
4. **Comece seu jogo!**

---

## Suporte

Se tiver problemas:

1. Execute `test_setup.bat` para verificar o sistema
2. Verifique os logs de erro detalhados
3. Consulte o `SETUP.md` para mais detalhes
4. Abra uma issue no GitHub

**Boa sorte com seu jogo!**
