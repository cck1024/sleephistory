#!/bin/bash

BUILD_DIR="native-build"
# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory: $BUILD_DIR..."
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR" || exit

echo "Starting CMake configuration..."
# Configure the project
# '..' points to the parent directory where CMakeLists.txt is located
cmake ..

# Check if configuration was successful
if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed!"
    exit 1
fi

echo "Starting compilation..."
# Build the project
# '--build .' is the platform-agnostic way to invoke the underlying build tool
# '-j $(nproc)' uses all available CPU cores for faster compilation
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j $(nproc)

# Check if the build process succeeded
if [ $? -eq 0 ]; then
    echo "--------------------------------------"
    echo "Build successful!"
    echo "Your binary is located in: $BUILD_DIR"
else
    echo "Error: Compilation failed!"
    exit 1
fi