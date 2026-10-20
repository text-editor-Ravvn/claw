#!/usr/bin/env bash
set -eu

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_DIR"

SPLASH_SCRIPT="$REPO_DIR/tools/claw_splash.py"
SPLASH_IMG="$REPO_DIR/assets/claw-splash.png"

if [ ! -f "$SPLASH_IMG" ]; then
    echo "CLAW splash image not found: $SPLASH_IMG" >&2
    exit 1
fi

# 1. WSL (Windows Subsystem for Linux): launch Python in Windows to display the native window
if [ -n "${WSL_DISTRO_NAME:-}" ]; then
    if command -v powershell.exe >/dev/null 2>&1; then
        WINDOWS_REPO_DIR="$(wslpath -w "$REPO_DIR")" || exit 1

        powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command \
          "& { \$py = (Get-Command py, python -All -ErrorAction SilentlyContinue | Where-Object { \$_.Source -notmatch 'WindowsApps' } | Select-Object -First 1); if (\$py) { if (\$py.Name -like 'py*') { & \$py.Source -3 '${WINDOWS_REPO_DIR}\\tools\\claw_splash.py' } else { & \$py.Source '${WINDOWS_REPO_DIR}\\tools\\claw_splash.py' } } else { & '${WINDOWS_REPO_DIR}\\tools\\launch_splash.ps1' } }"
        
        if [ $? -eq 0 ]; then
            exit 0
        fi

        powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "${WINDOWS_REPO_DIR}\\tools\\launch_splash.ps1"
        exit $?
    fi
fi

# 2. Windows native environments (Git Bash, MSYS2, MinGW, Cygwin)
OS_NAME="$(uname -s 2>/dev/null || echo '')"
case "$OS_NAME" in
    CYGWIN*|MINGW*|MSYS*)
        for py_bin in "py" "python" "python3"; do
            if command -v "$py_bin" >/dev/null 2>&1; then
                if [ "$py_bin" = "py" ]; then
                    if py -3 "$SPLASH_SCRIPT" 2>/dev/null; then exit 0; fi
                else
                    if "$py_bin" "$SPLASH_SCRIPT" 2>/dev/null; then exit 0; fi
                fi
            fi
        done
        if command -v powershell.exe >/dev/null 2>&1; then
            powershell.exe -NoProfile -NonInteractive -Command "& python '$SPLASH_SCRIPT'; if (\$LASTEXITCODE -ne 0) { & py -3 '$SPLASH_SCRIPT' }" 2>/dev/null && exit 0
        fi
        ;;
esac

# 3. Native Linux/macOS with a graphical display (X11, Wayland, or Quartz)
IS_MACOS=0
if [ "$OS_NAME" = "Darwin" ]; then
    IS_MACOS=1
fi

if [ -n "${DISPLAY:-}" ] || [ -n "${WAYLAND_DISPLAY:-}" ] || [ "$IS_MACOS" -eq 1 ]; then
    for py_bin in python3 python; do
        if command -v "$py_bin" >/dev/null 2>&1; then
            if "$py_bin" -c 'import tkinter' >/dev/null 2>&1; then
                if "$py_bin" "$SPLASH_SCRIPT"; then
                    exit 0
                fi
            fi
        fi
    done
fi

# Fallback: graphical splash could not be displayed
echo "Graphical splash unavailable; falling back to terminal splash." >&2
exit 1
