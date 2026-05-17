#!/usr/bin/env bash
set -euo pipefail

# newage_repo_list.sh
# List all repositories in dependency order (relative paths from $NewAge).

if [ -z "${NewAge:-}" ]; then
    echo "[newage_repo_list] ERROR: NewAge environment variable is not set." >&2
    exit 1
fi

usage() {
    cat <<EOF
Usage:
  newage_repo_list.sh [REPO_LIST_FILE]

Example:
  newage_repo_list.sh | xargs -I{} in_dir.sh "{}" git pull
EOF
}

# Handle --help
if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
    usage
    exit 0
fi

# Locate the dependency sorter
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEP_SORT="$SCRIPT_DIR/newage_dep_sort.sh"

if [ ! -f "$DEP_SORT" ]; then
    echo "[newage_repo_list] ERROR: Required script not found: $DEP_SORT" >&2
    exit 1
fi

# If stdin is a terminal, run the sorter with provided arguments.
# Otherwise, read from stdin (piped input).
if [ -t 0 ]; then
    "$DEP_SORT" "$@" | cut -d'|' -f1
else
    cut -d'|' -f1
fi
