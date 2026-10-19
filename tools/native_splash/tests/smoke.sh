#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
make
out=$(mktemp)
trap 'rm -f "$out"' EXIT HUP INT TERM
./claw-splash --once --cols 100 --rows 45 --color none > "$out"
[ -s "$out" ]
awk 'length($0) > 100 { bad = 1 } END { if (NR == 0 || bad) exit 1 }' "$out"
printf '%s\n' 'CLAW native splash smoke test passed.'
