#!/usr/bin/env bash
set -euo pipefail

# newage_all_build_coordinated.sh
# Run repo-level coordinated build front doors for all repos in dependency order.

usage() {
    cat <<EOF
Usage:
  newage_all_build_coordinated.sh [BuildArgs...]

Examples:
  newage_all_build_coordinated.sh Debug
  newage_all_build_coordinated.sh Release --fresh
EOF
}

if [ -z "${NewAge:-}" ]; then
    echo "[newage_all_build_coordinated] ERROR: NewAge environment variable is not set." >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEP_SORT="$SCRIPT_DIR/newage_dep_sort.sh"

if [ ! -f "$DEP_SORT" ]; then
    echo "[newage_all_build_coordinated] ERROR: Required script not found: $DEP_SORT" >&2
    exit 1
fi

REPO_LIST_FILE="$NewAge/NewAgeRepo.lst"

echo "[newage_all_build_coordinated] Sorting repositories..."
RECORDS=$("$DEP_SORT" "$REPO_LIST_FILE")

while IFS= read -r record || [ -n "$record" ]; do
    [ -z "$record" ] && continue
    
    REPO_NAME=$(echo "$record" | cut -d'|' -f1)
    REPO_DIR="$NewAge/$REPO_NAME"
    BUILD_SCRIPT="$NewAge/bin/newage_build_coordinated.sh"
    
    if [ -f "$BUILD_SCRIPT" ]; then
        echo "[newage_all_build_coordinated] Building $REPO_NAME..."
        bash "$BUILD_SCRIPT" "$REPO_NAME" "$@"
    else
        echo "[newage_all_build_coordinated] Skipping $REPO_NAME (no Bash/newage_build_coordinated.sh found)"
    fi
done <<< "$RECORDS"

echo "[newage_all_build_coordinated] Complete."
