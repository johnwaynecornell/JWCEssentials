#!/usr/bin/env bash
#
# newage_expand.sh
#
# Expand conditional capabilities (futures) based on repository composition.

set -euo pipefail

if [ -z "${NewAge:-}" ]; then
    echo "ERROR: \$NewAge environment variable is not set." >&2
    exit 1
fi

if [ ! -d "$NewAge" ]; then
    echo "ERROR: \$NewAge points to a missing directory: $NewAge" >&2
    exit 1
fi

# Source dev helpers if available in the default location
if [ -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
    . "$NewAge/JWCEssentials/Dev/NewAge.dev.sh"
fi

usage() {
    echo "Usage: $(basename "$0") <RepoName> [--dry-run | --apply]"
    echo
    echo "Options:"
    echo "  --dry-run    (Default) Show what would be done."
    echo "  --apply      Run the expansion scripts."
}

if [ $# -lt 1 ]; then
    usage
    exit 1
fi

TARGET_REPO="$1"
shift

MODE="dry-run"
while [ $# -gt 0 ]; do
    case "$1" in
        --dry-run) MODE="dry-run" ;;
        --apply) MODE="apply" ;;
        *) echo "Unknown option: $1"; usage; exit 1 ;;
    esac
    shift
done

REPO_ROOT="$NewAge/$TARGET_REPO"
if [ ! -d "$REPO_ROOT" ]; then
    echo "ERROR: Repository not found in workspace: $TARGET_REPO" >&2
    exit 1
fi

FUTURES_LST="$REPO_ROOT/Dev/futures.lst"
if [ ! -f "$FUTURES_LST" ]; then
    # Missing futures.lst is not an error
    exit 0
fi

# Determine available repos in the workspace
available_repos=()
# Check directories in $NewAge
for d in "$NewAge"/*/; do
    repo=$(basename "$d")
    available_repos+=("$repo")
done

# Also check NewAgeRepo.lst if it exists
if [ -f "$NewAge/NewAgeRepo.lst" ]; then
    while IFS= read -r line; do
        [[ "$line" =~ ^#.* ]] && continue
        [[ -z "$line" ]] && continue
        repo=$(basename "$line")
        available_repos+=("$repo")
    done < "$NewAge/NewAgeRepo.lst"
fi

is_repo_available() {
    local target="$1"
    for r in "${available_repos[@]}"; do
        if [ "$r" == "$target" ]; then
            return 0
        fi
    done
    return 1
}

echo "--- NewAge Expansion: $TARGET_REPO ($MODE) ---"

# Parse futures.lst
# Syntax: RepoA + RepoB |= capability -> prefix
while IFS= read -r line; do
    # Skip comments and empty lines
    [[ "$line" =~ ^#.* ]] && continue
    [[ -z "${line// /}" ]] && continue

    if [[ ! "$line" =~ \|= ]]; then
        continue
    fi

    # Split by |=
    left="${line%%|=*}"
    right="${line#*|=}"

    # Parse left side (dependencies)
    IFS='+' read -ra deps <<< "$left"
    all_met=true
    missing_deps=()
    for dep in "${deps[@]}"; do
        dep=$(echo "$dep" | xargs) # trim
        if ! is_repo_available "$dep"; then
            all_met=false
            missing_deps+=("$dep")
        fi
    done

    # Parse right side (capability -> prefix)
    capability_part="${right%%->*}"
    prefix_part="${right#*->}"
    
    capability=$(echo "$capability_part" | xargs)
    prefix=$(echo "$prefix_part" | xargs)

    if [ "$all_met" = true ]; then
        echo "Capability enabled: $capability (all dependencies met)"
        
        suffixes=("_native.sh" "_managed.sh" "_coordinated.sh")
        for suffix in "${suffixes[@]}"; do
            script_path="$prefix$suffix"
            full_script_path="$REPO_ROOT/$script_path"
            
            if [ "$MODE" == "dry-run" ]; then
                if [ -f "$full_script_path" ]; then
                    echo "  [MATCH] $script_path"
                else
                    # Optional scripts don't warn in dry-run unless we want them to
                    :
                fi
            elif [ "$MODE" == "apply" ]; then
                if [ -f "$full_script_path" ]; then
                    echo "  [EXEC] $script_path"
                    (cd "$REPO_ROOT" && bash "./$script_path")
                else
                    # Missing expansion scripts warn unless explicitly requested.
                    # Currently we don't have a mechanism for explicit capability request,
                    # so we just warn.
                    echo "  [WARN] Missing expansion script: $script_path" >&2
                fi
            fi
        done
    else
        if [ "$MODE" == "dry-run" ]; then
            echo "Capability disabled: $capability (missing: ${missing_deps[*]})"
        fi
    fi

done < "$FUTURES_LST"
