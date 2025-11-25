#!/bin/bash
# Build and package release for IEC61850 OPC UA Gateway

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "================================================"
echo "Building IEC61850 OPC UA Gateway - Release Mode"
echo "================================================"
echo ""

# Build
cd "$PROJECT_DIR/build"
echo "Running CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..
echo ""
echo "Compiling..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo ""
echo "✓ Build complete!"
echo ""

# Package
echo "Creating distribution package..."
"$SCRIPT_DIR/package.sh" "$@"
