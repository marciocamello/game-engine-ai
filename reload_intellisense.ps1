# Script para recarregar IntelliSense no VS Code
# Usage: .\reload_intellisense.ps1

Write-Host "Reloading IntelliSense for Game Engine Kiro..." -ForegroundColor Green

# Verificar se o build existe
if (-not (Test-Path "build")) {
    Write-Host "Build directory not found. Running build first..." -ForegroundColor Yellow
    & ".\build.bat"
}

# Verificar se compile_commands.json existe
if (Test-Path "build\compile_commands.json") {
    Write-Host "Found compile_commands.json" -ForegroundColor Green
} else {
    Write-Host "compile_commands.json not found. Generating..." -ForegroundColor Yellow
    
    # Gerar compile_commands.json
    Set-Location build
    cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
    Set-Location ..
}

# Verificar caminhos de include
Write-Host "Checking include paths..." -ForegroundColor Blue

$includePaths = @(
    "include",
    "src", 
    "examples",
    "vcpkg_installed\x64-windows\include",
    "vcpkg\installed\x64-windows\include"
)

foreach ($path in $includePaths) {
    if (Test-Path $path) {
        Write-Host "  ✓ $path" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $path (not found)" -ForegroundColor Red
    }
}

# Verificar headers principais
Write-Host "Checking main headers..." -ForegroundColor Blue

$mainHeaders = @(
    "include\Core\Engine.h",
    "include\Core\Math.h",
    "include\Graphics\Camera.h",
    "include\Input\InputManager.h"
)

foreach ($header in $mainHeaders) {
    if (Test-Path $header) {
        Write-Host "  ✓ $header" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $header (not found)" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "IntelliSense reload completed!" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Restart VS Code completely (Ctrl+Shift+P -> 'Developer: Reload Window')" -ForegroundColor White
Write-Host "2. Or use Command Palette -> 'C/C++: Reset IntelliSense Database'" -ForegroundColor White
Write-Host "3. Wait for IntelliSense to reindex (check bottom status bar)" -ForegroundColor White