# Cross-Platform Build System

This project uses GitHub Actions to automatically build releases for multiple platforms.

## Supported Platforms

- **Linux x86-64** (Standard 64-bit Linux)
- **Linux aarch64/arm64** (ARM-based Linux systems)
- **macOS Apple Silicon** (M1/M2/M3/M4 arm64)
- **Windows x64** (64-bit Windows)

## Local Build Instructions

### Prerequisites

**macOS:**
```bash
# Install build tools
brew install cmake ninja

# For Windows cross-compilation (optional)
brew install mingw-w64
```

**Linux (Ubuntu/Debian):**
```bash
# Install build tools
sudo apt-get update
sudo apt-get install -y cmake ninja-build build-essential \
  libssl-dev python3 libspdlog-dev libyaml-cpp-dev

# For arm64 cross-compilation (optional)
sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

**Linux (Fedora/RHEL):**
```bash
# Install build tools
sudo dnf install -y cmake ninja-build gcc-c++ \
  openssl-devel python3 spdlog-devel yaml-cpp-devel

# For arm64 cross-compilation (optional)
sudo dnf install -y gcc-aarch64-linux-gnu gcc-c++-aarch64-linux-gnu
```

### Build Commands by Platform

#### Linux x86-64 (native)
```bash
./scripts/build-linux-x86.sh
```

#### Linux arm64/aarch64 (cross-compile)
Requires cross-compilation toolchain installed:
```bash
./scripts/build-linux-arm64.sh
```

#### macOS Apple Silicon (M1/M2/M3/M4)
```bash
./scripts/build-macos-arm64.sh
```

#### Windows x64 (cross-compile from macOS)
Requires MinGW-w64 installed:
```bash
./scripts/build-windows.sh
```

### Manual Build

If you prefer manual control:

```bash
# Create build directory
mkdir -p build && cd build

# Configure (native build)
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..

# For cross-compilation, specify toolchain:
# cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
#   -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-linux-aarch64.cmake ..

# Build
ninja

# Package (optional)
cd ..
./scripts/package.sh <platform> <version>
# Platforms: linux-x86_64, linux-aarch64, macos-arm64, windows-x64
```

## Automated Releases (GitHub Actions)

### Trigger a Release

**Option 1: Push a tag**
```bash
git tag v1.0.0
git push origin v1.0.0
```

**Option 2: Manual trigger**
- Go to GitHub Actions tab
- Select "Multi-Platform Release Build" workflow  
- Click "Run workflow"
- Enter version (e.g., v1.0.0)

### What GitHub Actions Does

The workflow builds the gateway for all supported platforms in parallel:

1. **Linux x86-64 Job** (Ubuntu 22.04 runner)
   - Installs dependencies
   - Builds libiec61850 natively
   - Builds gateway for Linux x86-64
   - Creates distribution package

2. **Linux aarch64 Job** (Docker on Ubuntu runner)
   - Uses QEMU and Docker for arm64 emulation
   - Builds everything inside arm64 Ubuntu container
   - Creates distribution package

3. **macOS Apple Silicon Job** (macOS 14 M1 runner)
   - Builds natively on Apple Silicon
   - Forces arm64 architecture
   - Creates distribution package

4. **Windows x64 Job** (macOS runner with MinGW)
   - Cross-compiles using MinGW-w64
   - Builds libiec61850 and gateway for Windows
   - Creates distribution package

5. **Create Release Job**
   - Collects all platform artifacts
   - Creates GitHub Release with all packages
   - Attaches zip files for each platform

### Release Artifacts

Each release includes four zip files:
- `gateway-v1.0.0-linux-x86_64.zip`
- `gateway-v1.0.0-linux-aarch64.zip`
- `gateway-v1.0.0-macos-arm64.zip`
- `gateway-v1.0.0-windows-x64.zip`

All packages contain:
- Executable (statically linked dependencies where possible)
- Configuration files (`config/`)
- README with quick start guide

## Platform-Specific Notes

### Linux x86-64
- Standard ELF binary
- Runs on Ubuntu, Debian, Fedora, RHEL, CentOS, etc.
- Requires glibc 2.31+ (Ubuntu 20.04+)

### Linux aarch64
- Built for ARMv8-A architecture
- Runs on ARM servers (AWS Graviton, etc.)
- Runs on ARM SBCs (Raspberry Pi 4+, NVIDIA Jetson, etc.)
- Requires glibc 2.31+ (Ubuntu 20.04+)

### macOS Apple Silicon
- Native arm64 binary for M-series chips
- Minimum deployment target: macOS 11.0 (Big Sur)
- Optimized for Apple Silicon performance

### Windows x64
- PE32+ executable
- Statically linked (no DLL dependencies except system)
- Runs on Windows 10/11 64-bit

## Troubleshooting

### Build Failures

**libiec61850 not found:**
The project requires libiec61850 to be built first. The build scripts handle this automatically, but if you encounter issues:
```bash
cd deps/libiec61850
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF ..
make -j$(nproc)
sudo make install  # Or install to custom prefix
```

**Missing dependencies:**
Install the development packages for your platform (see Prerequisites section above).

**Cross-compilation issues:**
Ensure you have the correct cross-compilation toolchain installed. For detailed setup, refer to the GitHub Actions workflow (`.github/workflows/release.yml`) which shows the exact commands.

### Platform-Specific Issues

**Linux arm64**: If testing on actual ARM hardware, native builds are faster than cross-compilation. Use `./scripts/build-linux-x86.sh` on ARM machines (the script auto-detects architecture).

**macOS**: If building on Intel Mac, the script will detect x86_64 and use the standard build directory. For universal binaries, build both architectures separately and use `lipo`.

**Windows**: Cross-compilation from Linux is possible but requires more setup. Using macOS with Homebrew's MinGW-w64 is the most straightforward approach.

## Testing Builds

Verify the binary architecture:

```bash
# Linux
file iec61850-opcua-gateway
# Expected: ELF 64-bit LSB executable

# macOS  
file iec61850-opcua-gateway
lipo -info iec61850-opcua-gateway
# Expected: Mach-O 64-bit executable arm64

# Windows (from Linux/macOS)
file iec61850-opcua-gateway.exe
# Expected: PE32+ executable (console) x86-64
```

Run the executable:
```bash
./iec61850-opcua-gateway --version  # Should display version
./iec61850-opcua-gateway            # Should start server on port 6850
```

## Development Workflow

For active development, use standard CMake workflow:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Run tests
ctest

# Run with debugger
gdb ./iec61850-opcua-gateway
```

The packaging scripts are primarily for release builds. For day-to-day development, work directly in the `build/` directory.
