#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

python3 check_readme.py

for source in [0-9][0-9][0-9]_*.asm; do
    output="${source%.asm}.sg"
    echo "Building $output"
    sjasmplus --raw="$output" "$source"
done

