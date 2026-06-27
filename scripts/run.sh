#!/usr/bin/env bash
# Build (if needed) and run a preset's executable.
# Usage: scripts/run.sh [linux-debug|linux-release|windows-mingw-cross] [shairport-mqtt|shairport-mqtt-gui]
set -euo pipefail

PRESET="${1:-linux-debug}"
TARGET="${2:-shairport-mqtt}"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT_DIR/build/$PRESET/$TARGET"

if [[ "$PRESET" == windows-* ]]; then
    BIN="${BIN}.exe"
fi

# Always re-run the build (ninja no-ops if nothing changed) so this never
# silently runs a stale binary after a source edit.
"$ROOT_DIR/scripts/build.sh" "$PRESET"

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
