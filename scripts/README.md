# Game Engine Kiro - Scripts de Desenvolvimento

Este diretório contém scripts essenciais para desenvolvimento, build e testes do Game Engine Kiro.

## Scripts Principais

### 🏗️ `build_unified.bat` - Build Principal

**ÚNICO script autorizado para build do projeto.** Suporta todas as combinações de build necessárias.

**Uso:**

```cmd
# Build completo (padrão)
scripts\build_unified.bat

# Build com testes
scripts\build_unified.bat --tests

# Build de teste específico (recomendado para desenvolvimento)
scripts\build_unified.bat --tests MathTest

# Build apenas do engine
scripts\build_unified.bat --engine

# Build apenas dos projetos
scripts\build_unified.bat --projects

# Build debug
scripts\build_unified.bat --debug --tests

# Build com cobertura de código
scripts\build_unified.bat --coverage
```

### 🚀 `run_tests.bat` - Executor de Testes

Script principal para execução de todos os testes com descoberta automática.

**Uso:**

```cmd
# Executar todos os testes
scripts\run_tests.bat

# Executar apenas testes unitários
scripts\run_tests.bat --unit

# Executar apenas testes de integração
scripts\run_tests.bat --integration

# Executar testes do engine
scripts\run_tests.bat --engine

# Executar testes de projetos
scripts\run_tests.bat --projects
```

## Scripts de Desenvolvimento

### 🖥️ `dev.bat` - Console de Desenvolvimento

Console interativo com todas as opções de desenvolvimento em um só lugar.

**Uso:**

```cmd
scripts\dev.bat
```

**Funcionalidades:**

- Build Release/Debug
- Executar projetos
- Monitorar logs
- Análise de código
- Verificação de memória

### 🐛 `debug.bat` - Sessão de Debug

Launcher para sessões de debug com várias opções.

**Uso:**

```cmd
scripts\debug.bat
```

**Opções de Debug:**

- Visual Studio Debugger
- Console Output
- File Logging
- Memory Leak Detection
- Performance Profiling

### 📋 `monitor.bat` - Monitor de Logs

Monitor interativo para logs do sistema.

**Uso:**

```cmd
scripts\monitor.bat
```

**Funcionalidades:**

- Monitor de logs em tempo real
- Busca por erros e warnings
- Exportação de relatórios
- Análise de logs históricos

## Scripts Especializados

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

### 🎯 `run_final_validation.bat` - Validação Final

Script para validação completa do sistema antes de releases.

**Uso:**

```cmd
scripts\run_final_validation.bat
```

### 📦 `setup_dependencies.bat` - Setup de Dependências

Script para configurar dependências do projeto (vcpkg).

**Uso:**

```cmd
scripts\setup_dependencies.bat
```

## Executando Projetos

Após o build, execute os projetos diretamente:

**GameExample (Completo):**

```cmd
build\projects\GameExample\Release\GameExample.exe
```

**BasicExample (Simples):**

```cmd
build\projects\BasicExample\Release\BasicExample.exe
```

## Workflow de Desenvolvimento

### Desenvolvimento Diário

```cmd
# 1. Build e teste
scripts\build_unified.bat --tests

# 2. Executar todos os testes
scripts\run_tests.bat

# 3. Monitorar logs (se necessário)
scripts\monitor.bat
```

### Desenvolvimento de Specs (Recomendado)

```cmd
# 1. Build teste específico (muito mais rápido)
scripts\build_unified.bat --tests MathTest

# 2. Executar teste específico
build\Release\MathTest.exe

# 3. Quando todos os testes individuais passarem
scripts\run_tests.bat
```

### Antes de Commit

```cmd
# 1. Build completo
scripts\build_unified.bat --tests

# 2. Todos os testes
scripts\run_tests.bat

# 3. Validação final (opcional)
scripts\run_final_validation.bat
```

## Códigos de Saída

- `0` - Sucesso
- `1` - Falha (build, testes, etc.)
- `2` - Erro de configuração

## Troubleshooting

### "Build directory not found"

Execute `scripts\build_unified.bat --tests` primeiro.

### "Test executable not found"

Verifique se o teste foi compilado corretamente.

### Build falha

1. Limpe o build: `Remove-Item -Recurse -Force build`
2. Rebuild: `scripts\build_unified.bat --tests`

### Testes falhando

Use `scripts\run_tests.bat --unit` ou `--integration` para isolar problemas.

## Regras Importantes

- **NUNCA use cmake diretamente** - sempre use `build_unified.bat`
- **NUNCA use comandos de deleção** além do cleanup de build especificado
- **SEMPRE execute todos os testes** antes de completar tarefas
- **Use testes individuais** durante desenvolvimento para velocidade
