#!/bin/bash

echo "Building Game Engine Kiro..."

# Check if dependencies are installed
if [ ! -d "vcpkg" ] && [ ! -f "/usr/include/GLFW/glfw3.h" ]; then
    echo "Dependencies not found. Running setup..."
    chmod +x setup_dependencies.sh
    ./setup_dependencies.sh
    if [ $? -ne 0 ]; then
        echo "Failed to setup dependencies!"
        exit 1
    fi
fi

# Create build directory
mkdir -p build
cd build

# Configure CMake
if [ -d "../vcpkg" ]; then
    echo "Using vcpkg toolchain..."
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
else
    echo "Using system libraries..."
    cmake .. -DCMAKE_BUILD_TYPE=Release
fi

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    echo "Make sure all dependencies are installed by running ./setup_dependencies.sh"
    exit 1
fi

# Build the project
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "========================================"
echo "Build completed successfully!"
echo "========================================"
echo "Executable location: build/GameExample"
echo "Assets copied to: build/assets/"
echo ""
echo "To run the game:"
echo "  cd build"
echo "  ./GameExample"
echo ""