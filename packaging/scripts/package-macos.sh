#!/usr/bin/env bash
# macOS packaging script: builds DMG with macdeployqt, code signing, and optional notarization.
# Usage:
#   ./package-macos.sh [--build] [--no-sign] [--notarize] [--type=<version-type>]
#
# Environment variables (or set in packaging.config.local):
#   MACOS_SIGNING_IDENTITY      Developer ID Application identity (e.g. "Developer ID Application: VCreate Logic (ABC123)")
#   APPLE_TEAM_ID               Team ID for notarization
#   APPLE_ID_USER               Apple ID email for notarization
#   APPLE_NOTARIZE_PASSWORD     App-specific password from appleid.apple.com
#   QT_BIN_DIR                  Path to Qt bin directory (optional, uses PATH if not set)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
STAGING_DIR="${PROJECT_ROOT}/packaging/_staging/macos"
ASSETS_DIR="${PROJECT_ROOT}/packaging/assets/mac"
BUILD_DIR="${BUILD_DIR:-build}"

# shellcheck source=/dev/null
. "${SCRIPT_DIR}/_common.sh"

# Parse flags
BUILD=0
SIGN=1
NOTARIZE=0
while [ $# -gt 0 ]; do
    case "$1" in
        --build) BUILD=1 ;;
        --no-sign) SIGN=0 ;;
        --notarize) NOTARIZE=1 ;;
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

# Verify app bundle exists (built to binary/ directory by CMake)
APP_BUNDLE="${PROJECT_ROOT}/binary/Scrite.app"
if [ ! -d "$APP_BUNDLE" ]; then
    die "App bundle not found: $APP_BUNDLE\nHave you built the project? Try --build flag."
fi

# Verify this is a Release build (check for debug symbols)
SCRITE_BINARY="${APP_BUNDLE}/Contents/MacOS/Scrite"
if [ ! -f "$SCRITE_BINARY" ]; then
    die "Scrite executable not found in bundle: $SCRITE_BINARY"
fi

if file "$SCRITE_BINARY" | grep -q "with debug_info"; then
    echo "WARNING: App appears to be a Debug build. For release packages, use: ./build.sh (which defaults to Release)"
fi

echo "Packaging macOS DMG..."
echo "  App bundle: $APP_BUNDLE"

# Locate macdeployqt
if [ -n "$QT_BIN_DIR" ] && [ -x "$QT_BIN_DIR/macdeployqt" ]; then
    MACDEPLOYQT="$QT_BIN_DIR/macdeployqt"
else
    MACDEPLOYQT=$(find_qt_tool macdeployqt) || die "macdeployqt not found. Install Qt or set QT_BIN_DIR environment variable."
fi
echo "  macdeployqt: $MACDEPLOYQT"

# Step 1: Run macdeployqt to bundle Qt frameworks
# Remove qt.conf so macdeployqt can create one with correct paths
rm -f "$APP_BUNDLE/Contents/Resources/qt.conf"

MACDEPLOYQT_ARGS=("-qmldir=${PROJECT_ROOT}/apps/desktop/qml" "-verbose=1" "-appstore-compliant" "-hardened-runtime")

echo "Running macdeployqt..."
"$MACDEPLOYQT" "$APP_BUNDLE" "${MACDEPLOYQT_ARGS[@]}"

# Remove SQL plugins (not used by Scrite)
rm -rf "$APP_BUNDLE/Contents/PlugIns/sqldrivers"

