# Game Engine Kiro - Development Script for PowerShell
# Usage: .\dev.ps1 [command]
# Commands: setup, build, clean, run, test

param(
    [Parameter(Position = 0)]
    [string]$Command = "help"
)

function Show-Help {
    Write-Host "Game Engine Kiro - Development Commands" -ForegroundColor Cyan
    Write-Host "=======================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Usage: .\dev.ps1 [command]" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Commands:" -ForegroundColor Green
    Write-Host "  setup   - Install dependencies using vcpkg" -ForegroundColor White
    Write-Host "  build   - Build the project" -ForegroundColor White
    Write-Host "  clean   - Clean build directory" -ForegroundColor White
    Write-Host "  run     - Run the game" -ForegroundColor White
    Write-Host "  test    - Test system setup" -ForegroundColor White
    Write-Host "  help    - Show this help message" -ForegroundColor White
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Green
    Write-Host "  .\dev.ps1 setup" -ForegroundColor Gray
    Write-Host "  .\dev.ps1 build" -ForegroundColor Gray
    Write-Host "  .\dev.ps1 run" -ForegroundColor Gray
}

function Invoke-Setup {
    Write-Host "Setting up dependencies..." -ForegroundColor Yellow
    if (Test-Path "setup_dependencies.bat") {
        & ".\setup_dependencies.bat"
    }
    else {
        Write-Error "setup_dependencies.bat not found!"
        exit 1
    }
}

function Invoke-Build {
    Write-Host "Building project..." -ForegroundColor Yellow
    if (Test-Path "build.bat") {
        & ".\build.bat"
    }
    else {
        Write-Error "build.bat not found!"
        exit 1
    }
}

function Invoke-Clean {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build"
        Write-Host "Build directory cleaned successfully!" -ForegroundColor Green
    }
    else {
        Write-Host "Build directory does not exist." -ForegroundColor Gray
    }
}

function Invoke-Run {
    Write-Host "Running game..." -ForegroundColor Yellow
    $exePath = ".\build\Release\GameExample.exe"
    if (Test-Path $exePath) {
        Set-Location ".\build\Release"
        & ".\GameExample.exe"
        Set-Location "..\..\"
    }
    else {
        Write-Error "Game executable not found at $exePath"
        Write-Host "Please build the project first using: .\dev.ps1 build" -ForegroundColor Yellow
        exit 1
    }
}

function Invoke-Test {
    Write-Host "Testing system setup..." -ForegroundColor Yellow
    if (Test-Path "test_setup.bat") {
        & ".\test_setup.bat"
    }
    else {
        Write-Error "test_setup.bat not found!"
        exit 1
    }
}

# Main command dispatcher
switch ($Command.ToLower()) {
    "setup" { Invoke-Setup }
    "build" { Invoke-Build }
    "clean" { Invoke-Clean }
    "run" { Invoke-Run }
    "test" { Invoke-Test }
    "help" { Show-Help }
    default { 
        Write-Host "Unknown command: $Command" -ForegroundColor Red
        Write-Host ""
        Show-Help
        exit 1
    }
}