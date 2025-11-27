#!/bin/bash
# Package script for IEC61850 OPC UA Gateway
# Creates a distributable zip file with all necessary files

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
RELEASE_DIR="$PROJECT_DIR/release"

# Version (can be passed as second argument or read from git)
VERSION=${2:-$(git describe --tags --always 2>/dev/null || echo "dev")}

# Platform detection (can be overridden as first argument for cross-compilation)
if [ -n "$1" ]; then
    PLATFORM="$1"
    case "$PLATFORM" in
        windows|windows-x64)
            EXECUTABLE="iec61850-opcua-gateway.exe"
            BUILD_DIR="$PROJECT_DIR/build-windows"
            PLATFORM="windows-x64"
            ;;
        linux-x86_64|linux-x86)
            EXECUTABLE="iec61850-opcua-gateway"
            BUILD_DIR="$PROJECT_DIR/build-linux-x86"
            PLATFORM="linux-x86_64"
            ;;
        linux-aarch64|linux-arm64)
            EXECUTABLE="iec61850-opcua-gateway"
            BUILD_DIR="$PROJECT_DIR/build-linux-arm64"
            PLATFORM="linux-aarch64"
            ;;
        macos-arm64)
            EXECUTABLE="iec61850-opcua-gateway"
            BUILD_DIR="$PROJECT_DIR/build-macos-arm64"
            PLATFORM="macos-arm64"
            ;;
        macos|macos-universal)
            EXECUTABLE="iec61850-opcua-gateway"
            BUILD_DIR="$PROJECT_DIR/build"
            PLATFORM="macos-universal"
            ;;
        *)
            echo "Error: Unknown platform '$PLATFORM'"
            echo "Supported platforms: windows-x64, linux-x86_64, linux-aarch64, macos-arm64, macos-universal"
            exit 1
            ;;
    esac
elif [[ "$OSTYPE" == "darwin"* ]]; then
    # Auto-detect macOS architecture
    if [[ $(uname -m) == "arm64" ]]; then
        PLATFORM="macos-arm64"
        BUILD_DIR="$PROJECT_DIR/build-macos-arm64"
    else
        PLATFORM="macos-x86_64"
        BUILD_DIR="$PROJECT_DIR/build"
    fi
    EXECUTABLE="iec61850-opcua-gateway"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    PLATFORM="windows-x64"
    EXECUTABLE="iec61850-opcua-gateway.exe"
    BUILD_DIR="$PROJECT_DIR/build-windows"
else
    # Auto-detect Linux architecture
    if [[ $(uname -m) == "aarch64" || $(uname -m) == "arm64" ]]; then
        PLATFORM="linux-aarch64"
        BUILD_DIR="$PROJECT_DIR/build-linux-arm64"
    else
        PLATFORM="linux-x86_64"
        BUILD_DIR="$PROJECT_DIR/build-linux-x86"
    fi
    EXECUTABLE="iec61850-opcua-gateway"
fi

PACKAGE_NAME="gateway-${VERSION}-${PLATFORM}"
PACKAGE_DIR="$RELEASE_DIR/$PACKAGE_NAME"

echo "================================================"
echo "IEC61850 OPC UA Gateway - Packaging Script"
echo "================================================"
echo "Version: $VERSION"
echo "Platform: $PLATFORM"
echo ""

# Check if build exists
if [ ! -f "$BUILD_DIR/$EXECUTABLE" ]; then
    echo "Error: Executable not found at $BUILD_DIR/$EXECUTABLE"
    echo "Please run 'make' in the build directory first."
    exit 1
fi

# Clean old release
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"

echo "Copying files..."

# Copy executable
cp "$BUILD_DIR/$EXECUTABLE" "$PACKAGE_DIR/"
echo "  ✓ Executable"

# Copy www directory (Embedded in binary now)
# cp -r "$PROJECT_DIR/www" "$PACKAGE_DIR/"
# echo "  ✓ WebUI (www/)"

# Copy config directory (create default if not exists)
if [ -d "$PROJECT_DIR/config" ]; then
    cp -r "$PROJECT_DIR/config" "$PACKAGE_DIR/"
else
    mkdir -p "$PACKAGE_DIR/config"
    cat > "$PACKAGE_DIR/config/gateway.yaml" << 'EOF'
# IEC61850 OPC UA Gateway Configuration
version: "1.0"

# Gateway Settings
gateway:
  name: "IEC61850-OPCUA-Gateway"
  port: 6850

# OPC UA Server Settings
opcua:
  port: 4840
  endpoint: "opc.tcp://localhost:4840"

# IED Connections (optional - can be configured via WebUI)
ieds: []
EOF
fi
echo "  ✓ Config files"

# Copy README
cat > "$PACKAGE_DIR/README.md" << 'EOF'
# IEC61850 OPC UA Gateway

## Quick Start

1. **Run the Gateway**
   ```bash
   ./iec61850-opcua-gateway
   ```
   (On Windows: double-click `iec61850-opcua-gateway.exe`)

2. **Open WebUI**
   Open your browser to: http://localhost:6850

3. **Connect to IEDs**
   - Go to "IED Management" tab
   - Add IED connections
   - Start monitoring

## Configuration

Edit `config/gateway.yaml` to customize:
- Gateway port (default: 6850)
- OPC UA endpoint (default: opc.tcp://localhost:4840)
- Pre-configured IED connections

## Directory Structure

```
gateway/
├── iec61850-opcua-gateway    # Main executable
├── www/                       # WebUI files
├── config/                    # Configuration files
│   └── gateway.yaml          # Main config
└── README.md                 # This file
```

## Support

For issues or questions, please contact support.
EOF
echo "  ✓ README.md"

# Create zip archive
cd "$RELEASE_DIR"
if command -v zip &> /dev/null; then
    zip -r "${PACKAGE_NAME}.zip" "$PACKAGE_NAME" >/dev/null
    echo ""
    echo "✓ Package created: $RELEASE_DIR/${PACKAGE_NAME}.zip"
else
    echo ""
    echo "✓ Package directory created: $PACKAGE_DIR"
    echo "  (zip command not found, skipping archive creation)"
fi

echo ""
echo "================================================"
echo "Packaging complete!"
echo "================================================"
