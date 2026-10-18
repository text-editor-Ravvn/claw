#!/usr/bin/env bash
set -eu

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
WINDOWS_REPO_DIR="$(wslpath -w "$REPO_DIR")"

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
