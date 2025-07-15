#!/bin/bash

echo "Setting up Game Engine Kiro dependencies..."

# Function to install dependencies on Ubuntu/Debian
install_ubuntu() {
    echo "Installing dependencies for Ubuntu/Debian..."
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        cmake \
        git \
        libglfw3-dev \
        libglm-dev \
        libgl1-mesa-dev \
        libassimp-dev \
        libopenal-dev \
        libbullet-dev \
        liblua5.4-dev \
        nlohmann-json3-dev \
        libfmt-dev \
        libstb-dev
}

# Function to install dependencies on macOS
install_macos() {
    echo "Installing dependencies for macOS..."
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    brew install \
        cmake \
        glfw \
        glm \
        assimp \
        openal-soft \
        bullet \
        lua \
        nlohmann-json \
        fmt
}

# Function to install using vcpkg (cross-platform)
install_vcpkg() {
    echo "Installing dependencies using vcpkg..."
    
    # Clone vcpkg if it doesn't exist
    if [ ! -d "vcpkg" ]; then
        echo "Cloning vcpkg..."
        git clone https://github.com/Microsoft/vcpkg.git
        if [ $? -ne 0 ]; then
            echo "Failed to clone vcpkg. Please check your internet connection."
            exit 1
        fi
    fi
    
    # Bootstrap vcpkg
    cd vcpkg
    ./bootstrap-vcpkg.sh
    if [ $? -ne 0 ]; then
        echo "Failed to bootstrap vcpkg."
        exit 1
    fi
    
    # Install dependencies
    ./vcpkg install glfw3 glm glad stb assimp openal-soft bullet3 lua nlohmann-json fmt
    
    cd ..
    echo "vcpkg dependencies installed successfully!"
}

# Detect OS and install accordingly
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Check if we're on Ubuntu/Debian
    if command -v apt-get &> /dev/null; then
        install_ubuntu
    else
        echo "Unsupported Linux distribution. Using vcpkg..."
        install_vcpkg
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    install_macos
else
    echo "Unsupported OS. Using vcpkg..."
    install_vcpkg
fi

echo "Dependencies setup complete!"
echo "You can now build the project using:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make"