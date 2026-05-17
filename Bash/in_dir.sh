#!/usr/bin/env bash
set -euo pipefail
cd "$1" || { echo "[in_dir] ERROR: Could not cd into $1" >&2; exit 1; }
shift
exec "$@"
