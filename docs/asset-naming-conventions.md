# Game Engine Kiro - Asset Naming Conventions

## Overview

This document establishes comprehensive asset naming and organization conventions for Game Engine Kiro. These conventions ensure consistency, discoverability, and maintainability across all game projects, following industry standards used by professional game engines.

## Core Principles

### 1. Hierarchical Organization

- Assets are organized by **type** first, then by **category**, then by **specific use**
- Clear separation between **shared engine assets** and **project-specific assets**
- Consistent folder structure across all projects

### 2. Descriptive Naming

- Asset names clearly indicate their purpose and content
- Use PascalCase for asset files
- Include relevant descriptors (character name, animation type, material properties)

### 3. Scalable Structure

- Structure supports small indie projects and large AAA productions
- Easy to add new asset types and categories
- Clear ownership and responsibility for each asset category

## Engine Asset Organization

### Shared Engine Assets (assets/)

```
assets/
├── meshes/                     # Static 3D meshes (primitives, tools)
│   ├── primitives/            # Basic geometric shapes
│   │   ├── Cube.obj
│   │   ├── Sphere.obj
│   │   ├── Plane.obj
│   │   └── Cylinder.obj
│   ├── tools/                 # Development and debugging meshes
│   │   ├── DebugArrow.obj
│   │   ├── DebugSphere.obj
│   │   └── GridPlane.obj
│   └── fallbacks/             # Fallback meshes for missing assets
│       ├── MissingMesh.obj
│       └── ErrorCube.obj
├── materials/                  # Shared material definitions
│   ├── defaults/              # Default engine materials
│   │   ├── DefaultLit.mat
│   │   ├── DefaultUnlit.mat
│   │   └── DefaultPBR.mat
│   └── debug/                 # Debug and development materials
│       ├── DebugRed.mat
│       ├── DebugGreen.mat
│       └── Wireframe.mat
├── textures/                   # Shared engine textures
│   ├── defaults/              # Default textures
│   │   ├── White.png
│   │   ├── Black.png
│   │   ├── Normal.png
│   │   └── Checker.png
│   ├── fallbacks/             # Fallback textures
│   │   ├── MissingTexture.png
│   │   └── ErrorTexture.png
│   └── debug/                 # Debug textures
│       ├── UVChecker.png
│       └── DebugGrid.png
├── shaders/                    # Engine shaders
│   ├── vertex/
│   ├── fragment/
│   └── compute/
└── audio/                      # Shared engine audio
    ├── system/                # System sounds
    └── debug/                 # Debug audio files
```

## Project Asset Organization

### Project-Specific Assets (projects/[ProjectName]/assets/)

