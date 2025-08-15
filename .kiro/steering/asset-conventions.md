---
inclusion: always
---

# Asset Organization - Always Applied

## MANDATORY Asset Structure

### Project Asset Organization

```
projects/[ProjectName]/assets/
├── characters/[CharacterName]/    # Character-specific assets
│   ├── [CharacterName].fbx       # Main character model
│   ├── animations/               # Character animations
│   ├── textures/                 # Character textures
│   └── materials/                # Character materials
├── animations/                   # Shared animations
├── meshes/                       # Static meshes (non-character)
├── materials/                    # Project materials
├── textures/                     # Project textures
├── sounds/                       # Audio assets
├── vfx/                          # Visual effects
├── ui/                           # User interface assets
└── input/                        # Input-related assets
```

### Character Asset Rules - MANDATORY

- Each character gets its own folder: `characters/[CharacterName]/`
- Character model: `[CharacterName].fbx`
- Animations in: `characters/[CharacterName]/animations/`
- Textures in: `characters/[CharacterName]/textures/`
- Materials in: `characters/[CharacterName]/materials/`

### Asset Naming Patterns - MANDATORY

```
# Character Assets
XBotCharacter.fbx                 # ✅ Character model
Idle.fbx                          # ✅ Animation
XBotCharacter_Diffuse.png         # ✅ Texture

# Environmental Assets
Building_House_Medieval.fbx       # ✅ Static mesh
Grass_Diffuse_1024.png           # ✅ Environment texture

# Audio Assets
Footstep_Grass_01.wav            # ✅ Sound effect
BGM_Combat_Intense.ogg           # ✅ Background music
```

### Asset Loading Code Pattern

```cpp
// ✅ CORRECT: Character-specific asset loading
void XBotCharacter::LoadCharacterAnimations() {
    resourceManager.LoadAnimation("xbot_idle",
        "characters/XBotCharacter/animations/Idle.fbx");
    resourceManager.LoadAnimation("xbot_walk",
        "characters/XBotCharacter/animations/Walk.fbx");
}

// ❌ WRONG: Generic asset paths
resourceManager.LoadAnimation("idle", "Idle.fbx");  // No character context
```

### Violation = Task Failure

- Any asset placed in wrong location = IMMEDIATE correction required
- Any non-descriptive asset name = IMMEDIATE renaming required
- Any character asset outside character folder = IMMEDIATE reorganization required

This ensures professional asset organization following industry standards.
