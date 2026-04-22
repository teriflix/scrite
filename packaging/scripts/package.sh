#!/usr/bin/env bash
# Platform dispatcher for Scrite packaging.
# Detects the current platform and invokes the appropriate platform-specific script.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

case "$(uname -s)" in
    Darwin)
        exec "${SCRIPT_DIR}/package-macos.sh" "$@"
        ;;
    MINGW* | MSYS* | CYGWIN*)
        exec "${SCRIPT_DIR}/package-windows.sh" "$@"
        ;;
    Linux)
        exec "${SCRIPT_DIR}/package-linux.sh" "$@"
        ;;
    *)
        echo "ERROR: Unsupported platform: $(uname -s)" >&2
        exit 1
        ;;
esac
