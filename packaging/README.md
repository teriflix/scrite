# Scrite Packaging

This directory contains the CPack-based cross-platform packaging system for Scrite. It produces distributable packages for macOS (.dmg), Windows (.exe installer), and Linux (.AppImage).

## Quick Start

### Build
```bash
./scripts/build.sh
```

### Package (Platform Auto-Detection)
```bash
./scripts/package.sh
```

The dispatcher script automatically detects your platform and invokes the appropriate packaging script. All packages are created in `binary/packages/`.

## Platform-Specific Usage

### macOS
```bash
./scripts/package-macos.sh [--build] [--no-sign] [--notarize]
```

Creates a signed DMG with optional notarization.

**Prerequisites:**
- Xcode Command Line Tools
- Qt 6.8+
- Optional: `macdeployqt`, `dmgbuild` (from pip), `qmlscene`

**Setup (one-time):**
```bash
cp packaging.config.local.example packaging.config.local
# Edit packaging.config.local with your Developer ID identity
# Get your identity: security find-identity -v -p codesigning
```

**Example:**
```bash
export MACOS_SIGNING_IDENTITY="Developer ID Application: VCreate Logic (ABC123)"
./scripts/package-macos.sh --build
```

### Windows
```bash
./scripts/package-windows.sh [--build] [--sign]
```

Creates an NSIS installer with optional code signing.

**Prerequisites:**
- Visual Studio (MSVC 2022+)
- Qt 6.8+
- NSIS 3.0+
- Git Bash or MSYS2
- Code signing tool (optional, e.g., CodeSignTool or signtool.exe)
- Visual C++ redistributable `.exe` (place in `packaging/assets/windows/vcredist_x64.exe`)

**Setup (one-time):**
```bash
cp packaging.config.local.example packaging.config.local
# Edit packaging.config.local with your environment
# Set SCRITE_OPENSSL_LIBS, SCRITE_CRASHPAD_ROOT, code signing tool path, etc.

# Download and place Visual C++ redistributable:
# Download from: https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist
# Save as: packaging/assets/windows/vcredist_x64.exe
```

**Example (Git Bash):**
```bash
export SCRITE_OPENSSL_LIBS="C:/path/to/openssl-1.1"
export SCRITE_CRASHPAD_ROOT="C:/path/to/crashpad"
./scripts/package-windows.sh --build
```

### Linux
```bash
./scripts/package-linux.sh [--build] [--use-appimagetool]
```

Creates an AppImage using `linuxdeployqt` (default) or `appimagetool`.

**Prerequisites:**
- GCC/Clang
- Qt 6.8+
- `linuxdeployqt` (recommended; auto-detects or set `LINUXDEPLOYQT_PATH`)
  - Get: https://github.com/probonopd/linuxdeployqt/releases
- OR `appimagetool` with `--use-appimagetool` flag
  - Get: https://github.com/AppImage/AppImageKit/releases

**Setup (one-time):**
```bash
# Place linuxdeployqt in your PATH or set LINUXDEPLOYQT_PATH
chmod +x ~/path/to/linuxdeployqt
export LINUXDEPLOYQT_PATH="$HOME/path/to/linuxdeployqt"

# Create config file (optional, unless you need custom Qt paths)
cp packaging.config.local.example packaging.config.local
```

**Example:**
```bash
./scripts/package-linux.sh --build
```

## Unsigned Builds (Testing)

By default, signing is disabled. Code signing is opt-in and only happens if credentials are configured:

```bash
# Build and package without signing (all platforms)
./scripts/package.sh --build

# No certificates required — useful for testing, CI dry-runs, etc.
```

## CI/CD Integration

The packaging system is CI/CD-friendly. In GitHub Actions or similar:

1. **Set secrets** for credentials (see below)
2. **Inject environment variables** before running the package script
3. **Upload artifacts** from `binary/packages/`

### Example GitHub Actions Workflow

