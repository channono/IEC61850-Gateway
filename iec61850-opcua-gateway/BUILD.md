# Cross-Platform Build System

This project uses GitHub Actions to automatically build releases for multiple platforms.

## Supported Platforms

- **macOS** (Universal Binary: arm64 + x86_64)
- **Windows x64** (Cross-compiled using MinGW-w64)

## Local Build Instructions

### Prerequisites

**macOS:**
```bash
# Install build tools
brew install cmake ninja

# For Windows cross-compilation
brew install mingw-w64
```

**libiec61850 Dependency:**
The project expects libiec61850 to be installed at `../deps/install/`. For now, macOS native builds work with system-installed libiec61850.

### Build for macOS

```bash
# Standard build (uses system libiec61850)
./scripts/build-release.sh
```

### Cross-compile for Windows

**Note:** Windows cross-compilation requires libiec61850 to be built for Windows first. This is complex and is better handled by GitHub Actions.

For local development, we recommend using GitHub Actions for Windows builds.

## Automated Releases (GitHub Actions)

### Trigger a Release

**Option 1: Push a tag**
```bash
git tag v1.0.0
git push origin v1.0.0
```

**Option 2: Manual trigger**
- Go to GitHub Actions tab
- Select "Release Build" workflow  
- Click "Run workflow"
- Enter version (e.g., v1.0.0)

### What GitHub Actions Does

1. Sets up macOS runner
2. Installs dependencies (CMake, Ninja, MinGW-w64)
3. Builds libiec61850 for both macOS and Windows
4. Builds gateway for both platforms
5. Creates distribution packages
6. Uploads artifacts and creates GitHub release

### Release Artifacts

Each release includes two zip files:
- `gateway-v1.0.0-macos.zip`
- `gateway-v1.0.0-windows.zip`

Both packages contain:
- Executable
- WebUI (`www/`)
- Configuration files (`config/`)
- README

## Troubleshooting

### macOS Build Issues

**libiec61850 not found:**
```bash
# Install libiec61850 system-wide
# Or place it in ../deps/install/
```

### Windows Cross-Compilation

For Windows builds, we recommend using GitHub Actions which handles all dependencies automatically.

If you need local Windows builds, you'll need to:
1. Build libiec61850 for Windows using MinGW-w64
2. Install it to `../deps/install/`
3. Run `./scripts/build-windows.sh`