```
projects/GameExample/assets/
├── characters/                 # Character-specific assets
│   ├── XBotCharacter/         # Specific character folder
│   │   ├── XBotCharacter.fbx  # Main character model
│   │   ├── animations/        # Character animations
│   │   │   ├── Idle.fbx
│   │   │   ├── Walk.fbx
│   │   │   ├── Run.fbx
│   │   │   ├── Jump.fbx
│   │   │   ├── Attack_Sword.fbx
│   │   │   ├── Death_Forward.fbx
│   │   │   └── Celebrate.fbx
│   │   ├── textures/          # Character textures
│   │   │   ├── XBotCharacter_Diffuse.png
│   │   │   ├── XBotCharacter_Normal.png
│   │   │   ├── XBotCharacter_Roughness.png
│   │   │   └── XBotCharacter_Metallic.png
│   │   └── materials/         # Character materials
│   │       ├── XBotCharacter_Body.mat
│   │       ├── XBotCharacter_Clothing.mat
│   │       └── XBotCharacter_Accessories.mat
│   └── PlayerCharacter/       # Another character example
│       ├── PlayerCharacter.fbx
│       ├── animations/
│       ├── textures/
│       └── materials/
├── animations/                 # Shared animations (not character-specific)
│   ├── generic/               # Generic animations usable by multiple characters
│   │   ├── GenericWalk.fbx
│   │   ├── GenericRun.fbx
│   │   └── GenericIdle.fbx
│   └── environmental/         # Environmental animations
│       ├── DoorOpen.fbx
│       ├── FlagWave.fbx
│       └── WaterFlow.fbx
├── meshes/                     # Static meshes (non-character)
│   ├── environment/           # Environmental meshes
│   │   ├── buildings/
│   │   │   ├── House_Medieval.fbx
│   │   │   └── Castle_Tower.fbx
│   │   ├── nature/
│   │   │   ├── Tree_Oak_Large.fbx
│   │   │   ├── Rock_Granite_Boulder.fbx
│   │   │   └── Grass_Patch.fbx
│   │   └── terrain/
│   │       ├── Terrain_Hills.fbx
│   │       └── Terrain_Valley.fbx
│   ├── props/                 # Interactive objects and props
│   │   ├── weapons/
│   │   │   ├── Sword_Iron.fbx
│   │   │   └── Bow_Wooden.fbx
│   │   ├── containers/
│   │   │   ├── Chest_Wooden.fbx
│   │   │   └── Barrel_Oak.fbx
│   │   └── tools/
│   │       ├── Hammer_Blacksmith.fbx
│   │       └── Pickaxe_Mining.fbx
│   └── vehicles/              # Vehicles and mounts
│       ├── Horse_Brown.fbx
│       └── Cart_Wooden.fbx
├── materials/                  # Project materials
│   ├── environment/           # Environmental materials
│   │   ├── Stone_Granite.mat
│   │   ├── Wood_Oak.mat
│   │   └── Metal_Iron.mat
│   ├── props/                 # Prop materials
│   └── effects/               # Effect materials
├── textures/                   # Project textures
│   ├── environment/           # Environmental textures
│   │   ├── terrain/
│   │   │   ├── Grass_Diffuse_1024.png
│   │   │   ├── Dirt_Normal_1024.png
│   │   │   └── Stone_Roughness_512.png
│   │   ├── buildings/
│   │   └── nature/
│   ├── props/                 # Prop textures
│   ├── vfx/                   # Visual effects textures
│   │   ├── particles/
│   │   │   ├── Smoke_01.png
│   │   │   ├── Fire_01.png
│   │   │   └── Magic_Sparkle.png
│   │   ├── decals/
│   │   └── overlays/
│   └── ui/                    # User interface textures
│       ├── buttons/
│       ├── icons/
│       └── backgrounds/
├── sounds/                     # Audio assets
│   ├── characters/            # Character-specific sounds
│   │   └── XBotCharacter/
│   │       ├── Footstep_Grass.wav
│   │       ├── Footstep_Stone.wav
│   │       ├── Attack_Grunt.wav
│   │       └── Death_Scream.wav
│   ├── sfx/                   # Sound effects
│   │   ├── combat/
│   │   │   ├── Sword_Hit_Metal.wav
│   │   │   └── Arrow_Release.wav
│   │   ├── environment/
│   │   │   ├── Wind_Forest.wav
│   │   │   └── Water_Stream.wav
│   │   └── ui/
│   │       ├── Button_Click.wav
│   │       └── Menu_Navigate.wav
│   ├── music/                 # Background music
│   │   ├── BGM_Menu_Peaceful.ogg
│   │   ├── BGM_Combat_Intense.ogg
│   │   └── BGM_Victory_Triumphant.ogg
│   └── voice/                 # Voice acting
│       ├── narrator/
│       └── characters/
├── vfx/                        # Visual effects assets
│   ├── particles/             # Particle system definitions
│   ├── shaders/               # Project-specific shaders
│   └── textures/              # VFX-specific textures (linked to textures/vfx/)
├── ui/                         # User interface assets
│   ├── fonts/                 # Font files
│   ├── layouts/               # UI layout definitions
│   ├── textures/              # UI textures (linked to textures/ui/)
│   └── animations/            # UI animations
└── input/                      # Input-related assets
    ├── bindings/              # Input binding configurations
    └── icons/                 # Input prompt icons (keyboard, gamepad)
```

## Asset Naming Conventions

### Character Assets

#### Character Models

```
# Pattern: [CharacterName].fbx
XBotCharacter.fbx              # ✅ Main character model
PlayerCharacter.fbx            # ✅ Player character
EnemyOrc.fbx                   # ✅ Enemy character
NPCMerchant.fbx                # ✅ Non-player character
```

#### Character Animations

