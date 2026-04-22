#!/usr/bin/env bash
# Build script for Scrite (all platforms).
# Usage: ./build.sh [--clean] [--type=<version-type>]
#
# Options:
#   --clean              Remove build directory and rebuild from scratch
#   --type=<type>        Set SCRITE_VERSION_TYPE (e.g. "beta", "rc", "dev")
#                        Default: empty string (release)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-build}"
VERSION_TYPE=""

# shellcheck source=/dev/null
. "${SCRIPT_DIR}/_common.sh"

# Auto-detect cmake from Qt if not in PATH
ensure_cmake_in_path

# Parse arguments
while [ $# -gt 0 ]; do
    case "$1" in
        --clean)
            echo "Removing build directory..."
            rm -rf "${PROJECT_ROOT}/${BUILD_DIR}"
            ;;
        --type=*)
            VERSION_TYPE="${1#--type=}"
            ;;
        *)
            die "Unknown option: $1"
            ;;
    esac
    shift
done

echo "Configuring CMake..."

# On macOS, set minimum deployment target to macOS 12 (Qt 6.8 minimum)
CMAKE_ARGS=("-DCMAKE_BUILD_TYPE=Release")

# Set version type if provided
if [ -n "$VERSION_TYPE" ]; then
    CMAKE_ARGS+=("-DSCRITE_VERSION_TYPE=$VERSION_TYPE")
    echo "  Version type: $VERSION_TYPE"
fi

if [ "$(uname)" = "Darwin" ]; then
    MACOS_MIN_VERSION="${MACOS_DEPLOYMENT_TARGET:-}"

    # If not explicitly set, detect from Qt frameworks
    if [ -z "$MACOS_MIN_VERSION" ]; then
        # Collect all Qt frameworks and their versions, use the maximum (most restrictive)
        MAX_QT_VERSION=""
        while IFS= read -r QT_FRAMEWORK_PATH; do
            if [ -f "$QT_FRAMEWORK_PATH/QtCore" ]; then
                # Try LC_BUILD_VERSION first (newer format)
                QT_MIN_VERSION=$(otool -l "$QT_FRAMEWORK_PATH/QtCore" 2>/dev/null | grep -A3 "LC_BUILD_VERSION" | grep "minos" | head -1 | awk '{print $2}' | sed 's/\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)/\1.\2/')

                # Fallback to LC_VERSION_MIN_MACOSX (older format)
                if [ -z "$QT_MIN_VERSION" ]; then
                    QT_MIN_VERSION=$(otool -l "$QT_FRAMEWORK_PATH/QtCore" 2>/dev/null | grep -A2 "LC_VERSION_MIN_MACOSX" | grep "version" | head -1 | awk '{print $2}' | sed 's/\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)/\1.\2/')
                fi

                if [ -n "$QT_MIN_VERSION" ]; then
                    # Keep the maximum version (most restrictive for compatibility)
                    if [ -z "$MAX_QT_VERSION" ] || [ "$(printf '%s\n' "$MAX_QT_VERSION" "$QT_MIN_VERSION" | sort -rV | head -1)" = "$QT_MIN_VERSION" ]; then
                        MAX_QT_VERSION="$QT_MIN_VERSION"
                    fi
                fi
            fi
        done < <(find "$HOME/Qt" -name "QtCore.framework" -type d 2>/dev/null | grep -v "Qt Creator.app")

        if [ -n "$MAX_QT_VERSION" ]; then
            MACOS_MIN_VERSION="$MAX_QT_VERSION"
            echo "  Detected Qt framework minimum macOS version: ${MACOS_MIN_VERSION}"
        fi
    fi

    # Fallback to Qt 6.8 minimum if detection fails
    if [ -z "$MACOS_MIN_VERSION" ]; then
        MACOS_MIN_VERSION="12"
    fi

    CMAKE_ARGS+=("-DCMAKE_OSX_DEPLOYMENT_TARGET=${MACOS_MIN_VERSION}")
    echo "  macOS minimum deployment target: ${MACOS_MIN_VERSION}"
fi

cmake -B "${PROJECT_ROOT}/${BUILD_DIR}" -S "${PROJECT_ROOT}" "${CMAKE_ARGS[@]}"

echo "Building Scrite..."

# Use a reasonable number of parallel jobs (default: 4, or set via PARALLEL_JOBS env var)
PARALLEL_JOBS="${PARALLEL_JOBS:-4}"
echo "Using ${PARALLEL_JOBS} parallel jobs (set PARALLEL_JOBS to override)"

cmake --build "${PROJECT_ROOT}/${BUILD_DIR}" --config Release --parallel "$PARALLEL_JOBS"

echo ""
echo "Build complete: ${PROJECT_ROOT}/${BUILD_DIR}"
