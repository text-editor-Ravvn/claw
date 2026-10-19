#!/usr/bin/env bash
set -eu

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_DIR"

# Windows running CLAW through WSL: preserve the existing Windows splash.
if [ -n "${WSL_DISTRO_NAME:-}" ]; then
    if ! command -v powershell.exe >/dev/null 2>&1; then
        echo "Windows PowerShell unavailable; using terminal splash."
        exit 1
    fi

    WINDOWS_REPO_DIR="$(wslpath -w "$REPO_DIR")" || exit 1

    powershell.exe -NoProfile -Command \
      '& {
        param([string]$repo)
        $ErrorActionPreference = "Stop"
        $script = Join-Path $repo "tools\claw_splash.py"

        if (-not (Test-Path -LiteralPath $script)) {
            Write-Error "CLAW splash script not found: $script"
            exit 1
        }

        $python = Get-Command python -ErrorAction SilentlyContinue
        if ($python) {
            & $python.Source $script
            exit $LASTEXITCODE
        }

        $launcher = Get-Command py -ErrorAction SilentlyContinue
        if ($launcher) {
            & $launcher.Source -3 $script
            exit $LASTEXITCODE
        }

        Write-Error "Windows Python was not found."
        exit 1
      }' "$WINDOWS_REPO_DIR"

    exit $?
fi


# Native Linux/macOS: prefer the native terminal splash.
NATIVE_SPLASH="$REPO_DIR/tools/native_splash/claw-splash"

if [ -x "$NATIVE_SPLASH" ]; then
    if "$NATIVE_SPLASH"; then
        exit 0
    fi
    echo "Native splash failed; trying graphical splash."
fi

# Fallback: use the original PNG through Python/Tkinter.
if command -v python3 >/dev/null 2>&1; then
    python3 -c 'import tkinter' >/dev/null 2>&1 || {
        echo "Tkinter unavailable; using terminal splash."
        exit 1
    }

    python3 "$REPO_DIR/tools/claw_splash.py"
    exit $?
fi

echo "Python 3 unavailable; using terminal splash."
exit 1
