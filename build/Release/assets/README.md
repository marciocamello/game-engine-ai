# Game Engine Kiro - Assets Directory

This directory contains default assets for Game Engine Kiro, providing standard resources that cannot be generated programmatically.

## Directory Structure

```
assets/
├── audio/          # Audio files for testing and examples
├── shaders/        # GLSL shader files
├── textures/       # Image files for textures
└── README.md       # This file
```

## Audio Assets

### Supported Formats

- ✅ **WAV**: Uncompressed audio for sound effects
- ✅ **OGG**: Compressed audio for music and longer clips
- ❌ **MP3**: Not supported (patent concerns)

### Current Audio Files

- `file_example_WAV_5MG.wav` - Sample WAV file for testing (29.98s, stereo, 44.1kHz)
- `file_example_OOG_1MG.ogg` - Sample OGG file for testing (75.04s, stereo, 44.1kHz)
- `file_example_MP3_5MG.mp3` - Sample MP3 file (not supported, used for error testing)

### Usage in Code

```cpp
// Load audio clips
auto wavClip = audioEngine->LoadAudioClip("assets/audio/file_example_WAV_5MG.wav");
auto oggClip = audioEngine->LoadAudioClip("assets/audio/file_example_OOG_1MG.ogg");

// Play audio with 3D positioning
uint32_t sourceId = audioEngine->CreateAudioSource();
audioEngine->PlayAudioSource(sourceId, wavClip);
audioEngine->SetAudioSourcePosition(sourceId, Math::Vec3(10.0f, 0.0f, 5.0f));
```

### Testing Controls (in GameExample)

- **F3**: Play WAV audio sample
- **F4**: Play OGG audio sample
- **F5**: Stop all audio playback

## Shaders Directory

Contains GLSL shader files for the graphics pipeline:

- Vertex shaders (`.vert`)
- Fragment shaders (`.frag`)
- Compute shaders (`.comp`) - future use

## Textures Directory

Contains image files for texture mapping:

- PNG, JPG, TGA formats supported
- Default textures for fallback scenarios
- Test textures for development

## Asset Guidelines

### For Developers Adding Assets

1. **Audio Files**:

   - Use WAV for short sound effects (< 5 seconds)
   - Use OGG for music and longer audio clips
   - Keep file sizes reasonable for repository
   - Test with both mono and stereo configurations

2. **Texture Files**:

   - Use PNG for images with transparency
   - Use JPG for photographs and complex images
   - Power-of-2 dimensions recommended for GPU efficiency
   - Include fallback textures for error scenarios

3. **Shader Files**:
   - Follow GLSL version compatibility requirements
   - Include comments for complex shader logic
   - Test on multiple GPU vendors when possible

### Asset Naming Conventions

- Use descriptive names: `explosion_sound.wav`, `grass_texture.png`
- Include format in name when helpful: `menu_music.ogg`
- Use lowercase with underscores: `player_footstep.wav`
- Avoid spaces and special characters in filenames

## Build Process Integration

Assets are automatically copied to the build directory during compilation:

```
build/Release/assets/  # Assets copied here for runtime access
```

The CMake build system handles asset copying, ensuring all assets are available at runtime.

## Performance Considerations

### Audio Assets

- WAV files load faster but use more memory
- OGG files use less disk space but require decompression
- Consider audio quality vs. file size trade-offs

### Texture Assets

- Larger textures use more GPU memory
- Consider mipmap generation for distant objects
- Use appropriate compression for target platforms

### Shader Assets

- Complex shaders impact GPU performance
- Consider shader compilation time during development
- Profile shader performance on target hardware

## Future Enhancements

### Planned Asset Support

- [ ] 3D model files (FBX, GLTF)
- [ ] Animation files
- [ ] Font files for text rendering
- [ ] Configuration files (JSON, XML)

### Asset Pipeline

- [ ] Asset preprocessing and optimization
- [ ] Automatic asset validation
- [ ] Asset dependency tracking
- [ ] Hot-reloading for development

## Contributing

When adding new assets:

1. Ensure assets are legally usable (public domain, CC0, or owned)
2. Keep file sizes reasonable for repository storage
3. Test assets with the engine before committing
4. Update this README with new asset information
5. Follow the established naming conventions

## License

Assets in this directory may have different licenses than the engine code. Check individual asset files for specific licensing information. When in doubt, assume assets are for development and testing purposes only.
