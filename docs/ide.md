# Configuração da IDE para Game Engine Kiro

Este documento explica como configurar o VS Code para trabalhar com o projeto Game Engine Kiro, resolvendo problemas de highlight e IntelliSense.

## Configuração Automática

Execute o script de configuração automática:

```powershell
.\configure_ide.ps1
```

Este script irá:

- Criar as configurações do VS Code
- Configurar o CMake com vcpkg
- Gerar o `compile_commands.json`
- Preparar o ambiente para IntelliSense

## Arquivos de Configuração Criados

O script cria os seguintes arquivos em `.vscode/`:

- **settings.json**: Configurações gerais do workspace, incluindo CMake e C++
- **c_cpp_properties.json**: Configurações específicas do IntelliSense C/C++
- **tasks.json**: Tarefas de build e desenvolvimento
- **launch.json**: Configurações de debug
- **extensions.json**: Extensões recomendadas

## Resolução de Problemas

### Problema: Highlight do CMakeLists.txt não funciona

**Causa**: Extensão CMake não instalada ou não configurada.

**Solução:**

1. Instale a extensão "CMake" (twxs.cmake)
2. Instale a extensão "CMake Tools" (ms-vscode.cmake-tools)
3. Reinicie o VS Code
4. O arquivo `CMakeLists.txt` deve ter highlight automático

### Problema: IntelliSense C++ não funciona

**Causa**: Falta do arquivo `compile_commands.json` ou configuração incorreta.

**Solução Automática:**

```powershell
.\reload_intellisense.ps1
```

**Solução Manual:**

1. Ctrl+Shift+P → "C/C++: Reset IntelliSense Database"
2. Ctrl+Shift+P → "CMake: Reset CMake Tools Extension State"
3. Ctrl+Shift+P → "Developer: Reload Window"

### Problema: CMake não encontra dependências

**Causa**: vcpkg não configurado ou dependências não instaladas.

**Solução:**

1. Execute: `.\setup_dependencies.bat`
2. Verifique se existe `vcpkg/vcpkg.exe`
3. Execute: `.\configure_ide.ps1`
4. Verifique se existe `vcpkg_installed/x64-windows/include/`

### Problema: Erros de compilação no IntelliSense

**Causa**: Caminhos de include incorretos ou defines faltando.

**Solução:**

1. Verifique se o build foi executado: `.\build.bat`
2. Confirme que existe `build/compile_commands.json`
3. Execute: `.\reload_intellisense.ps1`

## Extensões Necessárias

As seguintes extensões são essenciais e serão sugeridas automaticamente:

### Obrigatórias

- **C/C++** (ms-vscode.cpptools) - IntelliSense para C++
- **CMake Tools** (ms-vscode.cmake-tools) - Integração com CMake
- **CMake** (twxs.cmake) - Syntax highlighting para CMake

### Recomendadas

- **C/C++ Extension Pack** (ms-vscode.cpptools-extension-pack)
- **C/C++ Themes** (ms-vscode.cpptools-themes)
- **Code Runner** (formulahendry.code-runner)

## Configuração Manual Passo a Passo

Se a configuração automática não funcionar:

### 1. Selecionar Kit do CMake

```
Ctrl+Shift+P → "CMake: Select a Kit"
Escolha: "Visual Studio Community 2022 Release - amd64"
```

### 2. Configurar CMake

```
Ctrl+Shift+P → "CMake: Configure"
```

### 3. Resetar IntelliSense

```
Ctrl+Shift+P → "C/C++: Reset IntelliSense Database"
```

### 4. Recarregar Janela

```
Ctrl+Shift+P → "Developer: Reload Window"
```

## Estrutura de Build

O projeto usa a seguinte estrutura:

```
GameEngineKiro/
├── build/                     # Diretório de build (gerado)
│   ├── compile_commands.json  # Comandos de compilação para IntelliSense
│   └── Release/               # Executáveis de release
├── include/                   # Headers da engine
│   ├── Core/                  # Sistema core
│   ├── Graphics/              # Sistema gráfico
│   ├── Physics/               # Sistema de física
│   └── ...
├── src/                       # Código fonte da engine
├── examples/                  # Exemplos de uso
├── vcpkg/                     # Gerenciador de dependências
├── vcpkg_installed/           # Dependências instaladas
└── .vscode/                   # Configurações do VS Code
```

## Atalhos Úteis

- **F7**: Build (CMake: Build)
- **Ctrl+F5**: Run without debugging
- **F5**: Debug
- **Ctrl+Shift+P**: Command Palette
- **Ctrl+Shift+B**: Run Build Task

## Troubleshooting Avançado

### Cache do IntelliSense Corrompido

Se o IntelliSense continuar com problemas:

1. Feche o VS Code completamente
2. Delete a pasta `.vscode/ipch` (se existir)
3. Execute: `.\reload_intellisense.ps1`
4. Abra o VS Code novamente: `code .`

### Problemas com vcpkg

Se as dependências não forem encontradas:

1. Verifique se `vcpkg.exe` existe em `vcpkg/`
2. Execute: `.\setup_dependencies.bat`
3. Verifique se existe `vcpkg_installed/x64-windows/`
4. Reconfigure: `.\configure_ide.ps1`

### Problemas com Visual Studio

Certifique-se de ter instalado:

- **Visual Studio 2022 Community** (ou superior)
- **Workload**: "Desktop development with C++"
- **Windows 10/11 SDK**
- **CMake tools for Visual Studio**

### Verificação da Configuração

Execute para verificar se tudo está configurado:

```powershell
.\check_ide_setup.ps1
```

Este script verifica:

- Arquivos de configuração do VS Code
- Estrutura do projeto
- Dependências instaladas
- Build executado
- Headers principais

## Comandos Úteis

### Reconfigurar Completamente

```powershell
# Limpar build
rmdir /s /q build

# Reconfigurar IDE
.\configure_ide.ps1

# Abrir VS Code
code .
```

### Debug de Configuração

```powershell
# Verificar configuração
.\check_ide_setup.ps1

# Recarregar IntelliSense
.\reload_intellisense.ps1
```

### Build e Execução

```powershell
# Build completo
.\build.bat

# Executar jogo
.\build\Release\GameExample.exe
```

## Suporte

Se os problemas persistirem:

1. Verifique se todas as dependências estão instaladas
2. Confirme que o Visual Studio 2022 está instalado corretamente
3. Execute os scripts de configuração na ordem correta
4. Verifique os logs de erro no VS Code (Output → C/C++ ou CMake Tools)
