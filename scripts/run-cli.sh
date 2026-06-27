#!/usr/bin/env bash
# Build (if needed) and run the CLI.
# Usage: scripts/run-cli.sh [linux-debug|linux-release|windows-mingw-cross]
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "$ROOT_DIR/scripts/run.sh" "${1:-linux-debug}" shairport-mqtt
