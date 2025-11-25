#!/bin/bash

# Exit on error
set -e

echo "🔧 Starting dependency setup..."

# Create a deps directory
mkdir -p deps
cd deps

# 1. Build libiec61850
if [ ! -d "libiec61850" ]; then
    echo "📦 Cloning libiec61850..."
    git clone https://github.com/mz-automation/libiec61850.git
else
    echo "📦 libiec61850 already cloned."
fi

echo "🔨 Building libiec61850..."
cd libiec61850
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$(pwd)/../../install
make -j$(nproc)
make install
echo "✅ libiec61850 installed to $(pwd)/../../install"

cd ../..

# 2. Check other dependencies (optional, just a reminder)
echo "ℹ️  Please ensure other dependencies are installed via brew:"
echo "   brew install open62541 spdlog yaml-cpp pugixml"

echo "🎉 Dependency setup complete!"
echo "   To use the local libiec61850, update your CMakeLists.txt to look in $(pwd)/install"
