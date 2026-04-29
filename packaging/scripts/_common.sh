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

    # Use PATH only if the tool is Qt 6
    if command -v "$tool" &> /dev/null; then
        local tool_path
        tool_path=$(command -v "$tool")
        if "$tool_path" --version 2>/dev/null | grep -q "Qt version 6"; then
            echo "$tool_path"
            return 0
        fi
    fi

    # Search Qt 6.x versions (sorted descending to prefer the latest)
    for qt_version_dir in $(find "$HOME/Qt" -maxdepth 1 -name "6.*" -type d 2>/dev/null | sort -rV); do
        for platform in macos gcc_64 msvc2022_64; do
            if [ -x "$qt_version_dir/$platform/bin/$tool" ]; then
                echo "$qt_version_dir/$platform/bin/$tool"
                return 0
            fi
        done
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

_cmake_meets_min_version() {
    local cmake_bin=$1
    local required="3.27"
    local actual
    actual=$("$cmake_bin" --version 2>/dev/null | head -1 | grep -oE '[0-9]+\.[0-9]+(\.[0-9]+)?')
    [ "$(printf '%s\n' "$required" "$actual" | sort -V | head -1)" = "$required" ]
}

_use_cmake() {
    local cmake_bin=$1
    local qt_root=$2
    export PATH="$(dirname "$cmake_bin"):$PATH"
    echo "Using cmake: $cmake_bin ($("$cmake_bin" --version | head -1))"
    if [ -n "$qt_root" ] && [ -z "${CMAKE_PREFIX_PATH:-}" ]; then
        export CMAKE_PREFIX_PATH="$qt_root"
        echo "Set CMAKE_PREFIX_PATH to: $CMAKE_PREFIX_PATH"
    fi
}

ensure_cmake_in_path() {
    # Use cmake already in PATH only if it meets the minimum version requirement.
    if command -v cmake &> /dev/null && _cmake_meets_min_version "$(command -v cmake)"; then
        # Ensure CMAKE_PREFIX_PATH points to Qt 6 if not already set
        if [ -z "${CMAKE_PREFIX_PATH:-}" ]; then
            for qt_version_dir in $(find "$HOME/Qt" -maxdepth 1 -name "6.*" -type d 2>/dev/null | sort -rV); do
                for platform in macos gcc_64 msvc2022_64; do
                    if [ -d "$qt_version_dir/$platform" ]; then
                        export CMAKE_PREFIX_PATH="$qt_version_dir/$platform"
                        echo "Set CMAKE_PREFIX_PATH to: $CMAKE_PREFIX_PATH"
                        break 2
                    fi
                done
            done
        fi
        return 0
    fi

    if command -v cmake &> /dev/null; then
        echo "System cmake is too old ($(cmake --version | head -1)), searching Qt installations..."
    else
        echo "cmake not found in PATH. Searching Qt installations..."
    fi

    # Try Qt Tools CMake (standard location from Qt Online Installer)
    for cmake_candidate in \
        "$HOME/Qt/Tools/CMake/bin/cmake" \
        "$HOME/Qt/Tools/CMake/CMake.app/Contents/bin/cmake"; do

        if [ -x "$cmake_candidate" ] && _cmake_meets_min_version "$cmake_candidate"; then
            qt_root=""
            for qt_version_dir in $(find "$HOME/Qt" -maxdepth 1 -name "6.*" -type d 2>/dev/null | sort -rV); do
                for platform in macos gcc_64 msvc2022_64; do
                    if [ -d "$qt_version_dir/$platform" ]; then
                        qt_root="$qt_version_dir/$platform"
                        break 2
                    fi
                done
            done
            _use_cmake "$cmake_candidate" "$qt_root"
            return 0
        fi
    done

    # Try cmake bundled inside Qt 6 component directories (libexec preferred, then bin)
    for qt_path in \
        "$HOME"/Qt/6.*/macos/libexec \
        "$HOME"/Qt/6.*/macos/bin \
        "$HOME"/Qt/6.*/gcc_64/libexec \
        "$HOME"/Qt/6.*/gcc_64/bin \
        "$HOME"/Qt/6.*/msvc2022_64/libexec \
        "$HOME"/Qt/6.*/msvc2022_64/bin; do

        if [ -x "$qt_path/cmake" ] && _cmake_meets_min_version "$qt_path/cmake"; then
            qt_root=$(echo "$qt_path" | sed -E 's|(.*/Qt/6\.[^/]+/[^/]+).*|\1|')
            _use_cmake "$qt_path/cmake" "$qt_root"
            return 0
        fi
    done

    die "cmake >= 3.27 not found. Install a newer cmake or install Qt (which bundles cmake)."
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

