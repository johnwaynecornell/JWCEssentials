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

warn() {
    printf '[JWCEssentials configure] WARNING: %s\n' "$*" >&2
}

fail() {
    printf '[JWCEssentials configure] ERROR: %s\n' "$*" >&2
    exit 1
}

is_windows_shell() {
    case "$(uname -s)" in
        CYGWIN*|MINGW*|MSYS*) return 0 ;;
        *) return 1 ;;
    esac
}

to_unix_path() {
    # Git Bash/MSYS/MinGW can translate Windows-style paths to Unix paths.
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -u "$1"
    else
        printf '%s\n' "$1"
    fi
}

to_windows_path() {
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -w "$1"
    else
        printf '%s\n' "$1"
    fi
}

canonical_path() {
    local path="$1"

    if command -v cygpath >/dev/null 2>&1; then
        cygpath -am "$path"
        return
    fi

    if [ -d "$path" ]; then
        (cd "$path" && pwd -P)
        return
    fi

    local dir
    local base
    dir="$(dirname "$path")"
    base="$(basename "$path")"

    if [ -d "$dir" ]; then
        printf '%s/%s\n' "$(cd "$dir" && pwd -P)" "$base"
    else
        printf '%s\n' "$path"
    fi
}

same_path() {
    local left="$1"
    local right="$2"

    [ "$(canonical_path "$left")" = "$(canonical_path "$right")" ]
}

path_is_link_like() {
    local path="$1"

    # Unix symlink / MSYS symlink.
    if [ -L "$path" ]; then
        return 0
    fi

    # Windows junctions/symlinks created by mklink usually show up as reparse
    # points. fsutil requires Windows paths and may fail when unavailable; this
    # helper is intentionally best-effort.
    if is_windows_shell && command -v fsutil >/dev/null 2>&1; then
        local win_path
        win_path="$(to_windows_path "$path")"
        if fsutil reparsepoint query "$win_path" >/dev/null 2>&1; then
            return 0
        fi
    fi

    return 1
}

remove_link_like_path() {
    local path="$1"

    if [ -L "$path" ]; then
        rm "$path"
        return
    fi

    # Windows junctions must be removed with rmdir from cmd. Do not use rm -rf
    # here because we only want to remove the registration point, never the
    # target directory contents.
    if is_windows_shell; then
        local win_path
        win_path="$(to_windows_path "$path")"
        cmd /c "rmdir \"$win_path\"" >/dev/null 2>&1 && return
    fi

    fail "Refusing to remove non-symlink/non-junction path: $path"
}

create_directory_registration() {
    local target="$1"
    local link="$2"

    if is_windows_shell; then
        local target_win
        local link_win
        target_win="$(to_windows_path "$target")"
        link_win="$(to_windows_path "$link")"

        # Directory symlinks may require Administrator privileges or Developer
        # Mode. Junctions usually do not, and are sufficient for workspace
        # registration directories.
        if cmd /c "mklink /J \"$link_win\" \"$target_win\"" >/dev/null 2>&1; then
            return 0
        fi

        warn "Windows junction creation failed. Proof command:"
        warn "  cmd /c mklink /J \"$link_win\" \"$target_win\""
        return 1
    fi

    ln -s "$target" "$link"
}

link_or_register() {
    local target="$1"
    local link="$2"
    local required="${3:-required}"

    if [ ! -e "$target" ] && [ ! -L "$target" ]; then
        fail "Cannot register missing target: $target"
    fi

    mkdir -p "$(dirname "$link")"

    if [ -e "$link" ] || [ -L "$link" ]; then
        if same_path "$target" "$link"; then
            log "Already registered: $link"
            return 0
        fi

        if path_is_link_like "$link"; then
            log "Replacing existing link/junction: $link"
            remove_link_like_path "$link"
        else
            local message="Path exists and is not a link/junction: $link"

            if [ "$required" = "optional" ]; then
                warn "$message"
                warn "Skipping optional registration for: $link"
                return 0
            fi

            fail "$message"
        fi
    fi

    if [ -d "$target" ]; then
        if ! create_directory_registration "$target" "$link"; then
            if [ "$required" = "optional" ]; then
                warn "Could not create optional directory registration: $link -> $target"
                return 0
            fi
            fail "Could not create directory registration: $link -> $target"
        fi
    else
        # File registrations are not used for $NewAge/bin anymore. Keep this
        # for completeness on non-Windows systems only.
        if is_windows_shell; then
            if [ "$required" = "optional" ]; then
                warn "Skipping optional Windows file symlink: $link -> $target"
                return 0
            fi
            fail "Windows file symlink requested for required registration: $link -> $target"
        fi

        if ! ln -s "$target" "$link"; then
            if [ "$required" = "optional" ]; then
                warn "Could not create optional file registration: $link -> $target"
                return 0
            fi
            fail "Could not create file registration: $link -> $target"
        fi
    fi

    log "Registered: $link -> $target"
}

install_script_to_bin() {
    local source="$1"
    local destination="$2"

    if [ ! -f "$source" ]; then
        fail "Cannot install missing script: $source"
    fi

    mkdir -p "$(dirname "$destination")"

    if [ -e "$destination" ] || [ -L "$destination" ]; then
        if same_path "$source" "$destination"; then
            log "Script already available: $destination"
            return 0
        fi

        if path_is_link_like "$destination"; then
            log "Replacing existing script link/junction: $destination"
            remove_link_like_path "$destination"
        elif [ -f "$destination" ]; then
            log "Updating script copy: $destination"
            rm "$destination"
        else
            fail "Refusing to replace non-file path in bin: $destination"
        fi
    fi

    # On Windows, file symlinks normally require Administrator privileges or
    # Developer Mode. For $NewAge/bin, copied scripts are safer and sufficient:
    # post-build commands only need bash to locate the script on PATH.
    cp "$source" "$destination"
    chmod +x "$destination" 2>/dev/null || true
    log "Installed script: $destination"
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
# This is a link/junction, not a clone, so a user can run configure from any
# checkout and still get the standard $NewAge/Repos/JWCEssentials path.
link_or_register "$REPO_ROOT" "$NewAge/Repos/JWCEssentials" optional

# Backward-compatible direct repo path.
#
# Existing NewAge scripts historically expected $NewAge/JWCEssentials.
# Keep this as a compatibility registration during the integration refactor.
# This is required unless the current checkout already lives at that path.
link_or_register "$REPO_ROOT" "$NewAge/JWCEssentials" required

# Expose public headers through the shared include directory.
link_or_register "$REPO_ROOT/include/JWCEssentials" "$NewAge/include/JWCEssentials" required

# Expose the managed project through the traditional NewAge .NET library area.
if [ -d "$REPO_ROOT/Project/JWCEssentials.net" ]; then
    link_or_register "$REPO_ROOT/Project/JWCEssentials.net" "$NewAge/DotNet/Libs/JWCEssentials.net" required
else
    log "Skipping .NET project link; Project/JWCEssentials.net not found."
fi

# Expose JWCEssentials Bash tools through $NewAge/bin.
#
# These are copied instead of linked. On Windows, file symlinks often require
# elevation, and copied scripts are sufficient for PATH-based command lookup.
if [ -d "$REPO_ROOT/Bash" ]; then
    while IFS= read -r script; do
        name="$(basename "$script")"
        install_script_to_bin "$script" "$NewAge/bin/$name"
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
