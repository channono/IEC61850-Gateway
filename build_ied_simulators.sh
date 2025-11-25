#!/bin/bash

# Cross-platform IED Simulator Builder
# Builds libIEC61850 server examples for Windows, macOS Intel, and macOS ARM

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEPS_DIR="${SCRIPT_DIR}/deps"
LIBIEC_DIR="${DEPS_DIR}/libiec61850"
OUTPUT_DIR="${SCRIPT_DIR}/ied_simulators_binaries"

echo "🏗️  IEC61850 IED Simulator Cross-Platform Builder"
echo "=================================================="
echo ""

# Create output directory
mkdir -p "${OUTPUT_DIR}"

# Function to build for a specific platform
build_platform() {
    local PLATFORM=$1
    local CMAKE_ARGS=$2
    local BUILD_DIR="${LIBIEC_DIR}/build_${PLATFORM}"
    local PACKAGE_DIR="${OUTPUT_DIR}/${PLATFORM}"
    
    echo "📦 Building for: ${PLATFORM}"
    echo "----------------------------------------"
    
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    
    # Configure with static linking for single executable
    # Note: macOS doesn't support -static-libgcc/-static-libstdc++
    if [ "${PLATFORM}" = "windows" ]; then
        LINKER_FLAGS="-static-libgcc -static-libstdc++"
    else
        LINKER_FLAGS=""
    fi
    
    cmake .. ${CMAKE_ARGS} \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS}" \
        -DCMAKE_FIND_LIBRARY_SUFFIXES=".a"
    
    # Build
    make -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
    
    # Strip binaries to reduce size
    if [ "${PLATFORM}" != "windows" ]; then
        find examples -type f -perm +111 -name "server_example_*" -exec strip {} \; 2>/dev/null || true
    fi
    
    # Package binaries
    echo "📦 Packaging binaries to: ${PACKAGE_DIR}"
    mkdir -p "${PACKAGE_DIR}"
    
    # Copy server examples
    if [ -d "examples" ]; then
        find examples -type f -perm +111 -name "server_example_*" | while read -r exe; do
            cp "$exe" "${PACKAGE_DIR}/" || true
        done
    fi
    
    # Copy essential libraries if needed
    if [ "${PLATFORM}" = "windows" ]; then
        find . -name "*.dll" -exec cp {} "${PACKAGE_DIR}/" \; || true
    fi
    
    echo "✅ ${PLATFORM} build complete"
    echo ""
}

# 1. macOS ARM (M1/M2/M3/M4)
if [[ $(uname) == "Darwin" ]]; then
    echo "🍎 Building for macOS ARM (M1/M2/M3/M4)"
    echo "   Cross-compiling for Apple Silicon..."
    build_platform "macos_arm64" "-DCMAKE_OSX_ARCHITECTURES=arm64"
else
    echo "⚠️  Skipping macOS ARM (not on macOS)"
fi

# 2. macOS Intel (x86_64)
if [[ $(uname) == "Darwin" ]]; then
    echo "🍎 Building for macOS Intel (x86_64)"
    if [[ $(uname -m) == "arm64" ]]; then
        # Cross-compile on ARM Mac
        build_platform "macos_x86_64" "-DCMAKE_OSX_ARCHITECTURES=x86_64"
    else
        # Native build on Intel Mac
        build_platform "macos_x86_64" "-DCMAKE_OSX_ARCHITECTURES=x86_64"
    fi
else
    echo "⚠️  Skipping macOS Intel (not on macOS)"
fi

# 3. Windows (using MinGW cross-compilation)
echo "🪟 Building for Windows"

# Check if MinGW is available
if command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "✅ MinGW found, building Windows binary"
    build_platform "windows_x64" "-DCMAKE_TOOLCHAIN_FILE=${LIBIEC_DIR}/mingw-w64-x86_64.cmake"
elif command -v brew &> /dev/null; then
    echo "📦 Installing MinGW via Homebrew..."
    brew install mingw-w64 || true
    if command -v x86_64-w64-mingw32-gcc &> /dev/null; then
        build_platform "windows_x64" "-DCMAKE_TOOLCHAIN_FILE=${LIBIEC_DIR}/mingw-w64-x86_64.cmake"
    else
        echo "❌ MinGW installation failed. Please build on Windows or install MinGW manually."
    fi
else
    echo "❌ Cannot build Windows binary: MinGW not found and Homebrew not available"
    echo "   Options:"
    echo "   1. Install MinGW: brew install mingw-w64"
    echo "   2. Build on a Windows machine with MSVC or MinGW"
fi

# Create distribution packages
echo ""
echo "📦 Creating distribution packages..."
cd "${OUTPUT_DIR}"

for platform_dir in */; do
    platform=$(basename "$platform_dir")
    if [ -d "$platform_dir" ] && [ "$(ls -A $platform_dir)" ]; then
        echo "   Packaging: ${platform}"
        
        # Create README for each package
        cat > "${platform_dir}/README.txt" << 'EOF'
IEC61850 IED Simulator Package
================================

Quick Start:
1. Open terminal/command prompt
2. Run: ./server_example_basic_io 10102
   (Windows: server_example_basic_io.exe 10102)

Available Simulators:
- server_example_basic_io: Basic I/O (4 analog + 4 digital)
- server_example_goose: GOOSE publisher
- server_example_control: Control models
- server_example_config_file: SCL config support

Default port: 102 (requires root/admin)
Recommended port: 10102 (no special privileges)

For more info: https://github.com/mz-automation/libiec61850
EOF
        
        # Create tar.gz
        tar -czf "ied_simulators_${platform}.tar.gz" "$platform"
        
        # Create macOS .app bundle if on macOS
        if [[ "$platform" == macos_* && $(uname) == "Darwin" ]]; then
            echo "   Creating macOS .app bundle for ${platform}..."
            APP_NAME="IED_Simulator.app"
            APP_DIR="${platform_dir}/${APP_NAME}"
            mkdir -p "${APP_DIR}/Contents/MacOS"
            mkdir -p "${APP_DIR}/Contents/Resources"
            
            # Copy executables
            cp ${platform_dir}/server_example_* "${APP_DIR}/Contents/MacOS/" 2>/dev/null || true
            
            # Create Info.plist
            cat > "${APP_DIR}/Contents/Info.plist" << 'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>server_example_basic_io</string>
    <key>CFBundleIdentifier</key>
    <string>com.libiec61850.simulator</string>
    <key>CFBundleName</key>
    <string>IED Simulator</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
</dict>
</plist>
PLIST
            
            # Create DMG (optional)
            if command -v hdiutil &> /dev/null; then
                DMG_NAME="IED_Simulator_${platform}.dmg"
                hdiutil create -volname "IED Simulator" -srcfolder "${APP_DIR}" -ov -format UDZO "${DMG_NAME}" 2>/dev/null || true
                [ -f "${DMG_NAME}" ] && echo "   ✅ Created: ${DMG_NAME}"
            fi
        fi
    fi
done

echo ""
echo "🎉 Build Complete!"
echo "=================================================="
echo "Output directory: ${OUTPUT_DIR}"
echo ""
echo "Available packages:"
ls -lh *.tar.gz 2>/dev/null || echo "No packages created"
echo ""
echo "📝 To deploy to another machine:"
echo "   1. Copy the appropriate .tar.gz to the target machine"
echo "   2. Extract: tar -xzf ied_simulators_*.tar.gz"
echo "   3. Run: ./server_example_basic_io 10102"
