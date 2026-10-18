#!/usr/bin/env bash
set -eu

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
WINDOWS_REPO_DIR="$(wslpath -w "$REPO_DIR")"

powershell.exe -NoProfile -Command \
  "& python (Join-Path '${WINDOWS_REPO_DIR//\'/\'\'}' 'tools\claw_splash.py')"
