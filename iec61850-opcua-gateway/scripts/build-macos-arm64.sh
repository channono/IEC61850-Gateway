#!/bin/bash
# Build script for macOS Apple Silicon (M1/M2/M3/M4) arm64

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-macos-arm64"

echo "================================================"
echo "Building for macOS Apple Silicon (arm64)"
echo "================================================"
echo ""

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "Error: This script is intended for macOS systems."
    echo "Current OS: $OSTYPE"
    exit 1
fi

# Detect current architecture
CURRENT_ARCH=$(uname -m)
echo "Current architecture: $CURRENT_ARCH"

# Check for required tools
echo "Checking for required build tools..."
if ! command -v cmake &> /dev/null; then
    echo "Error: cmake not found!"
    echo "Install with: brew install cmake"
    exit 1
fi

echo "✓ cmake found"
echo ""

# If on Intel Mac, warn about cross-compilation limitations
if [[ "$CURRENT_ARCH" == "x86_64" ]]; then
    echo "⚠️  WARNING: You are running on an Intel Mac (x86_64)"
    echo "   Cross-compiling to arm64 requires arm64 versions of all dependencies."
    echo "   This build will likely fail unless you have Universal or arm64 dependencies."
    echo ""
    echo "   For production arm64 builds, use:"
    echo "   - An actual Apple Silicon Mac (M1/M2/M3/M4)"
    echo "   - GitHub Actions with macos-14 runner"
    echo ""
    echo "   Attempting build anyway..."
    echo ""
fi

# Clean and create build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Running CMake for arm64..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
      ..

echo ""
echo "Compiling..."
make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "✓ macOS arm64 build complete!"
echo "  Executable: $BUILD_DIR/iec61850-opcua-gateway"
echo ""

# Verify architecture
echo "Verifying binary architecture..."
file "$BUILD_DIR/iec61850-opcua-gateway"
lipo -info "$BUILD_DIR/iec61850-opcua-gateway"
echo ""

# Package
echo "Creating distribution package..."
"$SCRIPT_DIR/package.sh" macos-arm64
