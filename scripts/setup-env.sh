#!/usr/bin/env bash
# One-time machine setup: vcpkg + (optionally) the mingw-w64 cross-compiler.
set -euo pipefail

VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"

if [[ ! -d "$VCPKG_ROOT" ]]; then
    echo "Cloning vcpkg into $VCPKG_ROOT..."
    git clone --depth 1 https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
fi

if [[ ! -x "$VCPKG_ROOT/vcpkg" ]]; then
    "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
fi

echo "vcpkg ready at $VCPKG_ROOT"

if ! grep -q "VCPKG_ROOT" "$HOME/.bashrc" 2>/dev/null; then
    {
        echo ""
        echo "export VCPKG_ROOT=\"$VCPKG_ROOT\""
        echo "export PATH=\"\$VCPKG_ROOT:\$PATH\""
    } >> "$HOME/.bashrc"
    echo "Added VCPKG_ROOT to ~/.bashrc. Open a new shell or 'source ~/.bashrc' to pick it up."
else
    echo "VCPKG_ROOT already configured in ~/.bashrc."
fi

if ! command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
    echo
    echo "mingw-w64 not found (needed for the windows-mingw-cross preset). Install it with:"
    echo "  sudo apt-get update && sudo apt-get install -y mingw-w64 mingw-w64-tools"
fi