```
# Pattern: [ActionName].fbx or [ActionName]_[Variant].fbx
Idle.fbx                       # ✅ Basic idle animation
Walk.fbx                       # ✅ Basic walk animation
Run.fbx                        # ✅ Basic run animation
Jump.fbx                       # ✅ Jump animation
Attack_Sword.fbx               # ✅ Attack with sword
Attack_Bow.fbx                 # ✅ Attack with bow
Death_Forward.fbx              # ✅ Death falling forward
Death_Backward.fbx             # ✅ Death falling backward
Celebrate_Victory.fbx          # ✅ Victory celebration
Taunt_Aggressive.fbx           # ✅ Aggressive taunt
```

#### Character Textures

```
# Pattern: [CharacterName]_[MapType].png
XBotCharacter_Diffuse.png      # ✅ Diffuse/Albedo map
XBotCharacter_Normal.png       # ✅ Normal map
XBotCharacter_Roughness.png    # ✅ Roughness map
XBotCharacter_Metallic.png     # ✅ Metallic map
XBotCharacter_AO.png           # ✅ Ambient occlusion map
XBotCharacter_Emission.png     # ✅ Emission map
```

### Environmental Assets

#### Static Meshes

```
# Pattern: [Category]_[Type]_[Variant].fbx
Building_House_Medieval.fbx    # ✅ Medieval house
Building_Castle_Tower.fbx     # ✅ Castle tower
Tree_Oak_Large.fbx            # ✅ Large oak tree
Tree_Pine_Small.fbx           # ✅ Small pine tree
Rock_Granite_Boulder.fbx      # ✅ Granite boulder
Rock_Limestone_Cliff.fbx      # ✅ Limestone cliff
```

#### Environmental Textures

```
# Pattern: [Material]_[MapType]_[Resolution].png
Grass_Diffuse_1024.png        # ✅ Grass diffuse, 1024x1024
Stone_Normal_2048.png         # ✅ Stone normal map, 2048x2048
Wood_Roughness_512.png        # ✅ Wood roughness, 512x512
Metal_Metallic_1024.png       # ✅ Metal metallic map
```

### Audio Assets

#### Sound Effects

```
# Pattern: [Source]_[Action]_[Surface/Material].wav
Footstep_Grass_01.wav         # ✅ Footstep on grass, variation 1
Footstep_Stone_02.wav         # ✅ Footstep on stone, variation 2
Sword_Hit_Metal.wav           # ✅ Sword hitting metal
Arrow_Release_Bow.wav         # ✅ Arrow released from bow
Door_Open_Wood.wav            # ✅ Wooden door opening
```

#### Background Music

```
# Pattern: BGM_[Location/Situation]_[Mood].ogg
BGM_Menu_Peaceful.ogg         # ✅ Peaceful menu music
BGM_Combat_Intense.ogg        # ✅ Intense combat music
BGM_Forest_Ambient.ogg        # ✅ Ambient forest music
BGM_Victory_Triumphant.ogg    # ✅ Triumphant victory music
```

### VFX and UI Assets

#### Particle Textures

```
# Pattern: [Effect]_[Number].png
Smoke_01.png                  # ✅ Smoke texture variation 1
Fire_02.png                   # ✅ Fire texture variation 2
Magic_Sparkle.png             # ✅ Magic sparkle effect
Explosion_Debris.png          # ✅ Explosion debris
```

#### UI Elements

```
# Pattern: [Element]_[State].png
Button_Default.png            # ✅ Default button state
Button_Hover.png              # ✅ Button hover state
Button_Pressed.png            # ✅ Button pressed state
Icon_Health.png               # ✅ Health icon
Icon_Mana.png                 # ✅ Mana icon
```

## Asset Path Resolution

### Engine Asset Loading Priority

1. **Project-specific assets**: `projects/[ProjectName]/assets/`
2. **Shared engine assets**: `assets/`
3. **Fallback assets**: `assets/*/fallbacks/`

### Example Asset Loading

```cpp
// Engine automatically resolves in this order:
// 1. projects/GameExample/assets/characters/XBotCharacter/XBotCharacter.fbx
// 2. assets/meshes/fallbacks/MissingMesh.obj (if not found)

auto character = ResourceManager::LoadModel("characters/XBotCharacter/XBotCharacter.fbx");
auto animation = ResourceManager::LoadAnimation("characters/XBotCharacter/animations/Idle.fbx");
auto texture = ResourceManager::LoadTexture("characters/XBotCharacter/textures/XBotCharacter_Diffuse.png");
```

