# Shared Materials

Standard material definitions that can be used across projects.

## Available Materials

- `default.json` - Default material with basic properties
- `debug.json` - Debug material with wireframe and color coding
- `pbr_default.json` - Default PBR material
- `unlit.json` - Unlit material for UI and effects

## Material Format

Materials are defined in JSON format with the following structure:

```json
{
  "name": "MaterialName",
  "type": "PBR",
  "properties": {
    "albedo": [1.0, 1.0, 1.0, 1.0],
    "metallic": 0.0,
    "roughness": 0.5,
    "emission": [0.0, 0.0, 0.0]
  },
  "textures": {
    "diffuse": "textures/default_diffuse.png",
    "normal": "textures/default_normal.png",
    "specular": "textures/default_specular.png"
  }
}
```
