# Scrite Packaging

This directory contains the CPack-based cross-platform packaging system for Scrite. It produces distributable packages for macOS (.dmg), Windows (.exe installer), and Linux (.AppImage).

## Quick Start

### Build
```bash
./scripts/build.sh
```

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
```bat
.\scripts\package-windows.bat [--build] [--sign]
```

Creates an NSIS installer with optional code signing.

**Prerequisites:**
- Visual Studio (MSVC 2022+)
- Qt 6.8+
- NSIS 3.0+
- Code signing tool (optional, e.g., CodeSignTool or signtool.exe)

**Setup (one-time):**
```bat
copy packaging.config.local.bat.example packaging.config.local.bat
:: Edit packaging.config.local.bat with your environment
:: Set QT_BIN_DIR, CMAKE_DIR, SCRITE_OPENSSL_LIBS, SCRITE_CRASHPAD_ROOT, etc.
```

The Visual C++ redistributable is downloaded automatically by the script if not already present.

**Example:**
```bat
set SCRITE_OPENSSL_LIBS=C:\path\to\openssl-1.1
set SCRITE_CRASHPAD_ROOT=C:\path\to\crashpad
.\scripts\package-windows.bat --build
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
# macOS / Linux — build and package without signing
./scripts/package-macos.sh --build
./scripts/package-linux.sh --build
```

```bat
:: Windows — build and package without signing
.\scripts\package-windows.bat --build
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
      - name: Build
        run: .\packaging\scripts\build.bat
      - name: Package
        env:
          CodeSignTool: ${{ secrets.CODESIGN_TOOL_PATH }}
          SCRITE_BUSINESS_NAME: ${{ secrets.WIN_CERT_SUBJECT }}
          SCRITE_OPENSSL_LIBS: C:\openssl
          SCRITE_CRASHPAD_ROOT: C:\crashpad
        run: .\packaging\scripts\package-windows.bat --sign
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

### `packaging.config.local` / `packaging.config.local.bat` (git-ignored)

Developer-specific configuration. Copy the appropriate example for your platform and fill in your values:

**macOS / Linux:**
```bash
cp packaging.config.local.example packaging.config.local
```

**Windows:**
```bat
copy packaging.config.local.bat.example packaging.config.local.bat
```

These files are **never committed** (in `.gitignore`) — each developer maintains their own.

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
│       ├── license.txt
│       └── vcredist_x64.exe         # (automatically downloaded by the package script)
├── scripts/
│   ├── _common.sh                   # Shared functions (sourced by shell scripts)
│   ├── build.sh                     # CMake build (macOS/Linux)
│   ├── build.bat                    # CMake build (Windows)
│   ├── package-macos.sh
│   ├── package-windows.bat
│   └── package-linux.sh
├── packaging.config.local.example   # Config template
├── packaging.config.local.bat.example  # Config template (Windows)
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
- Run `build.sh` (macOS/Linux) or `build.bat` (Windows) first to generate the executable

## Notes

- Packages are output to `binary/packages/`
- Staging directories (e.g., `packaging/_staging/`) are temporary and cleaned up after packaging
- The `packaging/packaging.config.local` file is never committed (`.gitignore`)
- Version is always derived from `CMakeLists.txt` project version — no hardcoding

## Contact

For issues or questions, see: https://github.com/teriflix/scrite
