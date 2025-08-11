# Game Engine Kiro - Test Scripts

Este diretório contém scripts para automatizar a execução de testes no Game Engine Kiro.

## Scripts Disponíveis

### 🏗️ `build.bat` - Build do Projeto

Script principal para compilar o projeto.

**Uso:**

```cmd
scripts\build.bat
```

### 🚀 `run_tests.bat` - Executor Avançado de Testes

Script principal para execução completa de testes com várias opções.

**Uso:**

```cmd
scripts\run_tests.bat [tipo] [opções]
```

**Tipos de Teste:**

- `unit` - Executa apenas testes unitários
- `integration` - Executa apenas testes de integração
- `performance` - Executa apenas testes de performance
- (sem parâmetro) - Executa todos os testes

**Opções:**

- `verbose` ou `-v` - Saída detalhada
- `stop` ou `-s` - Para na primeira falha
- `debug` - Usa build Debug ao invés de Release

**Exemplos:**

```cmd
# Executar todos os testes
scripts\run_tests.bat

# Executar apenas testes unitários com saída detalhada
scripts\run_tests.bat unit verbose

# Executar testes de integração e parar na primeira falha
scripts\run_tests.bat integration stop

# Executar testes usando build Debug
scripts\run_tests.bat debug
```

### ⚡ `quick_test.bat` - Teste Rápido

Script simples para execução rápida dos testes mais importantes.

**Uso:**

```cmd
scripts\quick_test.bat
```

Executa:

- Teste unitário de matemática
- Teste de integração básico do Bullet Physics

### 🔧 `test_runner.bat` - Executor Básico

Script básico que executa uma lista predefinida de testes.

**Uso:**

```cmd
scripts\test_runner.bat [debug]
```

**Parâmetros:**

- `debug` - Usa configuração Debug (padrão: Release)

### 🐧 `test_runner.sh` - Executor para Linux/macOS

Versão do test_runner para sistemas Unix.

**Uso:**

```bash
./scripts/test_runner.sh [debug]
```

## Estrutura de Saída

Todos os scripts seguem o padrão de saída estabelecido:

```
========================================
 Game Engine Kiro - Test Runner
========================================

Running Unit Tests...
----------------------------------------
[PASS] MathTest
[FAILED] SomeTest (Exit code: 1)

========================================
 Test Execution Summary
========================================
Total Tests: 2
Passed: 1
Failed: 1

[FAILED] 1 TEST(S) FAILED!
========================================
```

## Códigos de Saída

- `0` - Todos os testes passaram
- `1` - Um ou mais testes falharam
- `1` - Erro de configuração (diretório build não encontrado, etc.)

## Integração com Build System

Os scripts automaticamente:

- Detectam executáveis de teste no diretório `build/Release` ou `build/Debug`
- Filtram executáveis que não são testes (GameExample, etc.)
- Fornecem relatórios detalhados de execução
- Retornam códigos de saída apropriados para integração CI/CD

## Desenvolvimento

### Adicionando Novos Testes

1. Crie o teste seguindo os padrões estabelecidos
2. Adicione ao CMakeLists.txt usando as funções helper
3. Compile com `build.bat`
4. Os scripts automaticamente descobrirão o novo teste

### Modificando Scripts

- **Windows**: Edite os arquivos `.bat`
- **Linux/macOS**: Edite os arquivos `.sh` e torne-os executáveis com `chmod +x`

## Troubleshooting

### "Build directory not found"

Execute `build.bat` primeiro para compilar os testes.

### "Test executable not found"

Verifique se o teste foi adicionado corretamente ao CMakeLists.txt e compilado.

### Testes falhando

Use a opção `verbose` para obter mais detalhes sobre as falhas:

```cmd
scripts\run_tests.bat verbose
```

## Exemplos de Uso Comum

```cmd
# Desenvolvimento diário - teste rápido
scripts\quick_test.bat

# Antes de commit - todos os testes
scripts\run_tests.bat

# Debug de problema específico - testes unitários detalhados
scripts\run_tests.bat unit verbose

# CI/CD - todos os testes com parada na primeira falha
scripts\run_tests.bat stop
```

### 🧪 `run_physics_tests.bat` - Testes de Física

Script especializado para executar todos os testes relacionados à física.

**Uso:**

```cmd
scripts\run_physics_tests.bat [build] [verbose]
```

**Opções:**

- `build` - Compila o projeto antes de executar os testes
- `verbose` ou `-v` - Saída detalhada com informações de falhas

### 📊 `run_coverage_analysis.bat` - Análise de Cobertura

Script para análise de cobertura de código usando OpenCppCoverage.

**Uso:**

```cmd
scripts\run_coverage_analysis.bat [build] [verbose] [open]
```

**Opções:**

- `build` - Compila o projeto com suporte a cobertura
- `verbose` - Saída detalhada
- `open` - Abre o relatório HTML automaticamente

### 🎮 Running Examples

To run the example projects after building:

**GameExample (Comprehensive):**

```cmd
build\projects\GameExample\Release\GameExample.exe
```

**BasicExample (Simple):**

```cmd
build\projects\BasicExample\Release\BasicExample.exe
```

### 📦 `setup_dependencies.bat` - Setup de Dependências

Script para configurar dependências do projeto (vcpkg).

**Uso:**

```cmd
scripts\setup_dependencies.bat
```

### 📋 `monitor.bat` - Monitor de Logs

Script para monitorar logs do sistema.

**Uso:**

```cmd
scripts\monitor.bat
```
