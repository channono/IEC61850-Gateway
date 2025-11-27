#!/bin/bash
# Build script for Linux x86-64 (native)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-linux-x86"

echo "================================================"
echo "Building for Linux x86-64 (native)"
echo "================================================"
echo ""

# Check if we're on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "Warning: This script is intended for Linux systems."
    echo "Current OS: $OSTYPE"
fi

# Check for required tools
echo "Checking for required build tools..."
MISSING_TOOLS=()

if ! command -v cmake &> /dev/null; then
    MISSING_TOOLS+=("cmake")
fi

if ! command -v g++ &> /dev/null; then
    MISSING_TOOLS+=("g++")
fi

if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
    echo "Error: Missing required tools: ${MISSING_TOOLS[*]}"
    echo ""
    echo "Install with:"
    echo "  Ubuntu/Debian: sudo apt-get install cmake g++ ninja-build"
    echo "  Fedora/RHEL:   sudo dnf install cmake gcc-c++ ninja-build"
    exit 1
fi

echo "✓ All required tools found"
echo ""

# Clean and create build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Running CMake..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      ..

echo ""
echo "Compiling..."
make -j$(nproc 2>/dev/null || echo 4)

echo ""
echo "✓ Linux x86-64 build complete!"
echo "  Executable: $BUILD_DIR/iec61850-opcua-gateway"
echo ""

# Package
echo "Creating distribution package..."
"$SCRIPT_DIR/package.sh" linux-x86_64