# Step 2: Code sign the app bundle (must be done after macdeployqt)
if [ $SIGN -eq 1 ] && [ -n "$MACOS_SIGNING_IDENTITY" ]; then
    echo "Code signing app bundle: $MACOS_SIGNING_IDENTITY"

    # Sign the entire app bundle with --deep to sign all frameworks and plugins
    echo "  Signing: app bundle (deep)"
    codesign --force --deep --options=runtime --sign "$MACOS_SIGNING_IDENTITY" "$APP_BUNDLE"

    # Then re-sign specific executables with WebEngine entitlements for V8 JIT support
    # (This restores the entitlements that were needed for WebEngine to work)
    QT_WEBENGINE_ENTITLEMENTS="$APP_BUNDLE/Contents/Frameworks/QtWebEngineCore.framework/Versions/A/Helpers/QtWebEngineProcess.app/Contents/Resources/QtWebEngineProcess.entitlements"

    if [ -f "$QT_WEBENGINE_ENTITLEMENTS" ]; then
        QT_WEBENGINE_PROCESS="$APP_BUNDLE/Contents/Frameworks/QtWebEngineCore.framework/Versions/A/Helpers/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess"
        if [ -f "$QT_WEBENGINE_PROCESS" ]; then
            echo "  Re-signing: QtWebEngineProcess with entitlements"
            codesign --force --options=runtime --entitlements "$QT_WEBENGINE_ENTITLEMENTS" --sign "$MACOS_SIGNING_IDENTITY" "$QT_WEBENGINE_PROCESS"
        fi

        echo "  Re-signing: Scrite executable with entitlements"
        codesign --force --options=runtime --entitlements "$QT_WEBENGINE_ENTITLEMENTS" --sign "$MACOS_SIGNING_IDENTITY" "$SCRITE_BINARY"
    fi

    # Verify the signature
    echo "Verifying signature..."
    codesign --verify --verbose "$APP_BUNDLE"
else
    echo "  WARNING: Code signing disabled (set MACOS_SIGNING_IDENTITY to enable)"
fi

# Step 3: Generate DMG background from QML
echo "Generating DMG background image..."
rm -f "${ASSETS_DIR}/background.png"

# Copy backdrop PNG to assets dir
cp "${PROJECT_ROOT}/apps/desktop/images/dmgbackdrop.png" "${ASSETS_DIR}/"

# Create temporary directory for QML rendering (must be in assets dir so dmgbackdrop.png is found)
TEMP_QML="${ASSETS_DIR}/dmgbackdrop_gen.qml"
sed "s/{{VERSION}}/Version ${VERSION}${VERSION_SUFFIX}/" "${ASSETS_DIR}/dmgbackdrop.qml" > "$TEMP_QML"

# Find qml tool (Qt 6 replacement for qmlscene)
if [ -n "$QT_BIN_DIR" ] && [ -x "$QT_BIN_DIR/qml" ]; then
    QML_TOOL="$QT_BIN_DIR/qml"
else
    QML_TOOL=$(find_qt_tool qml) || die "qml not found. Install Qt 6 or set QT_BIN_DIR environment variable."
fi

echo "Rendering DMG background with QML..."
cd "${ASSETS_DIR}"
"$QML_TOOL" "$TEMP_QML" 2>&1 | grep -v "qmlscene is deprecated" || true

rm -f "$TEMP_QML" "${ASSETS_DIR}/dmgbackdrop.png"

# Verify background was created
if [ ! -f "${ASSETS_DIR}/background.png" ]; then
    die "Failed to generate background.png from QML"
fi

echo "Background image created:"
file "${ASSETS_DIR}/background.png"
ls -lh "${ASSETS_DIR}/background.png"

# Step 4: Convert background to RGB (remove alpha) for better dmgbuild compatibility
echo "Processing background image..."
identify "${ASSETS_DIR}/background.png" 2>/dev/null || echo "Image generated"
# Convert from RGBA to RGB to improve dmgbuild compatibility
sips -s format bmp "${ASSETS_DIR}/background.png" --out "${ASSETS_DIR}/background_temp.bmp" 2>/dev/null || true
if [ -f "${ASSETS_DIR}/background_temp.bmp" ]; then
    sips -s format png "${ASSETS_DIR}/background_temp.bmp" --out "${ASSETS_DIR}/background.png"
    rm -f "${ASSETS_DIR}/background_temp.bmp"
