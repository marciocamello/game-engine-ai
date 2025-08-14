# Examples Directory Migration Notice

## ⚠️ DEPRECATED DIRECTORY

This `examples/` directory has been **deprecated** and is no longer actively maintained or built by the CMake system.

## Migration to Projects Structure

All examples have been migrated to the new `projects/` directory structure for better organization and maintainability.

### Migration Mapping

| Old Location                 | New Location                                  | Status                |
| ---------------------------- | --------------------------------------------- | --------------------- |
| `examples/basic_example.cpp` | `projects/BasicExample/src/basic_example.cpp` | ✅ Migrated           |
| `examples/main.cpp`          | `projects/GameExample/src/main.cpp`           | ✅ Migrated           |
| Other examples               | Reference only                                | 📚 Kept for reference |

## How to Use the New Structure

### Building Examples

```powershell
# Build all projects (including examples)
.\scripts\build_unified.bat --all

# Build specific project
.\scripts\build_unified.bat --project BasicExample
.\scripts\build_unified.bat --project GameExample
```

### Running Examples

```powershell
# Run BasicExample (clean scene navigation)
.\build\projects\BasicExample\Release\BasicExample.exe

# Run GameExample (comprehensive feature demonstration)
.\build\projects\GameExample\Release\GameExample.exe
```

## What Changed

### Benefits of New Structure

1. **Better Organization**: Each project has its own directory with proper CMake configuration
2. **Individual Configurations**: Projects can have their own assets, configs, and dependencies
3. **Cleaner Build System**: No more cluttered CMakeLists.txt with individual executable definitions
4. **Scalability**: Easy to add new projects without modifying the main build system

### Technical Changes

- **CMakeLists.txt**: All `add_executable` statements for examples have been commented out
- **Build System**: Examples are no longer built as part of the main build
- **Asset Management**: Projects use the new asset deployment system
- **Configuration**: Each project can have its own configuration files

## Legacy Files Status

The files in this directory are kept for:

- **Reference purposes**: Developers can still examine the original code
- **Historical context**: Understanding the evolution of the project structure
- **Backup**: In case any functionality was missed during migration

## For Developers

If you need to:

- **Add new examples**: Use the `projects/` structure instead
- **Modify existing examples**: Work with files in `projects/BasicExample/` or `projects/GameExample/`
- **Reference old code**: Files in this directory are still available for reference

## Migration Completion

This migration was completed as part of the namespace class conflict resolution spec (Task 11), which involved:

- Cleaning up legacy examples directory references
- Removing conflicting build targets
- Establishing clear separation between legacy and current structures

---

**For current examples, please use the `projects/` directory structure.**
