#!/usr/bin/env bash
# Linux packaging script: builds AppImage using linuxdeploy + linuxdeploy-plugin-qt.
# Usage:
#   ./package-linux.sh [--build] [--use-appimagetool] [--type=<version-type>]
#
# Environment variables (or set in packaging.config.local):
#   LINUXDEPLOY_PATH            Path to linuxdeploy executable (auto-detected if not set)
#   LINUXDEPLOY_QT_PLUGIN_PATH  Path to linuxdeploy-plugin-qt executable (auto-detected if not set)
#   APPIMAGETOOL_PATH           Path to appimagetool executable (used with --use-appimagetool)
#
# Download tools from:
#   linuxdeploy:           https://github.com/linuxdeploy/linuxdeploy/releases
#   linuxdeploy-plugin-qt: https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases

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
# CMake outputs runtime artifacts to binary/ (set via CMAKE_RUNTIME_OUTPUT_DIRECTORY)
EXE="${PROJECT_ROOT}/binary/Scrite"
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

# Remove dev-only and debug artifacts not needed at runtime
rm -rf "${APPDIR}/usr/include" \
       "${APPDIR}/usr/lib/cmake" \
       "${APPDIR}/usr/lib/pkgconfig"
find "${APPDIR}" -name "*.debug" -type f -delete

# Copy Sonnet plugins into Qt's plugin tree.
# cmake --install has no install() rules for these plugins; they are only
# built to binary/kf6/sonnet/ as build artifacts. Sonnet's loader appends
# /kf6/sonnet/ to each Qt library path, and linuxdeploy-plugin-qt's qt.conf
# sets Plugins = ../plugins (= usr/plugins/), so the plugins must live at
# usr/plugins/kf6/sonnet/ to be found at runtime inside the AppImage.
SONNET_BUILD_DIR="${PROJECT_ROOT}/binary/kf6/sonnet"
SONNET_DST="${APPDIR}/usr/plugins/kf6/sonnet"
if [ -d "$SONNET_BUILD_DIR" ] && [ -n "$(ls -A "$SONNET_BUILD_DIR"/*.so 2>/dev/null)" ]; then
    echo "Copying Sonnet plugins to AppDir: usr/plugins/kf6/sonnet/"
    mkdir -p "$SONNET_DST"
    cp -a "$SONNET_BUILD_DIR"/. "$SONNET_DST/"
else
    die "Sonnet plugins not found at $SONNET_BUILD_DIR — build may be incomplete"
fi

