#!/usr/bin/env bash
set -euo pipefail

# newage_all_configure.sh
# Configure all known workspace repos in dependency-first order.

usage() {
    cat <<EOF
Usage:
  newage_all_configure.sh [REPO_LIST_FILE]

Default REPO_LIST_FILE is \$NewAge/NewAgeRepo.lst
EOF
}

if [ -z "${NewAge:-}" ]; then
    echo "[newage_all_configure] ERROR: NewAge environment variable is not set." >&2
    exit 1
fi

REPO_LIST_FILE="${1:-$NewAge/NewAgeRepo.lst}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEP_SORT="$SCRIPT_DIR/newage_dep_sort.sh"

if [ ! -f "$DEP_SORT" ]; then
    echo "[newage_all_configure] ERROR: Required script not found: $DEP_SORT" >&2
    exit 1
fi

echo "[newage_all_configure] Sorting repositories..."
RECORDS=$("$DEP_SORT" "$REPO_LIST_FILE")

while IFS= read -r record || [ -n "$record" ]; do
    [ -z "$record" ] && continue
    
    REPO_NAME=$(echo "$record" | cut -d'|' -f1)
    REPO_DIR="$NewAge/$REPO_NAME"
    CONFIG_SCRIPT="$REPO_DIR/configure.sh"
    
    if [ -f "$CONFIG_SCRIPT" ]; then
        echo "[newage_all_configure] Configuring $REPO_NAME..."
        if [ -x "$CONFIG_SCRIPT" ]; then
            "$CONFIG_SCRIPT" --newage "$NewAge" || exit 1
        else
            echo "[newage_all_configure] WARNING: $CONFIG_SCRIPT is not executable. Running via bash."
            bash "$CONFIG_SCRIPT" --newage "$NewAge" || exit 1
        fi
    else
        echo "[newage_all_configure] Skipping $REPO_NAME (no configure.sh found)"
    fi
done <<< "$RECORDS"

echo "[newage_all_configure] Complete."
