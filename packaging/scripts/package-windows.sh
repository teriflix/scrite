#!/usr/bin/env bash
# Windows packaging script: builds NSIS installer with windeployqt and code signing.
# Intended to be run from Git Bash or MSYS2 on Windows.
# Usage:
#   ./package-windows.sh [--build] [--no-sign] [--type=<version-type>]
#
# Environment variables (or set in packaging.config.local):
#   CODESIGN_TOOL               Path to code signing tool (e.g. signtool.exe or CodeSignTool.exe)
#   WIN_CERT_SUBJECT            Certificate CN for signing
#   SCRITE_OPENSSL_LIBS         Root directory of OpenSSL 1.1 x64 DLLs
#   SCRITE_CRASHPAD_ROOT        Root directory of Crashpad SDK
#   QT_BIN_DIR                  Path to Qt bin directory (optional, uses PATH if not set)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
STAGING_DIR="${PROJECT_ROOT}/packaging/_staging/windows"
ASSETS_DIR="${PROJECT_ROOT}/packaging/assets/windows"
BUILD_DIR="${BUILD_DIR:-build}"

# shellcheck source=/dev/null
. "${SCRIPT_DIR}/_common.sh"

# Parse flags
BUILD=0
SIGN=0
while [ $# -gt 0 ]; do
    case "$1" in
        --build) BUILD=1 ;;
        --no-sign) SIGN=0 ;;
        --sign) SIGN=1 ;;
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
EXE="${PROJECT_ROOT}/${BUILD_DIR}/Scrite.exe"
if [ ! -f "$EXE" ]; then
    die "Executable not found: $EXE\nHave you built the project? Try --build flag."
fi

echo "Packaging Windows NSIS installer..."
echo "  Executable: $EXE"

# Locate windeployqt
WINDEPLOYQT="${QT_BIN_DIR}/windeployqt.exe"
if [ ! -x "$WINDEPLOYQT" ]; then
    WINDEPLOYQT=$(command -v windeployqt.exe) || die "windeployqt.exe not found. Set QT_BIN_DIR environment variable."
fi
echo "  windeployqt: $WINDEPLOYQT"

# Step 1: Create staging directory and copy executable
echo "Preparing staging directory..."
rm -rf "$STAGING_DIR"
mkdir -p "$STAGING_DIR"
cp "$EXE" "$STAGING_DIR/"

# Copy appicon for NSIS
cp "${PROJECT_ROOT}/apps/desktop/appicon.ico" "$STAGING_DIR/"

# Step 2: Sign executable if requested
if [ $SIGN -eq 1 ]; then
    if [ -z "$CODESIGN_TOOL" ]; then
        echo "WARNING: Code signing requested but CODESIGN_TOOL not set. Skipping exe signing."
    else
        echo "Signing Scrite.exe..."
        "$CODESIGN_TOOL" sign /tr http://timestamp.sectigo.com /td sha256 /fd sha256 \
            /n "${WIN_CERT_SUBJECT}" "${STAGING_DIR}/Scrite.exe"
    fi
fi

# Step 3: Copy OpenSSL DLLs
echo "Copying OpenSSL 1.1 DLLs..."
if [ -z "$SCRITE_OPENSSL_LIBS" ]; then
    die "SCRITE_OPENSSL_LIBS not set. Cannot find OpenSSL DLLs."
fi
cp "${SCRITE_OPENSSL_LIBS}/openssl-1.1/x64/bin/libcrypto-1_1-x64.dll" "$STAGING_DIR/"
cp "${SCRITE_OPENSSL_LIBS}/openssl-1.1/x64/bin/libssl-1_1-x64.dll" "$STAGING_DIR/"

# Step 4: Copy Visual C++ redistributable
echo "Copying VC++ redistributable..."
if [ ! -f "${PROJECT_ROOT}/packaging/assets/windows/vcredist_x64.exe" ]; then
    die "vcredist_x64.exe not found at packaging/assets/windows/\nAdd the Visual C++ redistributable manually."
fi
cp "${PROJECT_ROOT}/packaging/assets/windows/vcredist_x64.exe" "$STAGING_DIR/"

# Step 5: Copy crashpad_handler if available
if [ -n "$SCRITE_CRASHPAD_ROOT" ] && [ -f "${SCRITE_CRASHPAD_ROOT}/bin/crashpad_handler.exe" ]; then
    echo "Copying crashpad_handler..."
    cp "${SCRITE_CRASHPAD_ROOT}/bin/crashpad_handler.exe" "$STAGING_DIR/"
fi

# Step 6: Run windeployqt to populate with Qt files
echo "Running windeployqt..."
"$WINDEPLOYQT" \
    --qmldir "${PROJECT_ROOT}/apps/desktop/qml" \
    --no-compiler-runtime \
    --no-translations \
    "$STAGING_DIR/"

# Step 7: Copy additional assets
echo "Copying additional assets..."
cp "${ASSETS_DIR}/qt.conf" "$STAGING_DIR/"
cp "${ASSETS_DIR}/FileAssociation.nsh" "$STAGING_DIR/"
cp "${ASSETS_DIR}/license.txt" "$STAGING_DIR/"

# Step 8: Run CPack NSIS generator
echo "Creating NSIS installer with CPack..."
cd "${PROJECT_ROOT}"

CPACK_ARGS=("-DSCRITE_STAGING_DIR=${STAGING_DIR}")
if [ -n "$VERSION_SUFFIX" ]; then
    CPACK_ARGS+=("-DSCRITE_VERSION_SUFFIX=${VERSION_SUFFIX}")
fi

cpack --config "${PROJECT_ROOT}/${BUILD_DIR}/CPackConfig.cmake" \
    -G NSIS \
    -B "${STAGING_DIR}" \
    -P "${STAGING_DIR}" \
    "${CPACK_ARGS[@]}"

# Find the generated EXE
SETUP_EXE=$(find "${PROJECT_ROOT}/binary/packages" -name "Scrite-${VERSION}${VERSION_SUFFIX}-64bit-Setup.exe" -type f | head -1)
if [ -z "$SETUP_EXE" ] || [ ! -f "$SETUP_EXE" ]; then
    die "Setup executable not found after packaging"
fi

# Step 9: Sign installer if requested
if [ $SIGN -eq 1 ] && [ -n "$CODESIGN_TOOL" ]; then
    echo "Signing installer..."
    "$CODESIGN_TOOL" sign /tr http://timestamp.sectigo.com /td sha256 /fd sha256 \
        /n "${WIN_CERT_SUBJECT}" "$SETUP_EXE"
fi

# Clean up
rm -rf "$STAGING_DIR"

print_manifest "$SETUP_EXE"
print_hint "Windows packaging complete!"
