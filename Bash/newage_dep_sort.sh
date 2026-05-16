#!/usr/bin/env bash
set -euo pipefail

# newage_dep_sort.sh
# Produce dependency-first repository records.

usage() {
    cat <<EOF
Usage:
  newage_dep_sort.sh [REPO_LIST_FILE]

Default REPO_LIST_FILE is \$NewAge/NewAgeRepo.lst
EOF
}

if [ -z "${NewAge:-}" ]; then
    echo "[newage_dep_sort] ERROR: NewAge environment variable is not set." >&2
    exit 1
fi

REPO_LIST_FILE="${1:-$NewAge/NewAgeRepo.lst}"

if [ ! -f "$REPO_LIST_FILE" ]; then
    echo "[newage_dep_sort] ERROR: Repo list file not found: $REPO_LIST_FILE" >&2
    exit 1
fi

declare -A VISITED
declare -A PROCESSING
declare -A REPO_TO_RECORD

# Function to parse a record and return RepoName
get_repo_name() {
    echo "$1" | cut -d'|' -f1
}

# Function to ensure a record has the default branch if it has a GitUrl
canonicalize_record() {
    local record="$1"
    local name
    local url
    local branch
    
    name=$(echo "$record" | cut -d'|' -s -f1)
    if [ -z "$name" ]; then
        # Not a pipe-separated record
        echo "$record"
        return
    fi
    
    url=$(echo "$record" | cut -d'|' -s -f2)
    branch=$(echo "$record" | cut -d'|' -s -f3)
    
    if [ -n "$url" ] && [ -z "$branch" ]; then
        echo "$name|$url|main"
    else
        echo "$record"
    fi
}

process_repo() {
    local record="$1"
    local repo
    repo=$(get_repo_name "$record")
    
    if [[ -n "${VISITED[$repo]:-}" ]]; then
        return
    fi
    
    if [[ -n "${PROCESSING[$repo]:-}" ]]; then
        echo "[newage_dep_sort] ERROR: Circular dependency detected involving $repo" >&2
        exit 1
    fi
    
    PROCESSING[$repo]=1
    
    # Check for dependencies
    local deps_file="$NewAge/$repo/Dev/NewAgeDeps.lst"
    if [ -f "$deps_file" ]; then
        while IFS= read -r line || [ -n "$line" ]; do
            # Ignore comments and blank lines
            [[ "$line" =~ ^[[:space:]]*# ]] && continue
            [[ -z "${line//[[:space:]]/}" ]] && continue
            
            local dep_record
            dep_record=$(canonicalize_record "$line")
            local dep_name
            dep_name=$(get_repo_name "$dep_record")
            
            # Store the most detailed record we've seen for this repo
            if [[ -z "${REPO_TO_RECORD[$dep_name]:-}" ]] || [[ "$dep_record" == *'|'* ]]; then
                REPO_TO_RECORD[$dep_name]="$dep_record"
            fi
            
            process_repo "$dep_record"
        done < "$deps_file"
    fi
    
    # Output the record
    local final_record="${REPO_TO_RECORD[$repo]:-$record}"
    echo "$final_record"
    
    unset PROCESSING[$repo]
    VISITED[$repo]=1
}

# Read initial repos from the list file
while IFS= read -r line || [ -n "$line" ]; do
    [[ "$line" =~ ^[[:space:]]*# ]] && continue
    [[ -z "${line//[[:space:]]/}" ]] && continue
    
    record=$(canonicalize_record "$line")
    name=$(get_repo_name "$record")
    
    if [[ -z "${REPO_TO_RECORD[$name]:-}" ]] || [[ "$record" == *'|'* ]]; then
        REPO_TO_RECORD[$name]="$record"
    fi
done < "$REPO_LIST_FILE"

# Process all repos found in the list file
# We need to re-read or use an array because we want to maintain the order from the file where possible,
# but dependencies must come first.
while IFS= read -r line || [ -n "$line" ]; do
    [[ "$line" =~ ^[[:space:]]*# ]] && continue
    [[ -z "${line//[[:space:]]/}" ]] && continue
    
    record=$(canonicalize_record "$line")
    process_repo "$record"
done < "$REPO_LIST_FILE"
