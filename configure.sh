#!/usr/bin/env bash
set -euo pipefail

# JWCEssentials workspace configure script.
#
# Purpose:
#   Establish the minimal NewAge workspace layout and register JWCEssentials
#   into that workspace.
#
# Usage:
#   ./configure.sh
#   ./configure.sh --newage /path/to/NewAge
#
# Notes:
#   If NewAge is not already exported, --newage may be used for this run.
#   A script cannot permanently set the parent shell's environment variable.
#   To persist it, add the printed export/setx command to your shell/profile.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"

usage() {
    cat <<'EOF'
Usage:
  ./configure.sh [--newage PATH]

Options:
  --newage PATH    Use PATH as the NewAge workspace for this configure run.
  -h, --help       Show this help.

Environment:
  NewAge           Root of the shared NewAge workspace.

Examples:
  export NewAge="$HOME/NewAge"
  ./configure.sh

  ./configure.sh --newage "$HOME/NewAge"
EOF
}

log() {
    printf '[JWCEssentials configure] %s\n' "$*"
}

fail() {
    printf '[JWCEssentials configure] ERROR: %s\n' "$*" >&2
    exit 1
}

to_unix_path() {
    # Git Bash/MSYS/MinGW can translate Windows-style paths to Unix paths.
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -u "$1"
    else
        printf '%s\n' "$1"
    fi
}

link_or_replace() {
    local target="$1"
    local link="$2"

    if [ ! -e "$target" ] && [ ! -L "$target" ]; then
        fail "Cannot link missing target: $target"
    fi

    mkdir -p "$(dirname "$link")"

    if [ -e "$link" ] || [ -L "$link" ]; then
        # If it is already the correct symlink, leave it alone.
        if [ -L "$link" ]; then
            local existing
            existing="$(readlink "$link" || true)"
            if [ "$existing" = "$target" ]; then
                log "Link already exists: $link -> $target"
                return 0
            fi
        fi

        log "Replacing existing path: $link"
        rm -rf "$link"
    fi

    local create_symlink="$REPO_ROOT/Bash/create_symlink.sh"

    if [ -f "$create_symlink" ]; then
        bash "$create_symlink" "$target" "$link"
    else
        ln -s "$target" "$link"
    fi

    log "Linked: $link -> $target"
}

require_tool() {
    local tool="$1"

    if ! command -v "$tool" >/dev/null 2>&1; then
        fail "Required tool not found on PATH: $tool"
    fi
}

NEWAGE_ARG=""

while [ "$#" -gt 0 ]; do
    case "$1" in
        --newage)
            shift
            [ "$#" -gt 0 ] || fail "--newage requires a path"
            NEWAGE_ARG="$1"
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            fail "Unknown argument: $1"
            ;;
    esac
    shift
done

if [ -n "$NEWAGE_ARG" ]; then
    export NewAge="$(to_unix_path "$NEWAGE_ARG")"
fi

if [ -z "${NewAge:-}" ]; then
    cat >&2 <<'EOF'
[JWCEssentials configure] ERROR: NewAge is not set.

Set NewAge to the shared workspace root, for example:

  Linux / Git Bash:
    export NewAge="$HOME/NewAge"

  Windows PowerShell:
    [Environment]::SetEnvironmentVariable("NewAge", "C:\src\NewAge", "User")

Or run:

  ./configure.sh --newage "$HOME/NewAge"
EOF
    exit 1
fi

NewAge="$(to_unix_path "$NewAge")"
export NewAge

log "Repository root: $REPO_ROOT"
log "NewAge workspace: $NewAge"

require_tool bash

# Create the foundational workspace directories.
#
# $NewAge/include is intentionally first-class. Other repositories should be
# able to depend on this directory existing after JWCEssentials configure runs.
mkdir -p "$NewAge"
mkdir -p "$NewAge/include"
mkdir -p "$NewAge/bin"
mkdir -p "$NewAge/lib"
mkdir -p "$NewAge/DotNet/Libs"
mkdir -p "$NewAge/DotNet/Libs/lib"
mkdir -p "$NewAge/Repos"

# Anchor otherwise-empty directories for visibility and repo/tool friendliness.
touch "$NewAge/include/.anchor"
touch "$NewAge/bin/.anchor"
touch "$NewAge/lib/.anchor"
touch "$NewAge/DotNet/Libs/lib/.anchor"
touch "$NewAge/Repos/.anchor"

# Register this checkout in the recommended workspace repo location.
#
# This is a link, not a clone, so a user can run configure from any checkout
# and still get the standard $NewAge/Repos/JWCEssentials path.
link_or_replace "$REPO_ROOT" "$NewAge/Repos/JWCEssentials"

# Backward-compatible direct repo path.
#
# Existing NewAge scripts historically expected $NewAge/JWCEssentials.
# Keep this as a compatibility link during the integration refactor.
link_or_replace "$REPO_ROOT" "$NewAge/JWCEssentials"

# Expose public headers through the shared include directory.
link_or_replace "$REPO_ROOT/include/JWCEssentials" "$NewAge/include/JWCEssentials"

# Expose the managed project through the traditional NewAge .NET library area.
if [ -d "$REPO_ROOT/Project/JWCEssentials.net" ]; then
    link_or_replace "$REPO_ROOT/Project/JWCEssentials.net" "$NewAge/DotNet/Libs/JWCEssentials.net"
else
    log "Skipping .NET project link; Project/JWCEssentials.net not found."
fi

# Expose JWCEssentials Bash tools through $NewAge/bin.
#
# This makes commands such as create_symlink.sh, shuttle_to.sh, verbose.sh,
# NewAge_stage.sh, etc. available from the shared workspace bin directory.
if [ -d "$REPO_ROOT/Bash" ]; then
    while IFS= read -r script; do
        name="$(basename "$script")"
        link_or_replace "$script" "$NewAge/bin/$name"
    done < <(find "$REPO_ROOT/Bash" -maxdepth 1 -type f -name '*.sh' | sort)
else
    log "Skipping Bash tool exposure; Bash directory not found."
fi

# Create native artifact lane roots.
#
# The exact toolchain lane is selected by CMake/build configuration later.
# These roots prevent older scripts from failing before a lane exists.
mkdir -p "$NewAge/lib/Debug"
mkdir -p "$NewAge/lib/Release"
mkdir -p "$NewAge/bin/Debug"
mkdir -p "$NewAge/bin/Release"

log "Workspace setup complete."

cat <<EOF

JWCEssentials is registered with the NewAge workspace.

NewAge:
  $NewAge

Important paths:
  $NewAge/include
  $NewAge/include/JWCEssentials
  $NewAge/bin
  $NewAge/lib
  $NewAge/DotNet/Libs
  $NewAge/Repos/JWCEssentials

For this shell session:
  export NewAge="$NewAge"
  export PATH="\$PATH:$NewAge/bin"

Suggested verification:
  echo "\$NewAge"
  bash --version
  ls "$NewAge/include"
  ls "$NewAge/bin"

EOF