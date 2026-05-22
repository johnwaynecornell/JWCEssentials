#!/usr/bin/env bash
set -euo pipefail

# newage_futures.sh
# Manage stateful NewAge futures (capabilities).

if [ -z "${NewAge:-}" ]; then
    echo "ERROR: \$NewAge environment variable is not set." >&2
    exit 1
fi

if [ ! -d "$NewAge" ]; then
    echo "ERROR: \$NewAge points to a missing directory: $NewAge" >&2
    exit 1
fi

# Source dev helpers if available
if [ -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
    . "$NewAge/JWCEssentials/Dev/NewAge.dev.sh"
fi

usage() {
    echo "Usage: $(basename "$0") <repo> [list|enable|disable|build] ..."
    echo "       $(basename "$0") --help"
    echo
    echo "Commands:"
    echo "  list                                 List all futures for the repo (default)"
    echo "  enable <future|all>                  Enable a future"
    echo "  disable <future|all>                 Disable a future"
    echo "  build <future|all> <mode> [args...]  Build futures"
    echo "                                       mode: native|managed|both"
}

if [ $# -lt 1 ] || [[ "$1" == "--help" ]]; then
    usage
    exit 0
fi

TARGET_REPO="$1"
shift

REPO_ROOT="$NewAge/$TARGET_REPO"
FUTURES_LST="$REPO_ROOT/Dev/futures.lst"
ENABLED_FUTURES_LST="$REPO_ROOT/Dev/enabled_futures.lst"

if [ ! -d "$REPO_ROOT" ]; then
    echo "ERROR: Repository not found in workspace: $TARGET_REPO" >&2
    exit 1
fi

COMMAND="${1:-list}"
[ $# -gt 0 ] && shift

# Helper to get available repos
get_available_repos() {
    local repos=()
    # Check directories in $NewAge
    for d in "$NewAge"/*/; do
        repos+=("$(basename "$d")")
    done
    # Also check NewAgeRepo.lst if it exists
    if [ -f "$NewAge/NewAgeRepo.lst" ]; then
        while IFS= read -r line; do
            [[ "$line" =~ ^#.* ]] && continue
            [[ -z "$line" ]] && continue
            repos+=("$(basename "$line")")
        done < "$NewAge/NewAgeRepo.lst"
    fi
    echo "${repos[@]}"
}

is_repo_available() {
    local target="$1"
    local available=($(get_available_repos))
    for r in "${available[@]}"; do
        if [ "$r" == "$target" ]; then
            return 0
        fi
    done
    return 1
}

is_future_enabled() {
    local future="$1"
    if [ -f "$ENABLED_FUTURES_LST" ]; then
        grep -Fxq "$future" "$ENABLED_FUTURES_LST"
        return $?
    fi
    return 1
}

parse_futures() {
    if [ ! -f "$FUTURES_LST" ]; then
        return
    fi
    while IFS= read -r line; do
        # Skip comments and empty lines
        [[ "$line" =~ ^#.* ]] && continue
        [[ -z "${line// /}" ]] && continue
        if [[ ! "$line" =~ \|= ]]; then continue; fi
        
        echo "$line"
    done < "$FUTURES_LST"
}

cmd_list() {
    echo "Futures for $TARGET_REPO"
    echo
    
    if [ ! -f "$FUTURES_LST" ]; then
        echo "This repository has no futures.lst"
        return
    fi

    # Keep track of declared futures to find stale enabled ones later
    declared_futures=()

    while read -r line; do
        left="${line%%|=*}"
        right="${line#*|=}"
        
        IFS='+' read -ra deps <<< "$left"
        all_met=true
        missing_deps=()
        for dep in "${deps[@]}"; do
            dep=$(echo "$dep" | xargs)
            if ! is_repo_available "$dep"; then
                all_met=false
                missing_deps+=("$dep")
            fi
        done
        
        capability_part="${right%%->*}"
        capability=$(echo "$capability_part" | xargs)
        declared_futures+=("$capability")
        
        enabled=$(is_future_enabled "$capability" && echo true || echo false)
        
        if [ "$enabled" = "true" ]; then
            if [ "$all_met" = "true" ]; then
                echo "[enabled] $capability"
            else
                echo "[stale-enabled] $capability"
                echo "  missing: ${missing_deps[*]}"
            fi
        else
            if [ "$all_met" = "true" ]; then
                echo "[available] $capability"
            else
                echo "[blocked] $capability"
                echo "  missing: ${missing_deps[*]}"
            fi
        fi
        echo "  $line"
        echo
    done < <(parse_futures)
    
    # Check for enabled futures that are NOT in futures.lst
    if [ -f "$ENABLED_FUTURES_LST" ]; then
        while read -r enabled; do
            found=false
            for declared in "${declared_futures[@]}"; do
                if [ "$enabled" == "$declared" ]; then
                    found=true
                    break
                fi
            done
            if [ "$found" = "false" ]; then
                echo "[stale-enabled] $enabled"
                echo "  (not found in futures.lst)"
                echo
            fi
        done < "$ENABLED_FUTURES_LST"
    fi
}

cmd_enable() {
    local future_to_enable="$1"
    if [ -z "$future_to_enable" ]; then
        echo "ERROR: enable requires a future name or 'all'" >&2
        exit 1
    fi
    
    if [ ! -f "$FUTURES_LST" ]; then
        echo "ERROR: No futures.lst found in $TARGET_REPO" >&2
        exit 1
    fi

    local count=0
    while read -r line; do
        left="${line%%|=*}"
        right="${line#*|=}"
        capability_part="${right%%->*}"
        capability=$(echo "$capability_part" | xargs)
        
        if [ "$future_to_enable" != "all" ] && [ "$future_to_enable" != "$capability" ]; then
            continue
        fi
        
        count=$((count + 1))
        
        # Check availability
        IFS='+' read -ra deps <<< "$left"
        all_met=true
        missing_deps=()
        for dep in "${deps[@]}"; do
            dep=$(echo "$dep" | xargs)
            if ! is_repo_available "$dep"; then
                all_met=false
                missing_deps+=("$dep")
            fi
        done
        
        if [ "$all_met" = "false" ]; then
            if [ "$future_to_enable" == "all" ]; then
                echo "$capability: blocked (missing: ${missing_deps[*]})"
                continue
            else
                echo "ERROR: Future '$capability' is blocked (missing: ${missing_deps[*]})" >&2
                exit 1
            fi
        fi
        
        if is_future_enabled "$capability"; then
            echo "$capability: already enabled"
        else
            mkdir -p "$(dirname "$ENABLED_FUTURES_LST")"
            echo "$capability" >> "$ENABLED_FUTURES_LST"
            echo "$capability: enabled"
        fi
        
    done < <(parse_futures)
    
    if [ "$count" -eq 0 ] && [ "$future_to_enable" != "all" ]; then
        echo "ERROR: Future '$future_to_enable' not found in futures.lst" >&2
        exit 1
    fi
}

cmd_disable() {
    local future_to_disable="$1"
    if [ -z "$future_to_disable" ]; then
        echo "ERROR: disable requires a future name or 'all'" >&2
        exit 1
    fi
    
    if [ ! -f "$ENABLED_FUTURES_LST" ]; then
        echo "No futures are enabled."
        return
    fi
    
    if [ "$future_to_disable" == "all" ]; then
        rm "$ENABLED_FUTURES_LST"
        echo "All futures disabled."
        return
    fi
    
    if grep -Fxq "$future_to_disable" "$ENABLED_FUTURES_LST"; then
        # Use a temporary file for sed to be safe and portable
        sed -i "/^$future_to_disable$/d" "$ENABLED_FUTURES_LST"
        
        # Determine if it was in futures.lst to decide on status message
        if parse_futures | grep -q "|= $future_to_disable "; then
            echo "$future_to_disable: disabled"
        else
            echo "$future_to_disable: removed stale enabled future"
        fi
        
        # Remove if empty
        if [ ! -s "$ENABLED_FUTURES_LST" ]; then
            rm "$ENABLED_FUTURES_LST"
        fi
    else
        echo "$future_to_disable: was not enabled"
    fi
}

cmd_build() {
    local future_to_build="$1"
    local mode="${2:-}"
    shift 2 || true # remaining args
    
    if [ -z "$future_to_build" ] || [ -z "$mode" ]; then
        echo "ERROR: build requires a future name (or 'all') and a mode (native|managed|both)" >&2
        exit 1
    fi
    
    local modes=()
    if [ "$mode" == "both" ]; then
        modes=("native" "managed")
    else
        modes=("$mode")
    fi
    
    for current_mode in "${modes[@]}"; do
        while read -r line; do
            left="${line%%|=*}"
            right="${line#*|=}"
            capability_part="${right%%->*}"
            capability=$(echo "$capability_part" | xargs)
            prefix_part="${right#*->}"
            prefix=$(echo "$prefix_part" | xargs)
            
            if [ "$future_to_build" != "all" ] && [ "$future_to_build" != "$capability" ]; then
                continue
            fi
            
            # Check availability
            IFS='+' read -ra deps <<< "$left"
            all_met=true
            missing_deps=()
            for dep in "${deps[@]}"; do
                dep=$(echo "$dep" | xargs)
                if ! is_repo_available "$dep"; then
                    all_met=false
                    missing_deps+=("$dep")
                fi
            done
            
            if [ "$future_to_build" == "all" ]; then
                # Only build enabled AND available futures
                if ! is_future_enabled "$capability" || [ "$all_met" == "false" ]; then
                    continue
                fi
            else
                # Explicitly requested future
                if [ "$all_met" == "false" ]; then
                    echo "ERROR: Future '$capability' is blocked (missing: ${missing_deps[*]})" >&2
                    exit 1
                fi
                if ! is_future_enabled "$capability"; then
                    echo "[explicit] building available but not enabled future: $capability"
                fi
            fi
            
            script="${prefix}_${current_mode}.sh"
            full_script_path="$REPO_ROOT/$script"
            
            if [ -f "$full_script_path" ]; then
                echo "Building future: $capability ($current_mode)"
                (cd "$REPO_ROOT" && bash "./$script" "$@") || {
                    echo "ERROR: Build failed for future: $capability ($current_mode)" >&2
                    exit 1
                }
            else
                if [ "$future_to_build" == "all" ]; then
                    echo "$capability: has no $script"
                else
                    echo "ERROR: Missing build script for $capability: $script" >&2
                    exit 1
                fi
            fi
            
        done < <(parse_futures)
    done
}

case "$COMMAND" in
    list) cmd_list ;;
    enable) cmd_enable "$@" ;;
    disable) cmd_disable "$@" ;;
    build) cmd_build "$@" ;;
    *) echo "Unknown command: $COMMAND"; usage; exit 1 ;;
esac
