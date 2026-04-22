#!/usr/bin/env bash
# Common functions and setup for packaging scripts.
# Source this from platform-specific package scripts.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-build}"
STAGING_DIR="${STAGING_DIR:-${SCRIPT_DIR}/../_staging}"

# Extract version from root CMakeLists.txt
VERSION=$(grep -m1 'project(Scrite VERSION' "${PROJECT_ROOT}/CMakeLists.txt" | \
  sed -E 's/.*VERSION ([0-9.]+).*/\1/')

VERSION_TYPE=""
VERSION_SUFFIX=""

echo "Scrite version: $VERSION"

build_package_name() {
    local base_name=$1
    local extension=$2
    if [ -n "$VERSION_SUFFIX" ]; then
        echo "${base_name}-${VERSION}${VERSION_SUFFIX}.${extension}"
    else
        echo "${base_name}-${VERSION}.${extension}"
    fi
}

# Load local configuration if it exists
CONFIG_LOCAL="${SCRIPT_DIR}/../packaging.config.local"
if [ -f "$CONFIG_LOCAL" ]; then
    echo "Loading configuration from $CONFIG_LOCAL"
    # shellcheck source=/dev/null
    . "$CONFIG_LOCAL"
else
    echo "Note: Configuration file not found at $CONFIG_LOCAL"
    echo "  (This is OK if you set environment variables directly)"
fi

# Utility functions
die() {
    echo "ERROR: $*" >&2
    exit 1
}

need_tool() {
    local tool=$1
    if ! command -v "$tool" &> /dev/null; then
        die "Required tool not found: $tool"
    fi
}

find_qt_tool() {
    local tool=$1

    # Check if already in PATH
    if command -v "$tool" &> /dev/null; then
        command -v "$tool"
        return 0
    fi

    # Search Qt 6.x versions first (sorted by version descending to get latest)
    # This ensures we skip Qt 5.x and pick the latest Qt 6
    for qt_version_dir in $(find "$HOME/Qt" -maxdepth 1 -name "6.*" -type d 2>/dev/null | sort -rV); do
        for platform in macos gcc_64 msvc2022_64; do
            if [ -x "$qt_version_dir/$platform/bin/$tool" ]; then
                echo "$qt_version_dir/$platform/bin/$tool"
                return 0
            fi
        done
    done

    # Fallback to any Qt version if no Qt 6 found
    for qt_bin_path in \
        "$HOME/Qt/Tools/Qt_Creator/bin" \
        "$HOME/Qt/"*"/macos/bin" \
        "$HOME/Qt/"*"/gcc_64/bin" \
        "$HOME/Qt/"*"/msvc2022_64/bin" \
        "/Applications/Qt/"*"/macos/bin"; do

        if [ -x "$qt_bin_path/$tool" ]; then
            echo "$qt_bin_path/$tool"
            return 0
        fi
    done

    return 1
}

print_manifest() {
    local pkg_file=$1
    if [ ! -f "$pkg_file" ]; then
        die "Package file not found: $pkg_file"
    fi

    local size=$(stat -f%z "$pkg_file" 2>/dev/null || stat -c%s "$pkg_file")
    local sha256=$(shasum -a 256 "$pkg_file" | awk '{print $1}')

    echo "Package created: $(basename "$pkg_file")"
    echo "  Size: $size bytes"
    echo "  SHA256: $sha256"
    echo "  Path: $pkg_file"
}

ensure_cmake_in_path() {
    if ! command -v cmake &> /dev/null; then
        echo "cmake not found in PATH. Searching Qt installations..."

        # Try Qt Tools CMake (standard location from Qt Online Installer)
        if [ -x "$HOME/Qt/Tools/CMake/CMake.app/Contents/bin/cmake" ]; then
            echo "Found cmake at: $HOME/Qt/Tools/CMake/CMake.app/Contents/bin/cmake"
            export PATH="$HOME/Qt/Tools/CMake/CMake.app/Contents/bin:$PATH"

            # Also set CMAKE_PREFIX_PATH to find Qt libraries
            # Detect Qt version by looking for recent directories
            for qt_version_dir in "$HOME"/Qt/6.*; do
                if [ -d "$qt_version_dir/macos" ]; then
                    export CMAKE_PREFIX_PATH="$qt_version_dir/macos:${CMAKE_PREFIX_PATH:-}"
                    echo "Set CMAKE_PREFIX_PATH to: $qt_version_dir/macos"
                    return 0
                fi
            done
            return 0
        fi

        # Try common Qt component locations (macos, gcc_64, msvc2022_64)
        for qt_path in \
            "$HOME/Qt/"*"/macos/libexec" \
            "$HOME/Qt/"*"/macos/bin" \
            "$HOME/Qt/"*"/gcc_64/libexec" \
            "$HOME/Qt/"*"/gcc_64/bin" \
            "$HOME/Qt/"*"/msvc2022_64/libexec" \
            "$HOME/Qt/"*"/msvc2022_64/bin"; do

            if [ -x "$qt_path/cmake" ]; then
                echo "Found cmake at: $qt_path/cmake"
                export PATH="$qt_path:$PATH"

                # Extract Qt root from path (e.g., ~/Qt/6.8.0)
                qt_root=$(echo "$qt_path" | sed -E 's|(.*/Qt/[^/]+).*|\1|')
                if [ -d "$qt_root" ]; then
                    export CMAKE_PREFIX_PATH="$qt_root:${CMAKE_PREFIX_PATH:-}"
                    echo "Set CMAKE_PREFIX_PATH to: $qt_root"
                fi
                return 0
            fi
        done

        die "cmake not found. Install Qt with cmake or add it to PATH manually."
    fi
}

ensure_build() {
    local version_type="${1}"

    BUILD_SCRIPT_ARGS=()
    if [ -n "$version_type" ]; then
        BUILD_SCRIPT_ARGS+=("--type=$version_type")
    fi

    "${SCRIPT_DIR}/build.sh" "${BUILD_SCRIPT_ARGS[@]}"
}

print_hint() {
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "$*"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
}