# The Sonnet plugins are built with an absolute RPATH pointing to the
# developer's machine (e.g. /home/user/.../binary). Fix this to a relative
# $ORIGIN path so the plugins can find their dependencies (libhunspell, etc.)
# inside the AppImage regardless of which machine it runs on.
# Also explicitly bundle libhunspell — it is not a system-essential library
# and may not be present on all target distributions (openSUSE, minimal Ubuntu).
mkdir -p "${APPDIR}/usr/lib"
for _sonnet_so in "${SONNET_DST}"/*.so; do
    [ -f "$_sonnet_so" ] || continue

    # Patch RPATH to $ORIGIN/../../../lib (resolves to usr/lib/ from usr/plugins/kf6/sonnet/)
    if command -v patchelf &>/dev/null; then
        patchelf --set-rpath '$ORIGIN/../../../lib' "$_sonnet_so"
        echo "  Patched RPATH: $(basename "$_sonnet_so")"
    else
        echo "WARNING: patchelf not found — Sonnet plugin RPATH not patched; spell check may fail"
    fi

    # Bundle libhunspell (and any other non-system deps the plugin needs).
    # Filter out only the truly non-portable low-level glibc/libstdc++ libs.
    while IFS= read -r _dep; do
        case "$(basename "$_dep")" in
            linux-vdso*|ld-linux*|libgcc_s*|libstdc++*|libc.so*|libm.so*|libpthread*|libdl.so*|librt.so*) continue ;;
        esac
        [ -f "$_dep" ] || continue
        _real=$(realpath "$_dep")
        _rname=$(basename "$_real")
        _lname=$(basename "$_dep")
        if [ ! -f "${APPDIR}/usr/lib/$_rname" ]; then
            echo "  Bundling dep: $_rname (for $(basename "$_sonnet_so"))"
            cp "$_real" "${APPDIR}/usr/lib/$_rname"
        fi
        if [ "$_lname" != "$_rname" ] && [ ! -e "${APPDIR}/usr/lib/$_lname" ]; then
            ln -sf "$_rname" "${APPDIR}/usr/lib/$_lname"
        fi
    done < <(ldd "$_sonnet_so" 2>/dev/null | awk '/=>/ { print $3 }')
done
unset _sonnet_so _dep _real _rname _lname

# Ensure desktop file is present
mkdir -p "${APPDIR}/usr/share/applications"
cp "${ASSETS_DIR}/Scrite.desktop" "${APPDIR}/usr/share/applications/"

ARCH="$(uname -m)"
APPIMAGE="${PROJECT_ROOT}/binary/packages/Scrite-${VERSION}${VERSION_SUFFIX}-${ARCH}.AppImage"
mkdir -p "$(dirname "$APPIMAGE")"

# Step 3: Deploy Qt and dependencies
if [ $USE_APPIMAGETOOL -eq 1 ]; then
    # Use appimagetool directly — Qt libraries must already be bundled in AppDir
    echo "Using appimagetool for AppImage creation..."

    APPIMAGETOOL="${APPIMAGETOOL_PATH:-$(command -v appimagetool 2>/dev/null)}"
    if [ -z "$APPIMAGETOOL" ] || [ ! -x "$APPIMAGETOOL" ]; then
        die "appimagetool not found. Set APPIMAGETOOL_PATH or download from https://github.com/AppImage/AppImageKit/releases"
    fi

    "$APPIMAGETOOL" "${APPDIR}" "$APPIMAGE"
else
    # Use linuxdeploy + linuxdeploy-plugin-qt (Qt 6 compatible)
    echo "Using linuxdeploy for AppImage creation..."
    echo "  LINUXDEPLOY_PATH=${LINUXDEPLOY_PATH:-<not set>}"
    echo "  LINUXDEPLOY_QT_PLUGIN_PATH=${LINUXDEPLOY_QT_PLUGIN_PATH:-<not set>}"
    echo "  APPIMAGETOOL_PATH=${APPIMAGETOOL_PATH:-<not set>}"

    LINUXDEPLOY="${LINUXDEPLOY_PATH:-$(command -v linuxdeploy 2>/dev/null)}"
    if [ -z "$LINUXDEPLOY" ] || [ ! -x "$LINUXDEPLOY" ]; then
        die "linuxdeploy not found. Set LINUXDEPLOY_PATH or download from https://github.com/linuxdeploy/linuxdeploy/releases"
    fi

    LINUXDEPLOY_QT_PLUGIN="${LINUXDEPLOY_QT_PLUGIN_PATH:-$(command -v linuxdeploy-plugin-qt 2>/dev/null)}"
    if [ -z "$LINUXDEPLOY_QT_PLUGIN" ] || [ ! -x "$LINUXDEPLOY_QT_PLUGIN" ]; then
        die "linuxdeploy-plugin-qt not found. Set LINUXDEPLOY_QT_PLUGIN_PATH or download from https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases"
    fi

    # Locate Qt 6 qmake — linuxdeploy-plugin-qt uses it to find Qt libraries
    QT_QMAKE=$(find_qt_tool qmake)
    if [ -z "$QT_QMAKE" ]; then
        die "qmake (Qt 6) not found. Add Qt 6 bin directory to PATH or install Qt 6."
    fi
    echo "Using qmake: $QT_QMAKE"

    # Both the Qt plugin and Qt bin must be in PATH for linuxdeploy to find them
    export PATH="$(dirname "$LINUXDEPLOY_QT_PLUGIN"):$(dirname "$QT_QMAKE"):$PATH"

    # linuxdeploy uses ldd to resolve ELF dependencies; ldd won't find Qt libs
    # unless their directory is in LD_LIBRARY_PATH
    QT_LIB_DIR="$(dirname "$QT_QMAKE")/../lib"
    export LD_LIBRARY_PATH="${QT_LIB_DIR}:${PROJECT_ROOT}/binary:${LD_LIBRARY_PATH:-}"
    echo "  QT_LIB_DIR=${QT_LIB_DIR}"

    # Cache the AppImage type-2 runtime next to linuxdeploy so it is only
    # downloaded once. It is passed via APPIMAGETOOL_ADDITIONAL_ARGS at package
    # time so appimagetool (called internally by linuxdeploy) uses it without fetching.
    APPIMAGE_RUNTIME="${APPIMAGE_RUNTIME_PATH:-$(dirname "$LINUXDEPLOY")/runtime-${ARCH}}"
    if [ ! -f "$APPIMAGE_RUNTIME" ]; then
        echo "Downloading AppImage runtime (one-time) to: $APPIMAGE_RUNTIME"
        curl -fsSL -o "$APPIMAGE_RUNTIME" \
            "https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-${ARCH}"
    fi
    echo "Using AppImage runtime: $APPIMAGE_RUNTIME"

    # linuxdeploy-plugin-qt's QMLPATHS env var is silently ignored when the plugin
    # runs as a nested AppImage (env vars don't survive AppImage FUSE mount).
    # Workaround: run qmlimportscanner ourselves against the project QML source
    # directory and pre-populate AppDir/usr/qml/ before linuxdeploy runs.
    # linuxdeploy's ELF scanner then picks up the library deps for the copied plugins.
    QT_QML_DIR="$("$QT_QMAKE" -query QT_INSTALL_QML)"
    QML_SOURCE_DIR="${PROJECT_ROOT}/apps/desktop/qml"
    QMLIMPORTSCANNER="$(dirname "$QT_QMAKE")/../libexec/qmlimportscanner"
    echo "Pre-deploying QML modules from: $QT_QML_DIR"
    mkdir -p "$APPDIR/usr/qml"
    if [ -x "$QMLIMPORTSCANNER" ]; then
        _SCANNER_JSON=$(mktemp /tmp/qml_imports_XXXXXX.json)
        _DEPLOY_PY=$(mktemp /tmp/deploy_qml_XXXXXX.py)

        "$QMLIMPORTSCANNER" \
            -rootPath "$QML_SOURCE_DIR" \
            -importPath "$QT_QML_DIR" \
            > "$_SCANNER_JSON" 2>/dev/null

        cat > "$_DEPLOY_PY" <<'PYEOF'
import json, sys, os, shutil
qt_qml, dest, scanner_file = sys.argv[1], sys.argv[2], sys.argv[3]
with open(scanner_file) as f:
    raw = f.read().strip()
if not raw:
    sys.exit(0)
try:
    data = json.loads(raw)
except json.JSONDecodeError as e:
    print(f"WARNING: qmlimportscanner output could not be parsed: {e}", flush=True)
    sys.exit(0)
for item in data:
    path = item.get("path", "")
    if not path or not os.path.isdir(path):
        continue
    rel = os.path.relpath(path, qt_qml)
    if rel.startswith(".."):
        continue
    dst = os.path.join(dest, rel)
    if not os.path.exists(dst):
        print(f"  Copying QML module: {rel}", flush=True)
        shutil.copytree(path, dst)
PYEOF

        python3 "$_DEPLOY_PY" "$QT_QML_DIR" "$APPDIR/usr/qml" "$_SCANNER_JSON"
        rm -f "$_SCANNER_JSON" "$_DEPLOY_PY"
    else
        echo "WARNING: qmlimportscanner not found at $QMLIMPORTSCANNER — QML modules may be missing"
    fi

    # linuxdeploy only scans usr/bin/Scrite for deps in its initial pass; it never
    # walks usr/qml/. For each pre-populated QML plugin, copy the real (versioned)
    # .so file and recreate the .so.N symlink so nothing in AppDir is dangling.
    echo "Deploying library dependencies for pre-populated QML modules..."
    mkdir -p "$APPDIR/usr/lib"
    _install_lib() {
        local dep_path=$1
        [ -e "$dep_path" ] || return
        local real_path link_name real_name
        real_path=$(realpath "$dep_path")
        real_name=$(basename "$real_path")
        link_name=$(basename "$dep_path")
        if [ ! -f "$APPDIR/usr/lib/$real_name" ]; then
            echo "  $real_name"
            cp "$real_path" "$APPDIR/usr/lib/$real_name"
        fi
        if [ "$link_name" != "$real_name" ] && [ ! -e "$APPDIR/usr/lib/$link_name" ]; then
            ln -sf "$real_name" "$APPDIR/usr/lib/$link_name"
        fi
    }
    while IFS= read -r -d '' qml_so; do
        while IFS= read -r dep_path; do
            _install_lib "$dep_path"
        done < <(
            LD_LIBRARY_PATH="$QT_LIB_DIR:${PROJECT_ROOT}/binary:${LD_LIBRARY_PATH:-}" \
            ldd "$qml_so" 2>/dev/null \
            | awk '/=>/ { print $3 }' \
            | grep -v "^/lib\|^/usr/lib/${ARCH}"
        )
    done < <(find "$APPDIR/usr/qml" -name "*.so" -type f -print0)

    # linuxdeploy-plugin-qt deploys ALL Qt SQL drivers, which pull in optional external
    # database client libraries (MySQL, Firebird, PostgreSQL, ODBC, …) that are unlikely
    # to be installed on every build machine, and that Scrite doesn't use anyway.
    # Solution: temporarily move sqldrivers out of the Qt plugins directory so the Qt
    # plugin never sees them. A trap guarantees they are restored even on failure.
    QT_PLUGINS_DIR="$(realpath "$(dirname "$QT_QMAKE")/../plugins")"
    QT_SQLDRIVERS_DIR="$QT_PLUGINS_DIR/sqldrivers"
    _SQLDRIVERS_TMP=""
    if [ -d "$QT_SQLDRIVERS_DIR" ]; then
        _SQLDRIVERS_TMP=$(mktemp -d)
        mv "$QT_SQLDRIVERS_DIR" "$_SQLDRIVERS_TMP/"
        echo "Temporarily moved Qt SQL drivers to: $_SQLDRIVERS_TMP"
    fi
    _restore_sqldrivers() {
        if [ -n "$_SQLDRIVERS_TMP" ] && [ -d "$_SQLDRIVERS_TMP/sqldrivers" ]; then
            mv "$_SQLDRIVERS_TMP/sqldrivers" "$QT_SQLDRIVERS_DIR"
            rm -rf "$_SQLDRIVERS_TMP"
            _SQLDRIVERS_TMP=""
        fi
    }
    trap _restore_sqldrivers EXIT INT TERM

    # Phase 1: Deploy Qt libraries, plugins, and QML modules into AppDir.
    echo "Running linuxdeploy + Qt plugin (deploy phase)..."
    QMAKE="$QT_QMAKE" \
    "$LINUXDEPLOY" --appdir "$APPDIR" --plugin qt

    _restore_sqldrivers
    trap - EXIT INT TERM

    # Remove debug symbols the Qt plugin may have copied in.
    find "$APPDIR" -name "*.debug" -type f -delete

    # Phase 2: Create the AppImage from the prepared AppDir.
    echo "Creating AppImage..."
    VERSION="$VERSION" \
    APPIMAGETOOL_ADDITIONAL_ARGS="--runtime-file $APPIMAGE_RUNTIME" \
    "$LINUXDEPLOY" --appdir "$APPDIR" --output appimage

    TEMP_APPIMAGE="$(find . -maxdepth 1 -name 'Scrite*.AppImage' -type f | head -1)"
    [ -n "$TEMP_APPIMAGE" ] && mv "$TEMP_APPIMAGE" "$APPIMAGE"
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
