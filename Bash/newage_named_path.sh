#!/usr/bin/env bash

# newage_named_path.sh
# Prints full paths for lib, bin, or build directories based on the current NewAge lane.

if [ -z "${NewAge:-}" ]; then
    echo "[newage_named_path] ERROR: NewAge environment variable is not set." >&2
    exit 1
fi

TYPE="${1:-}"

case "$TYPE" in
    lib|bin|build)
        ;;
    *)
        echo "Usage: $0 {lib|bin|build} [repo_path]" >&2
        echo "  [repo_path] defaults to current directory for 'build'." >&2
        exit 1
        ;;
esac

# Ensure Config and Lane are set
if [ -z "${NewAge_Config:-}" ] || [ -z "${NewAge_Lane:-}" ]; then
    # Try to find NewAge.dev.sh to resolve them
    DEV_SH=""
    if [ -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
        DEV_SH="$NewAge/JWCEssentials/Dev/NewAge.dev.sh"
    elif [ -f "Dev/NewAge.dev.sh" ]; then
        DEV_SH="Dev/NewAge.dev.sh"
    fi

    if [ -n "$DEV_SH" ]; then
        . "$DEV_SH" >/dev/null 2>&1 || true
        
        export NewAge_Config="${NewAge_Config:-Debug}"
        if [ -z "${NewAge_Lane:-}" ]; then
            if command -v newage_resolve_platform_lane >/dev/null 2>&1; then
                export NewAge_Lane="$(newage_resolve_platform_lane)"
            fi
        fi
    fi
fi

# Final check and normalization
if [ -z "${NewAge_Config:-}" ] || [ -z "${NewAge_Lane:-}" ]; then
    echo "[newage_named_path] ERROR: NewAge_Config and NewAge_Lane must be set or detectable." >&2
    exit 1
fi

# Normalize NewAge_Lane (replace \ with /)
NewAge_Lane="${NewAge_Lane//\\//}"

case "$TYPE" in
    lib)
        echo "$NewAge/lib/$NewAge_Config/$NewAge_Lane"
        ;;
    bin)
        echo "$NewAge/bin/$NewAge_Config/$NewAge_Lane"
        ;;
    build)
        REPO="${2:-.}"
        # If REPO is not a directory, try $NewAge/$REPO
        if [ ! -d "$REPO" ] && [ -d "$NewAge/$REPO" ]; then
            REPO="$NewAge/$REPO"
        fi

        # Try to get absolute path for source directory
        if [ -d "$REPO" ]; then
            ABS_REPO="$(cd "$REPO" && pwd -P)"
            echo "$ABS_REPO/build/$NewAge_Config/$NewAge_Lane"
        else
            echo "$REPO/build/$NewAge_Config/$NewAge_Lane"
        fi
        ;;
esac
