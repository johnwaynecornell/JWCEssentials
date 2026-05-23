#!/usr/bin/env bash
set -euo pipefail

# newage_get.sh
# Acquire or update one repository into the active $NewAge workspace.

usage() {
    cat <<EOF
Usage:
  newage_get.sh 'RepoName|GitUrl'
  newage_get.sh 'RepoName|GitUrl|Branch'
  newage_get.sh RepoName GitUrl
  newage_get.sh RepoName GitUrl Branch

Examples:
  newage_get.sh JWCEssentials https://github.com/johnwaynecornell/JWCEssentials.git
  newage_get.sh 'JWCEssentials|https://github.com/johnwaynecornell/JWCEssentials.git|main'

Note:
  Requires \$NewAge environment variable to be set.
EOF
}

if [ -z "${NewAge:-}" ]; then
    echo "[newage_get] ERROR: NewAge environment variable is not set." >&2
    exit 1
fi

# Source NewAge helpers if available
if [ -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
    . "$NewAge/JWCEssentials/Dev/NewAge.dev.sh"
fi

# Parse arguments
REPO_NAME=""
GIT_URL=""
BRANCH="main"

if [ "$#" -eq 1 ]; then
    IFS='|' read -r REPO_NAME GIT_URL BRANCH_ARG <<< "$1"
    if [ -n "$BRANCH_ARG" ]; then
        BRANCH="$BRANCH_ARG"
    fi
elif [ "$#" -eq 2 ]; then
    REPO_NAME="$1"
    GIT_URL="$2"
elif [ "$#" -eq 3 ]; then
    REPO_NAME="$1"
    GIT_URL="$2"
    BRANCH="$3"
else
    usage
    exit 1
fi

if [ -z "$REPO_NAME" ] || [ -z "$GIT_URL" ]; then
    echo "[newage_get] ERROR: RepoName and GitUrl are required." >&2
    usage
    exit 1
fi

DESTINATION="$NewAge/$REPO_NAME"

echo "[newage_get] Target: $REPO_NAME"
echo "[newage_get] URL:    $GIT_URL"
echo "[newage_get] Branch: $BRANCH"
echo "[newage_get] Dest:   $DESTINATION"

if [ ! -e "$DESTINATION" ]; then
    echo "[newage_get] Cloning..."
    git clone "$GIT_URL" "$DESTINATION"
    echo "[newage_get] Switching to branch $BRANCH..."
    git -C "$DESTINATION" switch "$BRANCH" || git -C "$DESTINATION" checkout -b "$BRANCH" "origin/$BRANCH" 2>/dev/null || git -C "$DESTINATION" checkout "$BRANCH"
    echo "$REPO_NAME" >> "$NewAge/NewAgeRepo.lst"
    newage_add_repo_entry "$REPO_NAME"
elif [ -d "$DESTINATION/.git" ]; then
    echo "[newage_get] Destination exists and is a git repo. Updating..."
    git -C "$DESTINATION" fetch origin
    echo "[newage_get] Switching to branch $BRANCH..."
    git -C "$DESTINATION" switch "$BRANCH"
    echo "[newage_get] Pulling latest..."
    git -C "$DESTINATION" pull --ff-only origin "$BRANCH"
else
    echo "[newage_get] ERROR: Destination exists but is not a git repository: $DESTINATION" >&2
    exit 1
fi

echo "[newage_get] Success: $REPO_NAME is ready."
