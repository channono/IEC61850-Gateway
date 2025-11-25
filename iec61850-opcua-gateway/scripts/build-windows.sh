#!/bin/bash
# Build script for Windows x64 using MinGW-w64 cross-compiler

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-windows"

echo "================================================"
echo "Cross-compiling for Windows x64"
echo "================================================"
echo ""

# Check if MinGW-w64 is installed
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "Error: MinGW-w64 not found!"
    echo "Please install it with: brew install mingw-w64"
    exit 1
fi

echo "✓ MinGW-w64 found"
echo ""

# Clean and create build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Running CMake with Windows toolchain..."
cmake -DCMAKE_TOOLCHAIN_FILE="$PROJECT_DIR/cmake/Toolchain-mingw-w64.cmake" \
      -DCMAKE_BUILD_TYPE=Release \
      ..

echo ""
echo "Compiling..."
make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "✓ Windows build complete!"
echo "  Executable: $BUILD_DIR/iec61850-opcua-gateway.exe"
echo ""

# Package
echo "Creating distribution package..."
"$SCRIPT_DIR/package.sh" windows
