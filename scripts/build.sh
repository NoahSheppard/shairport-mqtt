#!/usr/bin/env bash
# Configure + build a CMake preset.
# Usage: scripts/build.sh [linux-debug|linux-release|windows-mingw-cross]
set -euo pipefail

PRESET="${1:-linux-debug}"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

: "${VCPKG_ROOT:?VCPKG_ROOT is not set. Run scripts/setup-env.sh first, then open a new shell.}"

cd "$ROOT_DIR"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET"
