#!/usr/bin/env bash
# Build (if needed) and run a preset's executable.
# Usage: scripts/run.sh [linux-debug|linux-release|windows-mingw-cross]
set -euo pipefail

PRESET="${1:-linux-debug}"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT_DIR/build/$PRESET/shairport-mqtt"

if [[ "$PRESET" == windows-* ]]; then
    BIN="${BIN}.exe"
fi

if [[ ! -x "$BIN" ]]; then
    "$ROOT_DIR/scripts/build.sh" "$PRESET"
fi

if [[ "$PRESET" == windows-* ]]; then
    if command -v wine >/dev/null 2>&1; then
        exec wine "$BIN"
    else
        echo "Cross-compiled for Windows. Copy $BIN to a Windows machine to run it" >&2
        echo "(or install 'wine' to run it locally for a quick sanity check)." >&2
        exit 1
    fi
fi

exec "$BIN"