## Asset Validation Rules

### File Size Guidelines

```
Textures:
- UI Icons: 64x64 to 256x256
- Character Textures: 1024x1024 to 2048x2048
- Environment Textures: 512x512 to 4096x4096
- VFX Textures: 256x256 to 1024x1024

Models:
- Character Models: < 50,000 triangles
- Environment Props: < 10,000 triangles
- Background Objects: < 5,000 triangles

Audio:
- Sound Effects: < 5MB, 44.1kHz, 16-bit
- Music: < 50MB, 44.1kHz, compressed
- Voice: < 10MB per file
```

### Naming Validation

```cpp
// Valid asset names (examples)
✅ XBotCharacter.fbx
✅ Idle.fbx
✅ XBotCharacter_Diffuse.png
✅ Footstep_Grass_01.wav
✅ BGM_Combat_Intense.ogg

// Invalid asset names
❌ xbot.fbx                    // Not descriptive
❌ anim1.fbx                   // Not descriptive
❌ texture.png                 // Too generic
❌ sound.wav                   // Too generic
❌ música_combate.ogg          // Non-English
```

## Project Setup Workflow

### 1. Create Asset Directory Structure

```powershell
# Create complete asset structure for new project
.\scripts\create_project_assets.bat YourProject

# This creates:
# projects/YourProject/assets/characters/
# projects/YourProject/assets/animations/
# projects/YourProject/assets/meshes/
# projects/YourProject/assets/materials/
# projects/YourProject/assets/textures/
# projects/YourProject/assets/sounds/
# projects/YourProject/assets/vfx/
# projects/YourProject/assets/ui/
# projects/YourProject/assets/input/
```

### 2. Asset Import Guidelines

```cpp
// When importing assets, follow naming conventions:
1. Rename files to match conventions
2. Place in appropriate category folders
3. Create character-specific folders for character assets
4. Use descriptive names with proper variants
5. Maintain consistent resolution and quality standards
```

### 3. Asset Validation

```powershell
# Validate asset organization and naming
.\scripts\validate_assets.bat YourProject

# Check for:
# - Proper naming conventions
# - Correct folder structure
# - Missing fallback assets
# - File size compliance
# - Texture resolution standards
```

## Integration with Code

### Asset Loading in Character Classes

```cpp
// projects/GameExample/src/XBotCharacter.cpp
void XBotCharacter::LoadCharacterAnimations() {
    auto& resourceManager = GameEngine::ResourceManager::GetInstance();

    // Load character model
    m_characterModel = resourceManager.LoadModel(
        "characters/XBotCharacter/XBotCharacter.fbx"
    );

    // Load character animations
    resourceManager.LoadAnimation(
        "xbot_idle",
        "characters/XBotCharacter/animations/Idle.fbx"
    );
    resourceManager.LoadAnimation(
        "xbot_walk",
        "characters/XBotCharacter/animations/Walk.fbx"
    );
    resourceManager.LoadAnimation(
        "xbot_run",
        "characters/XBotCharacter/animations/Run.fbx"
    );

    // Load character textures
    resourceManager.LoadTexture(
        "xbot_diffuse",
        "characters/XBotCharacter/textures/XBotCharacter_Diffuse.png"
    );
}
```

## Best Practices Summary

### Organization

1. **Hierarchical Structure**: Type → Category → Specific Use
2. **Character-Centric**: Each character has its own complete asset folder
3. **Shared Resources**: Common animations and assets in shared folders
4. **Clear Ownership**: Each asset has a clear location and purpose

### Naming

1. **Descriptive Names**: Names clearly indicate content and purpose
2. **Consistent Patterns**: Follow established naming patterns
3. **English Only**: All asset names in English
4. **Variant Support**: Clear naming for variations and alternatives

### Scalability

1. **Project Independence**: Each project manages its own assets
2. **Engine Fallbacks**: Shared engine assets provide fallbacks
3. **Easy Discovery**: Logical organization makes assets easy to find
4. **Tool Integration**: Structure supports automated tools and validation

This asset organization system ensures that Game Engine Kiro projects are:

- **Professional**: Following industry-standard organization
- **Scalable**: Supporting projects from indie to AAA scale
- **Maintainable**: Easy to manage and update assets
- **Discoverable**: Assets are easy to find and use
- **Consistent**: Uniform organization across all projects
