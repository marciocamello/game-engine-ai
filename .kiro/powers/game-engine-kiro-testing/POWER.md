---
name: "Game Engine Kiro - Testing Framework"
version: "1.0.0"
description: "Specialized knowledge for Game Engine Kiro's testing standards, including unit tests, integration tests, and property-based testing"
keywords:
  [
    "testing",
    "unit",
    "integration",
    "property",
    "test",
    "validation",
    "cmake",
    "build",
    "coverage",
  ]
author: "Game Engine Kiro Team"
category: "Testing"
---

# Game Engine Kiro - Testing Framework Power

Este Power fornece conhecimento especializado sobre os padrões de teste do Game Engine Kiro, incluindo estruturas obrigatórias, convenções e workflows de desenvolvimento.

## Quando Ativar Este Power

Este Power é ativado automaticamente quando você menciona:

- **Testes**: "test", "testing", "unit", "integration", "property"
- **Validação**: "validation", "verify", "check", "assert"
- **Build**: "build", "compile", "cmake", "scripts"
- **Coverage**: "coverage", "analysis", "report"

## Estrutura Obrigatória de Testes

### Template de Teste Unitário

```cpp
#include "TestUtils.h"
#include "Path/To/ComponentHeader.h"
// Add other necessary includes

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test description
 * Requirements: X.X, Y.Y (requirement description)
 */
bool TestFunctionName() {
    TestOutput::PrintTestStart("test description");

    // Test implementation here
    // Use EXPECT_* macros for assertions:
    // EXPECT_TRUE(condition)
    // EXPECT_FALSE(condition)
    // EXPECT_EQUAL(expected, actual)
    // EXPECT_NOT_EQUAL(expected, actual)
    // EXPECT_NEARLY_EQUAL(expected, actual)
    // EXPECT_VEC3_NEARLY_EQUAL(expected, actual)

    TestOutput::PrintTestPass("test description");
    return true;
}

int main() {
    TestOutput::PrintHeader("ComponentName");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ComponentName Tests");

        // Run all tests
        allPassed &= suite.RunTest("Test Description", TestFunctionName);
        // Add more tests here

        // Print detailed summary
        suite.PrintSummary();

        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    } catch (...) {
        TestOutput::PrintError("UNKNOWN TEST ERROR!");
        return 1;
    }
}
```

### Template de Teste de Integração

```cpp
#include <iostream>
#include "../TestUtils.h"
#include "Path/To/ComponentHeaders.h"
// Add other necessary includes

using namespace GameEngine::Testing;

/**
 * Test description
 * Requirements: X.X, Y.Y (requirement description)
 */
bool TestFunctionName() {
    TestOutput::PrintTestStart("test description");

    // Test implementation here
    // Use EXPECT_* macros for assertions
    // Include proper cleanup for resources

    TestOutput::PrintTestPass("test description");
    return true;
}

int main() {
    TestOutput::PrintHeader("Integration Test Name");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Integration Test Name Tests");

        // Run all tests
        allPassed &= suite.RunTest("Test Description", TestFunctionName);
        // Add more tests here

        // Print detailed summary
        suite.PrintSummary();

        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    } catch (...) {
        TestOutput::PrintError("UNKNOWN TEST ERROR!");
        return 1;
    }
}
```

## Convenções de Nomenclatura

### Arquivos de Teste

- **Testes unitários**: `tests/unit/test_[component_name].cpp`
- **Testes de integração**: `tests/integration/test_[feature_name].cpp`
- **Executáveis de teste**: `[ComponentName]Test.exe`

### Conversão de Nomes

Arquivo `test_animation_state_machine.cpp` → Executável `AnimationstatemachineTest`:

1. Remover prefixo `test_`
2. Remover extensão `.cpp`
3. Converter snake_case para PascalCase
4. Adicionar sufixo `Test`

**Exemplos**:

- `test_physics_engine.cpp` → `PhysicsengineTest`
- `test_resource_manager.cpp` → `ResourcemanagerTest`
- `test_shader_compiler.cpp` → `ShadercompilerTest`

## Macros de Asserção Disponíveis

```cpp
EXPECT_TRUE(condition)                    // Boolean true
EXPECT_FALSE(condition)                   // Boolean false
EXPECT_EQUAL(expected, actual)            // Exact equality
EXPECT_NOT_EQUAL(expected, actual)        // Inequality
EXPECT_NEARLY_EQUAL(expected, actual)     // Float comparison with epsilon
EXPECT_VEC3_NEARLY_EQUAL(expected, actual) // Vector3 comparison
EXPECT_VEC4_NEARLY_EQUAL(expected, actual) // Vector4 comparison
EXPECT_QUAT_NEARLY_EQUAL(expected, actual) // Quaternion comparison
```

## Workflow de Desenvolvimento Individual

### Para Desenvolvimento Rápido de Specs

