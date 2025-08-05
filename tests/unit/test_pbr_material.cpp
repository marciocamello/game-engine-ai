#include "TestUtils.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test PBR material creation and default properties
 * Requirements: 2.1, 2.2 (PBR material creation with default values)
 */
bool TestPBRMaterialCreation() {
    TestOutput::PrintTestStart("pbr material creation");

    PBRMaterial material("TestPBRMaterial");
    
    EXPECT_EQUAL(material.GetName(), "TestPBRMaterial");
    EXPECT_EQUAL(static_cast<int>(material.GetType()), static_cast<int>(Material::Type::PBR));
    
    // Test default PBR properties
    auto properties = material.GetProperties();
    EXPECT_VEC3_NEARLY_EQUAL(properties.albedo, Math::Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_NEARLY_EQUAL(properties.metallic, 0.0f);
    EXPECT_NEARLY_EQUAL(properties.roughness, 0.5f);
    EXPECT_NEARLY_EQUAL(properties.ao, 1.0f);
    EXPECT_VEC3_NEARLY_EQUAL(properties.emission, Math::Vec3(0.0f, 0.0f, 0.0f));
    EXPECT_NEARLY_EQUAL(properties.emissionStrength, 1.0f);
    EXPECT_NEARLY_EQUAL(properties.normalStrength, 1.0f);
    EXPECT_NEARLY_EQUAL(properties.alphaCutoff, 0.5f);
    EXPECT_FALSE(properties.useAlphaCutoff);

    TestOutput::PrintTestPass("pbr material creation");
    return true;
}

/**
 * Test PBR material property setters and getters
 * Requirements: 2.1, 2.2 (PBR property management)
 */
bool TestPBRMaterialProperties() {
    TestOutput::PrintTestStart("pbr material properties");

    PBRMaterial material("TestMaterial");
    
    // Test individual property setters
    material.SetAlbedo(Math::Vec3(0.8f, 0.2f, 0.2f));
    material.SetMetallic(0.1f);
    material.SetRoughness(0.3f);
    material.SetAO(0.9f);
    material.SetEmission(Math::Vec3(0.1f, 0.1f, 0.0f));
    material.SetEmissionStrength(2.0f);
    material.SetNormalStrength(1.5f);
    material.SetAlphaCutoff(0.7f);
    material.SetUseAlphaCutoff(true);
    
    // Test individual property getters
    EXPECT_VEC3_NEARLY_EQUAL(material.GetAlbedo(), Math::Vec3(0.8f, 0.2f, 0.2f));
    EXPECT_NEARLY_EQUAL(material.GetMetallic(), 0.1f);
    EXPECT_NEARLY_EQUAL(material.GetRoughness(), 0.3f);
    EXPECT_NEARLY_EQUAL(material.GetAO(), 0.9f);
    EXPECT_VEC3_NEARLY_EQUAL(material.GetEmission(), Math::Vec3(0.1f, 0.1f, 0.0f));
    EXPECT_NEARLY_EQUAL(material.GetEmissionStrength(), 2.0f);
    EXPECT_NEARLY_EQUAL(material.GetNormalStrength(), 1.5f);
    EXPECT_NEARLY_EQUAL(material.GetAlphaCutoff(), 0.7f);
    EXPECT_TRUE(material.GetUseAlphaCutoff());

    TestOutput::PrintTestPass("pbr material properties");
    return true;
}

/**
 * Test PBR material property validation and clamping
 * Requirements: 2.2, 2.3 (PBR property validation)
 */
bool TestPBRMaterialValidation() {
    TestOutput::PrintTestStart("pbr material validation");

    PBRMaterial material("ValidationTest");
    
    // Test property clamping for out-of-range values
    material.SetMetallic(-0.5f);  // Should clamp to 0.0f
    EXPECT_NEARLY_EQUAL(material.GetMetallic(), 0.0f);
    
    material.SetMetallic(1.5f);   // Should clamp to 1.0f
    EXPECT_NEARLY_EQUAL(material.GetMetallic(), 1.0f);
    
    material.SetRoughness(-0.1f); // Should clamp to 0.0f
    EXPECT_NEARLY_EQUAL(material.GetRoughness(), 0.0f);
    
    material.SetRoughness(2.0f);  // Should clamp to 1.0f
    EXPECT_NEARLY_EQUAL(material.GetRoughness(), 1.0f);
    
    material.SetAO(-0.2f);        // Should clamp to 0.0f
    EXPECT_NEARLY_EQUAL(material.GetAO(), 0.0f);
    
    material.SetAO(1.5f);         // Should clamp to 1.0f
    EXPECT_NEARLY_EQUAL(material.GetAO(), 1.0f);
    
    // Test validation method
    EXPECT_TRUE(material.ValidateProperties());

    TestOutput::PrintTestPass("pbr material validation");
    return true;
}

/**
 * Test PBR material properties structure
 * Requirements: 2.1, 2.2 (PBR properties structure)
 */
bool TestPBRMaterialPropertiesStruct() {
    TestOutput::PrintTestStart("pbr material properties struct");

    PBRMaterial material("StructTest");
    
    // Create custom properties
    PBRMaterial::Properties props;
    props.albedo = Math::Vec3(0.5f, 0.7f, 0.9f);
    props.metallic = 0.8f;
    props.roughness = 0.2f;
    props.ao = 0.85f;
    props.emission = Math::Vec3(0.2f, 0.0f, 0.1f);
    props.emissionStrength = 1.5f;
    props.normalStrength = 0.8f;
    props.alphaCutoff = 0.3f;
    props.useAlphaCutoff = true;
    
    // Set properties using struct
    material.SetProperties(props);
    
    // Verify properties were set correctly
    auto retrievedProps = material.GetProperties();
    EXPECT_VEC3_NEARLY_EQUAL(retrievedProps.albedo, props.albedo);
    EXPECT_NEARLY_EQUAL(retrievedProps.metallic, props.metallic);
    EXPECT_NEARLY_EQUAL(retrievedProps.roughness, props.roughness);
    EXPECT_NEARLY_EQUAL(retrievedProps.ao, props.ao);
    EXPECT_VEC3_NEARLY_EQUAL(retrievedProps.emission, props.emission);
    EXPECT_NEARLY_EQUAL(retrievedProps.emissionStrength, props.emissionStrength);
    EXPECT_NEARLY_EQUAL(retrievedProps.normalStrength, props.normalStrength);
    EXPECT_NEARLY_EQUAL(retrievedProps.alphaCutoff, props.alphaCutoff);
    EXPECT_EQUAL(retrievedProps.useAlphaCutoff, props.useAlphaCutoff);

    TestOutput::PrintTestPass("pbr material properties struct");
    return true;
}

/**
 * Test PBR material texture management
 * Requirements: 2.1, 2.3 (PBR texture support)
 */
bool TestPBRMaterialTextures() {
    TestOutput::PrintTestStart("pbr material textures");

    PBRMaterial material("TextureTest");
    
    // Test texture setters and getters (without actual texture objects)
    // Since we can't create actual textures without OpenGL context,
    // we test the interface and null handling
    
    // Initially, all texture maps should be null
    EXPECT_EQUAL(material.GetAlbedoMap(), nullptr);
    EXPECT_EQUAL(material.GetNormalMap(), nullptr);
    EXPECT_EQUAL(material.GetMetallicRoughnessMap(), nullptr);
    EXPECT_EQUAL(material.GetAOMap(), nullptr);
    EXPECT_EQUAL(material.GetEmissionMap(), nullptr);
    
    // Test setting null textures (should not crash)
    material.SetAlbedoMap(nullptr);
    material.SetNormalMap(nullptr);
    material.SetMetallicRoughnessMap(nullptr);
    material.SetAOMap(nullptr);
    material.SetEmissionMap(nullptr);
    
    // Verify they remain null
    EXPECT_EQUAL(material.GetAlbedoMap(), nullptr);
    EXPECT_EQUAL(material.GetNormalMap(), nullptr);
    EXPECT_EQUAL(material.GetMetallicRoughnessMap(), nullptr);
    EXPECT_EQUAL(material.GetAOMap(), nullptr);
    EXPECT_EQUAL(material.GetEmissionMap(), nullptr);

    TestOutput::PrintTestPass("pbr material textures");
    return true;
}

/**
 * Test PBR material advanced property system integration
 * Requirements: 2.5, 2.6 (Advanced property system integration)
 */
bool TestPBRMaterialPropertySystem() {
    TestOutput::PrintTestStart("pbr material property system");

    PBRMaterial material("PropertySystemTest");
    
    // Test that PBR properties are accessible through the advanced property system
    material.SetAlbedo(Math::Vec3(0.6f, 0.4f, 0.2f));
    material.SetMetallic(0.7f);
    material.SetRoughness(0.4f);
    
    // Verify properties are accessible through base Material interface
    EXPECT_TRUE(material.HasProperty("u_albedo"));
    EXPECT_TRUE(material.HasProperty("u_metallic"));
    EXPECT_TRUE(material.HasProperty("u_roughness"));
    EXPECT_TRUE(material.HasProperty("u_ao"));
    
    // Test property retrieval through advanced system
    auto albedoProperty = material.GetProperty("u_albedo");
    EXPECT_EQUAL(static_cast<int>(albedoProperty.GetType()), static_cast<int>(MaterialProperty::Type::Vec3));
    EXPECT_VEC3_NEARLY_EQUAL(albedoProperty.AsVec3(), Math::Vec3(0.6f, 0.4f, 0.2f));
    
    auto metallicProperty = material.GetProperty("u_metallic");
    EXPECT_EQUAL(static_cast<int>(metallicProperty.GetType()), static_cast<int>(MaterialProperty::Type::Float));
    EXPECT_NEARLY_EQUAL(metallicProperty.AsFloat(), 0.7f);

    TestOutput::PrintTestPass("pbr material property system");
    return true;
}

/**
 * Test PBR material defaults restoration
 * Requirements: 2.2, 2.3 (PBR property defaults and validation)
 */
bool TestPBRMaterialDefaults() {
    TestOutput::PrintTestStart("pbr material defaults");

    PBRMaterial material("DefaultsTest");
    
    // Create invalid properties
    PBRMaterial::Properties invalidProps;
    invalidProps.metallic = -1.0f;      // Invalid
    invalidProps.roughness = 2.0f;     // Invalid
    invalidProps.ao = -0.5f;           // Invalid
    invalidProps.emissionStrength = -1.0f; // Invalid
    invalidProps.normalStrength = -0.5f;   // Invalid
    invalidProps.alphaCutoff = 1.5f;       // Invalid
    
    // Set invalid properties
    material.SetProperties(invalidProps);
    
    // Properties should be clamped to valid ranges
    auto props = material.GetProperties();
    EXPECT_NEARLY_EQUAL(props.metallic, 0.0f);      // Clamped from -1.0f
    EXPECT_NEARLY_EQUAL(props.roughness, 1.0f);     // Clamped from 2.0f
    EXPECT_NEARLY_EQUAL(props.ao, 0.0f);            // Clamped from -0.5f
    EXPECT_NEARLY_EQUAL(props.emissionStrength, 0.0f); // Clamped from -1.0f
    EXPECT_NEARLY_EQUAL(props.normalStrength, 0.0f);   // Clamped from -0.5f
    EXPECT_NEARLY_EQUAL(props.alphaCutoff, 1.0f);      // Clamped from 1.5f
    
    // Validation should now pass
    EXPECT_TRUE(material.ValidateProperties());

    TestOutput::PrintTestPass("pbr material defaults");
    return true;
}

int main() {
    TestOutput::PrintHeader("PBRMaterial");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("PBRMaterial Tests");

        // Run all tests
        allPassed &= suite.RunTest("PBR Material Creation", TestPBRMaterialCreation);
        allPassed &= suite.RunTest("PBR Material Properties", TestPBRMaterialProperties);
        allPassed &= suite.RunTest("PBR Material Validation", TestPBRMaterialValidation);
        allPassed &= suite.RunTest("PBR Material Properties Struct", TestPBRMaterialPropertiesStruct);
        allPassed &= suite.RunTest("PBR Material Textures", TestPBRMaterialTextures);
        allPassed &= suite.RunTest("PBR Material Property System", TestPBRMaterialPropertySystem);
        allPassed &= suite.RunTest("PBR Material Defaults", TestPBRMaterialDefaults);

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