```yaml
name: Release Packages
on:
  push:
    tags: ['v*']

jobs:
  build-macos:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install Qt
        run: brew install qt
      - name: Build
        run: ./packaging/scripts/build.sh
      - name: Package
        env:
          MACOS_SIGNING_IDENTITY: ${{ secrets.MACOS_SIGNING_IDENTITY }}
          APPLE_TEAM_ID: ${{ secrets.APPLE_TEAM_ID }}
          APPLE_ID_USER: ${{ secrets.APPLE_ID_USER }}
          APPLE_NOTARIZE_PASSWORD: ${{ secrets.APPLE_NOTARIZE_PASSWORD }}
        run: ./packaging/scripts/package-macos.sh --notarize
      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: macos-packages
          path: binary/packages/

  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install Qt
        run: choco install qt-online
      - name: Build
        run: ./packaging/scripts/build.sh
      - name: Package
        env:
          CODESIGN_TOOL: ${{ secrets.CODESIGN_TOOL_PATH }}
          WIN_CERT_SUBJECT: ${{ secrets.WIN_CERT_SUBJECT }}
          SCRITE_OPENSSL_LIBS: C:/openssl
          SCRITE_CRASHPAD_ROOT: C:/crashpad
        run: ./packaging/scripts/package-windows.sh --sign
      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: windows-packages
          path: binary/packages/

  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install dependencies
        run: sudo apt-get install -y qt6-base linuxdeployqt
      - name: Build
        run: ./packaging/scripts/build.sh
      - name: Package
        run: ./packaging/scripts/package-linux.sh
      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: linux-packages
          path: binary/packages/
```

## Configuration Files

### `packaging.config.local` (git-ignored)

Developer-specific configuration. Copy from `packaging.config.local.example` and fill in your values:

```bash
export MACOS_SIGNING_IDENTITY="Developer ID Application: ..."
export APPLE_ID_USER="your.email@apple.com"
export APPLE_NOTARIZE_PASSWORD="xxxx-xxxx-xxxx-xxxx"
```

This file is **never committed** (in `.gitignore`) — each developer maintains their own.

### Environment Variables

All script configuration can be set via environment variables instead of the config file:

```bash
export MACOS_SIGNING_IDENTITY="..."
export APPLE_ID_USER="..."
./scripts/package-macos.sh --notarize
```

Environment variables override config file values.

## File Structure

```
packaging/
├── cmake/                           # CPack CMake configuration
│   ├── CPackConfig.cmake            # Includes platform-specific configs
│   ├── CPackMacOS.cmake
│   ├── CPackWindows.cmake
│   └── CPackLinux.cmake
├── assets/                          # Platform-specific resources
│   ├── mac/
│   │   ├── dmgbackdrop.qml          # DMG background renderer
│   │   ├── dmg_settings.py.in       # dmgbuild template
│   │   └── background.png           # (generated at package time)
│   ├── linux/
│   │   └── Scrite.desktop
│   └── windows/
│       ├── FileAssociation.nsh      # NSIS helper
│       ├── installer.nsi.in         # NSIS template
│       ├── license.txt
│       ├── qt.conf
│       └── vcredist_x64.exe         # (must be added manually)
├── scripts/
│   ├── _common.sh                   # Shared functions
│   ├── build.sh                     # CMake build
│   ├── package.sh                   # Platform dispatcher
│   ├── package-macos.sh
│   ├── package-windows.sh
│   └── package-linux.sh
├── packaging.config.local.example   # Config template
└── README.md                        # This file
```

## Troubleshooting

### macOS: "Code object is not signed at all" or notarization fails
- Ensure you're using a valid Developer ID Application certificate (not a Developer Certificate)
- Check: `security find-identity -v -p codesigning`
- Verify the certificate is in your Keychain

### Windows: windeployqt not found
- Ensure Qt is installed
- Set `QT_BIN_DIR` to the directory containing `windeployqt.exe`
- Or add it to your PATH

### Linux: linuxdeployqt appimage too large
- This is normal; the AppImage includes the entire Qt runtime
- Use `--use-appimagetool` for a smaller build if you want system Qt instead

### All platforms: "binary/packages" directory not found
- Run `build.sh` first to generate the executable

## Notes

- Packages are output to `binary/packages/`
- Staging directories (e.g., `packaging/_staging/`) are temporary and cleaned up after packaging
- The `packaging/packaging.config.local` file is never committed (`.gitignore`)
- Version is always derived from `CMakeLists.txt` project version — no hardcoding

## Contact

For issues or questions, see: https://github.com/teriflix/scrite
