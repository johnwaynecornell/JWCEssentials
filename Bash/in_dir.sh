#!/usr/bin/env bash
set -euo pipefail
cd "$1" || { echo "[in_dir] ERROR: Could not cd into $1" >&2; exit 1; }
shift

# Ensure a command was actually passed after the directory
if [ $# -eq 0 ]; then
    echo "[in_dir] ERROR: No command provided to execute" >&2
    exit 1
fi

exec "$@"
