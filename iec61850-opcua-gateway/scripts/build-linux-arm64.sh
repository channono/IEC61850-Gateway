#!/bin/bash
# Build script for Linux arm64/aarch64 (cross-compile)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-linux-arm64"

echo "================================================"
echo "Cross-compiling for Linux arm64/aarch64"
echo "================================================"
echo ""

# Check for aarch64 cross-compiler
if ! command -v aarch64-linux-gnu-g++ &> /dev/null; then
    echo "Error: aarch64 cross-compiler not found!"
    echo ""
    echo "Install with:"
    echo "  Ubuntu/Debian: sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu"
    echo "  Fedora/RHEL:   sudo dnf install gcc-aarch64-linux-gnu gcc-c++-aarch64-linux-gnu"
    echo "  macOS:         This is typically done via Docker (see GitHub Actions workflow)"
    exit 1
fi

echo "✓ aarch64 cross-compiler found"
echo ""

# Clean and create build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Running CMake with aarch64 toolchain..."
cmake -DCMAKE_TOOLCHAIN_FILE="$PROJECT_DIR/cmake/Toolchain-linux-aarch64.cmake" \
      -DCMAKE_BUILD_TYPE=Release \
      ..

echo ""
echo "Compiling..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "✓ Linux arm64 build complete!"
echo "  Executable: $BUILD_DIR/iec61850-opcua-gateway"
echo ""

# Package
echo "Creating distribution package..."
"$SCRIPT_DIR/package.sh" linux-aarch64
