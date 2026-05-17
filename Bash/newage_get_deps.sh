#!/usr/bin/env bash
set -euo pipefail

# newage_get_deps.sh
# Acquire dependencies using the sorter and newage_get.sh.

usage() {
    cat <<EOF
Usage:
  newage_get_deps.sh [REPO_LIST_FILE]

Default REPO_LIST_FILE is \$NewAge/NewAgeRepo.lst
EOF
}

if [ -z "${NewAge:-}" ]; then
    echo "[newage_get_deps] ERROR: NewAge environment variable is not set." >&2
    exit 1
fi

# Source NewAge helpers if available
if [ -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
    . "$NewAge/JWCEssentials/Dev/NewAge.dev.sh"
fi

REPO_LIST_FILE="${1:-$NewAge/NewAgeRepo.lst}"

# Determine where newage_dep_sort.sh and newage_get.sh are.
# They should be in the same directory as this script.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEP_SORT="$SCRIPT_DIR/newage_dep_sort.sh"
GET_REPO="$SCRIPT_DIR/newage_get.sh"

if [ ! -f "$DEP_SORT" ]; then
    echo "[newage_get_deps] ERROR: Required script not found: $DEP_SORT" >&2
    exit 1
fi
if [ ! -f "$GET_REPO" ]; then
    echo "[newage_get_deps] ERROR: Required script not found: $GET_REPO" >&2
    exit 1
fi

echo "[newage_get_deps] Sorting dependencies..."
RECORDS=$("$DEP_SORT" "$REPO_LIST_FILE")

while IFS= read -r record || [ -n "$record" ]; do
    [ -z "$record" ] && continue
    
    REPO_NAME=$(echo "$record" | cut -d'|' -f1)
    GIT_URL=$(echo "$record" | cut -d'|' -s -f2)
    
    if [ -n "$GIT_URL" ]; then
        echo "[newage_get_deps] Processing $REPO_NAME..."
        bash "$GET_REPO" "$record" || exit 1
    else
        echo "[newage_get_deps] Skipping $REPO_NAME (no GitUrl provided in record)"
    fi
done <<< "$RECORDS"

echo "[newage_get_deps] Complete."
