# Shared Engine Assets

This directory contains common assets that can be shared across multiple game projects.

## Directory Structure

- `shaders/` - Common shader files used by the engine
- `textures/` - Default textures and fallback images
- `materials/` - Standard material definitions
- `meshes/` - Basic primitive meshes and common models
- `audio/` - Engine sound effects and audio samples
- `fonts/` - Default fonts for UI and text rendering

## Usage

Game projects can reference these shared assets through the asset path resolution system. The engine will automatically search for assets in both project-specific and shared locations.

## Asset Path Resolution Order

1. Project-specific assets directory (`projects/{ProjectName}/assets/`)
2. Shared engine assets directory (`shared/assets/`)
3. Legacy assets directory (`assets/`) - for backward compatibility

This allows projects to override shared assets with project-specific versions while falling back to shared defaults when needed.
