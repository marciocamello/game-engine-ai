# Script para verificar se a configuração da IDE está correta
# Usage: .\check_ide_setup.ps1

Write-Host "============================================" -ForegroundColor Cyan
Write-Host " Game Engine Kiro - IDE Setup Verification" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# Verificar PowerShell 7
Write-Host "1. Checking PowerShell 7..." -ForegroundColor Yellow
if (Test-Path "C:\Program Files\PowerShell\7\pwsh.exe") {
    Write-Host "   ✓ PowerShell 7 found" -ForegroundColor Green
}
else {
    Write-Host "   ✗ PowerShell 7 not found" -ForegroundColor Red
}

# Verificar VS Code
Write-Host "2. Checking VS Code configuration..." -ForegroundColor Yellow
$vscodeFiles = @(
    ".vscode\settings.json",
    ".vscode\c_cpp_properties.json", 
    ".vscode\tasks.json",
    ".vscode\launch.json"
)

foreach ($file in $vscodeFiles) {
    if (Test-Path $file) {
        Write-Host "   ✓ $file" -ForegroundColor Green
    }
    else {
        Write-Host "   ✗ $file (missing)" -ForegroundColor Red
    }
}

# Verificar estrutura do projeto
Write-Host "3. Checking project structure..." -ForegroundColor Yellow
$projectDirs = @(
    "include\Core",
    "include\Graphics", 
    "include\Input",
    "src\Core",
    "src\Graphics",
    "src\Input",
    "examples"
)

foreach ($dir in $projectDirs) {
    if (Test-Path $dir) {
        Write-Host "   ✓ $dir" -ForegroundColor Green
    }
    else {
        Write-Host "   ✗ $dir (missing)" -ForegroundColor Red
    }
}

# Verificar dependências
Write-Host "4. Checking dependencies..." -ForegroundColor Yellow
if (Test-Path "vcpkg\vcpkg.exe") {
    Write-Host "   ✓ vcpkg installed" -ForegroundColor Green
}
else {
    Write-Host "   ✗ vcpkg not found" -ForegroundColor Red
}

if (Test-Path "vcpkg_installed\x64-windows\include") {
    Write-Host "   ✓ vcpkg packages installed" -ForegroundColor Green
}
else {
    Write-Host "   ✗ vcpkg packages not installed" -ForegroundColor Red
}

# Verificar build
Write-Host "5. Checking build..." -ForegroundColor Yellow
if (Test-Path "build") {
    Write-Host "   ✓ Build directory exists" -ForegroundColor Green
    
    if (Test-Path "build\compile_commands.json") {
        Write-Host "   ✓ compile_commands.json exists" -ForegroundColor Green
    }
    else {
        Write-Host "   ✗ compile_commands.json missing" -ForegroundColor Red
    }
    
    if (Test-Path "build\Release\GameExample.exe") {
        Write-Host "   ✓ Game executable exists" -ForegroundColor Green
    }
    else {
        Write-Host "   ✗ Game executable missing" -ForegroundColor Red
    }
}
else {
    Write-Host "   ✗ Build directory missing" -ForegroundColor Red
}

# Verificar headers principais
Write-Host "6. Checking main headers..." -ForegroundColor Yellow
$mainHeaders = @(
    "include\Core\Engine.h",
    "include\Core\Math.h", 
    "include\Graphics\Camera.h",
    "include\Input\InputManager.h",
    "include\Game\Character.h",
    "include\Game\ThirdPersonCameraSystem.h"
)

foreach ($header in $mainHeaders) {
    if (Test-Path $header) {
        Write-Host "   ✓ $header" -ForegroundColor Green
    }
    else {
        Write-Host "   ✗ $header (missing)" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host " Next Steps for VS Code:" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. Open VS Code in this directory:" -ForegroundColor White
Write-Host "   code ." -ForegroundColor Gray
Write-Host ""
Write-Host "2. Select CMake Kit:" -ForegroundColor White
Write-Host "   Ctrl+Shift+P -> 'CMake: Select a Kit'" -ForegroundColor Gray
Write-Host "   Choose: 'Visual Studio Community 2022 Release - amd64 (with vcpkg)'" -ForegroundColor Gray
Write-Host ""
Write-Host "3. Configure CMake:" -ForegroundColor White
Write-Host "   Ctrl+Shift+P -> 'CMake: Configure'" -ForegroundColor Gray
Write-Host ""
Write-Host "4. Reset IntelliSense:" -ForegroundColor White
Write-Host "   Ctrl+Shift+P -> 'C/C++: Reset IntelliSense Database'" -ForegroundColor Gray
Write-Host ""
Write-Host "5. Reload Window:" -ForegroundColor White
Write-Host "   Ctrl+Shift+P -> 'Developer: Reload Window'" -ForegroundColor Gray
Write-Host ""
Write-Host "After these steps, all C++ syntax errors should be resolved!" -ForegroundColor Green