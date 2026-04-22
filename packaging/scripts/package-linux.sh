#!/usr/bin/env bash
# Linux packaging script: builds AppImage using linuxdeployqt or appimagetool.
# Usage:
#   ./package-linux.sh [--build] [--use-appimagetool] [--type=<version-type>]
#
# Environment variables (or set in packaging.config.local):
#   QT_BIN_DIR                  Path to Qt bin directory (optional, uses PATH if not set)
#   LINUXDEPLOYQT_PATH          Path to linuxdeployqt executable (auto-detected if not set)
#   APPIMAGETOOL_PATH           Path to appimagetool executable (used if --use-appimagetool)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
STAGING_DIR="${PROJECT_ROOT}/packaging/_staging/linux"
ASSETS_DIR="${PROJECT_ROOT}/packaging/assets/linux"
BUILD_DIR="${BUILD_DIR:-build}"

# shellcheck source=/dev/null
. "${SCRIPT_DIR}/_common.sh"

# Parse flags
BUILD=0
USE_APPIMAGETOOL=0
while [ $# -gt 0 ]; do
    case "$1" in
        --build) BUILD=1 ;;
        --use-appimagetool) USE_APPIMAGETOOL=1 ;;
        --type=*) VERSION_SUFFIX="-${1#--type=}" ;;
        *) die "Unknown option: $1" ;;
    esac
    shift
done

if [ $BUILD -eq 1 ]; then
    BUILD_VERSION_TYPE=""
    if [ -n "$VERSION_SUFFIX" ]; then
        BUILD_VERSION_TYPE="${VERSION_SUFFIX#-}"
    fi
    ensure_build "$BUILD_VERSION_TYPE"
fi

# Verify executable exists
EXE="${PROJECT_ROOT}/${BUILD_DIR}/Scrite"
if [ ! -f "$EXE" ]; then
    die "Executable not found: $EXE\nHave you built the project? Try --build flag."
fi

echo "Packaging Linux AppImage..."
echo "  Executable: $EXE"

# Step 1: Set up AppDir
echo "Setting up AppDir structure..."
APPDIR="${STAGING_DIR}/Scrite.AppDir"
rm -rf "${APPDIR}"
mkdir -p "${APPDIR}/usr"

# Step 2: Install via CMake with FHS layout
echo "Installing to AppDir..."
cmake --install "${PROJECT_ROOT}/${BUILD_DIR}" \
    --prefix "${APPDIR}/usr" \
    --component Unspecified

# Ensure desktop file is present
mkdir -p "${APPDIR}/usr/share/applications"
cp "${ASSETS_DIR}/Scrite.desktop" "${APPDIR}/usr/share/applications/"

# Step 3: Deploy Qt and dependencies
if [ $USE_APPIMAGETOOL -eq 1 ]; then
    # Use appimagetool directly (manual dependency bundling)
    echo "Using appimagetool for AppImage creation..."

    APPIMAGETOOL="${APPIMAGETOOL_PATH:-$(command -v appimagetool)}"
    if [ -z "$APPIMAGETOOL" ] || [ ! -x "$APPIMAGETOOL" ]; then
        die "appimagetool not found. Set APPIMAGETOOL_PATH or install appimagetool."
    fi

    # Create AppImage directly
    APPIMAGE="${PROJECT_ROOT}/binary/packages/Scrite-${VERSION}${VERSION_SUFFIX}-x86_64.AppImage"
    mkdir -p "$(dirname "$APPIMAGE")"

    "$APPIMAGETOOL" "${APPDIR}" "$APPIMAGE"
else
    # Use linuxdeployqt (preferred)
    echo "Using linuxdeployqt for AppImage creation..."

    LINUXDEPLOYQT="${LINUXDEPLOYQT_PATH:-$(command -v linuxdeployqt)}"
    if [ -z "$LINUXDEPLOYQT" ] || [ ! -x "$LINUXDEPLOYQT" ]; then
        die "linuxdeployqt not found. Set LINUXDEPLOYQT_PATH or install linuxdeployqt."
    fi

    # linuxdeployqt expects the desktop file to be in a specific location
    DESKTOP_FILE="${APPDIR}/usr/share/applications/Scrite.desktop"
    if [ ! -f "$DESKTOP_FILE" ]; then
        die "Desktop file not found: $DESKTOP_FILE"
    fi

    APPIMAGE="${PROJECT_ROOT}/binary/packages/Scrite-${VERSION}-x86_64.AppImage"
    mkdir -p "$(dirname "$APPIMAGE")"

    # Run linuxdeployqt to bundle Qt and create AppImage
    "$LINUXDEPLOYQT" "$DESKTOP_FILE" \
        -appimage \
        -qmldir="${PROJECT_ROOT}/apps/desktop/qml" \
        -no-translations \
        -no-copy-copyright-files \
        -unsupported-allow-new-glibc

    # linuxdeployqt outputs the AppImage in the current directory
    TEMP_APPIMAGE="$(find . -maxdepth 1 -name '*.AppImage' -type f | head -1)"
    if [ -n "$TEMP_APPIMAGE" ]; then
        mv "$TEMP_APPIMAGE" "$APPIMAGE"
    fi
fi

# Verify AppImage was created
if [ ! -f "$APPIMAGE" ]; then
    die "AppImage not created: $APPIMAGE"
fi

# Make it executable
chmod +x "$APPIMAGE"

# Clean up
rm -rf "$STAGING_DIR"

print_manifest "$APPIMAGE"
print_hint "Linux packaging complete!"
