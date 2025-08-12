# Shared Configurations

Default configuration files for the engine and projects.

## Available Configurations

- `default_engine_config.json` - Default engine configuration
- `default_project_config.json` - Default project configuration template
- `module_defaults.json` - Default settings for each engine module

## Usage

These configurations serve as:

- Templates for new projects
- Fallback configurations when project-specific configs are missing
- Reference for available configuration options

## Configuration Hierarchy

1. Project-specific configuration (`projects/{ProjectName}/config/`)
2. Shared default configuration (`shared/configs/`)
3. Built-in engine defaults (hardcoded fallbacks)