fi

# Step 5: Create DMG using dmgbuild
echo "Creating DMG with dmgbuild..."
mkdir -p "${PROJECT_ROOT}/binary/packages"

# Create a temporary working directory for DMG contents
rm -rf "${STAGING_DIR}"
mkdir -p "${STAGING_DIR}/dmg_contents"

# Copy the app bundle to staging (dmg_settings.py will reference it)
mkdir -p "${STAGING_DIR}/dmg_contents"
cp -R "$APP_BUNDLE" "${STAGING_DIR}/dmg_contents/"

# Create Applications symlink
ln -s /Applications "${STAGING_DIR}/dmg_contents/Applications"

# Check for dmgbuild (Python tool)
if ! command -v dmgbuild &> /dev/null; then
    die "dmgbuild not found. Install with: pip install dmgbuild"
fi

echo "Running dmgbuild..."

# Copy background.png to dmg_contents
cp "${ASSETS_DIR}/background.png" "${STAGING_DIR}/dmg_contents/"
echo "Copied background.png to staging"
ls -lh "${STAGING_DIR}/dmg_contents/background.png"

# Create dmg_settings.py from template (in the dmg_contents directory)
sed "s/@VERSION@/${VERSION}/g; s/@VERSION_SUFFIX@/${VERSION_SUFFIX}/g" "${ASSETS_DIR}/dmg_settings.py.in" > "${STAGING_DIR}/dmg_contents/dmg_settings.py"
echo "Created dmg_settings.py with content:"
cat "${STAGING_DIR}/dmg_contents/dmg_settings.py"

# Construct the DMG filename with version and optional type suffix
DMG_NAME="Scrite-${VERSION}${VERSION_SUFFIX}"

# Run dmgbuild from the dmg_contents directory (where the files and settings are)
cd "${STAGING_DIR}/dmg_contents"
echo "Running dmgbuild from: $(pwd)"
echo "Files in current directory:"
ls -lh
dmgbuild -s "dmg_settings.py" \
    "${DMG_NAME}" \
    "${DMG_NAME}.dmg"

# Move the DMG from staging dir to packages dir
# Find the created DMG (may have -private suffix on macOS)
DMG_STAGING=$(find "${STAGING_DIR}/dmg_contents" -maxdepth 1 -name "*.dmg" -type f | head -1)
if [ -z "$DMG_STAGING" ] || [ ! -f "$DMG_STAGING" ]; then
    echo "DMG file not found at staging location"
    echo "Files in staging dir:"
    ls -lh "${STAGING_DIR}/dmg_contents/" || true
    die "dmgbuild failed to create DMG"
fi

DMG_FILE="${PROJECT_ROOT}/binary/packages/${DMG_NAME}.dmg"
mv "$DMG_STAGING" "$DMG_FILE"
echo "Moved DMG to: $DMG_FILE"

# Step 6: Notarize if requested
if [ $NOTARIZE -eq 1 ]; then
    if [ -z "$APPLE_ID_USER" ] || [ -z "$APPLE_NOTARIZE_PASSWORD" ]; then
        echo "WARNING: Notarization requested but APPLE_ID_USER or APPLE_NOTARIZE_PASSWORD not set. Skipping."
    else
        echo "Notarizing DMG..."
        xcrun notarytool submit "$DMG_FILE" \
            --apple-id "$APPLE_ID_USER" \
            --password "$APPLE_NOTARIZE_PASSWORD" \
            --team-id "$APPLE_TEAM_ID" \
            --wait

        echo "Stapling notarization to DMG..."
        xcrun stapler staple "$DMG_FILE"
    fi
else
    echo "Notarization skipped (use --notarize to enable)"
fi

# Clean up
rm -f "${ASSETS_DIR}/background.png"
rm -rf "${STAGING_DIR}"

print_manifest "$DMG_FILE"
print_hint "macOS packaging complete!"