```powershell
# 1. Compilar apenas o teste específico (30 segundos vs 3 minutos)
.\scripts\build_unified.bat --tests [TestName]

# Exemplo: Desenvolvimento de AnimationStateMachine
.\scripts\build_unified.bat --tests AnimationstatemachineTest

# 2. Executar teste individual
.\build\Release\AnimationstatemachineTest.exe

# 3. Depurar e corrigir até passar
# 4. Mover para próximo teste
# 5. Quando todos os testes individuais passarem, executar suite completa
.\scripts\run_tests.bat
```

### Benefícios do Desenvolvimento Individual

- **Compilação 10x mais rápida**: 30 segundos vs 5 minutos
- **Depuração focada**: Problemas isolados em componentes específicos
- **Melhor workflow**: Completar uma funcionalidade por vez
- **Feedback imediato**: Saber instantaneamente se mudanças funcionam

## Padrões de Teste para Specs

### Testes de Propriedade (Property-Based Testing)

Para specs com testes de propriedade, use este formato:

```cpp
/**
 * Property Test: Vertex Skinning Transformation
 * Validates: Requirements 2.1, 2.2, 2.3
 */
bool TestVertexSkinningProperty() {
    TestOutput::PrintTestStart("vertex skinning transformation property");

    // Property-based test implementation
    // Test universal properties across many inputs

    TestOutput::PrintTestPass("vertex skinning transformation property");
    return true;
}
```

### Anotação Obrigatória

- **DEVE usar**: `**Validates: Requirements X.X**`
- **DEVE implementar**: APENAS as propriedades especificadas pela tarefa
- **DEVE tentar**: Escrever testes sem mocking quando possível
- **DEVE usar**: Framework de teste especificado no documento de design

## Estratégias de Teste OpenGL

### Abordagem Recomendada

1. **SEMPRE tentar primeiro**: Testes matemáticos e baseados em CPU
2. **SE contexto OpenGL necessário**: Usar alternativas CPU/matemáticas quando possível
3. **SE erros de contexto OpenGL**: Deixar para estrutura futura de renderização offscreen
4. **FOCAR em**: Testar lógica, algoritmos e estruturas de dados ao invés de chamadas OpenGL
5. **NUNCA criar**: Testes fictícios apenas para passar

## Comandos de Build e Teste

### Comandos Obrigatórios

```powershell
# Build completo - ÚNICO comando permitido para build
.\scripts\build_unified.bat --tests

# Limpeza - ÚNICO comando permitido para limpeza
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
.\scripts\build_unified.bat --tests

# Executar todos os testes - OBRIGATÓRIO antes de completar tarefa
.\scripts\run_tests.bat

# Desenvolvimento individual
.\scripts\build_unified.bat --tests [TestName]
.\build\Release\[TestName].exe
```

## Regras Críticas

### Antes de Completar Qualquer Tarefa

1. **Testes individuais passam**: Cada teste compilado e executado individualmente
2. **Build bem-sucedido**: `.\scripts\build_unified.bat --tests`
3. **Todos os testes passam**: `.\scripts\run_tests.bat`
4. **Novos testes seguem template exatamente**
5. **Formato de saída corresponde aos testes existentes**
6. **Sem avisos de compilação para arquivos de teste**

### Violações = Build Quebrado

- **Caminhos de include errados**: Build falhará
- **Namespaces ausentes**: Erros de compilação
- **Estrutura main errada**: Test runner não funcionará
- **Chamadas TestOutput ausentes**: Formato de saída inconsistente
- **Tipos de retorno errados**: Erros do framework de teste
- **Tratamento de exceção ausente**: Crashes em erros

## Status Atual da Suite de Testes

### Testes Unitários (13 total - 100% passando)

- MathTest, MatrixTest, QuaternionTest
- AssertionmacrosTest, Audio3dpositioningTest, AudioengineTest
- AudioloaderTest, MeshloaderTest, MeshoptimizerTest
- ModelnodeTest, ResourcefallbacksTest, ResourcemanagerTest
- TextureloaderTest

### Testes de Integração (18 total - 100% passando)

- Physics: BulletUtilsSimpleTest, BulletIntegrationTest, BulletConversionTest
- Physics: CollisionShapeFactorySimpleTest, PhysicsQueriesTest, PhysicsConfigurationTest
- Movement: MovementComponentComparisonTest, PhysicsPerformanceSimpleTest
- Memory: MemoryUsageSimpleTest, CharacterBehaviorSimpleTest
- Audio: OpenALIntegrationTest, AudioCameraIntegrationTest
- Resources: ResourceStatisticsTest, ErrorHandlingTest, FinalV1ValidationTest
- 3D Models: ModelLoaderAssimpTest, MaterialImporterTest, GLTFLoaderTest, FBXLoaderTest

---

**Lembre-se**: Testes são a base da qualidade do código. Siga os templates exatamente e use desenvolvimento individual para máxima eficiência.
