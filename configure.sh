#!/usr/bin/env bash
set -euo pipefail

# JWCEssentials workspace configure script.
#
# Purpose:
#   Establish the minimal NewAge workspace layout and register JWCEssentials
#   into that workspace.
#
# Current doctrine:
#   - $NewAge is the workspace root.
#   - Repositories live physically under $NewAge, for example:
#       $NewAge/JWCEssentials
#       $NewAge/JWCCommandSpawn
#       $NewAge/CrystalCatalystLibrary
#       $NewAge/NewAge
#   - $NewAge/include is part of the build contract.
#   - $NewAge/include/JWCEssentials is required.
#   - $NewAge/bin receives copied command scripts.
#   - $NewAge/DotNet/Libs/lib remains the managed artifact staging area.
#   - $NewAge/DotNet/Libs/JWCEssentials.net is no longer required.
#   - $NewAge/NewAgeRepo.lst is a simple list of repo paths relative to $NewAge.
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
REPO_NAME="JWCEssentials"
REPO_REL_PATH="JWCEssentials"

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

    if [ -L "$path" ]; then
        return 0
    fi

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

    if is_windows_shell; then
        local win_path
        win_path="$(to_windows_path "$path")"

        # Junctions/reparse points should be removed with rmdir so only the
        # registration point is removed, not target contents.
        cmd /c "rmdir \"$win_path\"" >/dev/null 2>&1 && return
    fi

    fail "Refusing to remove non-symlink/non-junction path: $path"
}

cmd_run() {
    # Use /c with MSYS path conversion disabled for this process. This avoids
    # the ambiguity of cmd //C while still keeping Windows paths intact.
    MSYS2_ARG_CONV_EXCL='*' cmd /c "$*"
}

create_directory_registration() {
    local target="$1"
    local link="$2"

    if is_windows_shell; then
        local target_win
        local link_win

        target_win="$(to_windows_path "$target")"
        link_win="$(to_windows_path "$link")"

        # Try a directory junction. On Windows this is the intended live
        # directory-registration mechanism when symbolic links are unavailable.
        #
        # If this fails, the user likely needs Developer Mode, an elevated shell,
        # or a physical expected-path checkout depending on the registration.
        if cmd_run "mklink /J \"$link_win\" \"$target_win\"" >/dev/null 2>&1; then
            return 0
        fi

        warn "Windows directory registration failed. Proof command:"
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

    if ! create_directory_registration "$target" "$link"; then
        if [ "$required" = "optional" ]; then
            warn "Could not create optional directory registration: $link -> $target"
            return 0
        fi

        fail "Could not create required directory registration: $link -> $target"
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

    cp "$source" "$destination"
    chmod +x "$destination" 2>/dev/null || true

    log "Installed script: $destination"
}

add_repo_entry() {
    local rel_path="$1"
    local list_file="$NewAge/NewAgeRepo.lst"

    mkdir -p "$(dirname "$list_file")"

    if [ ! -f "$list_file" ]; then
        {
            echo "# NewAgeRepo.lst"
            echo "# Relative repository paths under \$NewAge."
            echo
        } > "$list_file"
    fi

    if ! grep -Fxq "$rel_path" "$list_file"; then
        echo "$rel_path" >> "$list_file"
        log "Added repo entry: $rel_path"
    else
        log "Repo entry already present: $rel_path"
    fi
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
    [Environment]::SetEnvironmentVariable("NewAge", "C:\Users\vboxuser\NewAge", "User")

Or run:

  ./configure.sh --newage "$HOME/NewAge"
EOF
    exit 1
fi

NewAge="$(to_unix_path "$NewAge")"
export NewAge

EXPECTED_REPO_ROOT="$NewAge/$REPO_REL_PATH"

log "Repository root: $REPO_ROOT"
log "NewAge workspace: $NewAge"
log "Expected repo root: $EXPECTED_REPO_ROOT"

require_tool bash

# Create foundational workspace directories.
mkdir -p "$NewAge"
mkdir -p "$NewAge/include"
mkdir -p "$NewAge/bin"
mkdir -p "$NewAge/lib"
mkdir -p "$NewAge/DotNet/Libs/lib"

# Native artifact lane roots. The toolchain lane is selected by CMake/build
# configuration later.
mkdir -p "$NewAge/lib/Debug"
mkdir -p "$NewAge/lib/Release"
mkdir -p "$NewAge/bin/Debug"
mkdir -p "$NewAge/bin/Release"

# Anchor otherwise-empty directories for visibility and tooling friendliness.
touch "$NewAge/include/.anchor"
touch "$NewAge/bin/.anchor"
touch "$NewAge/lib/.anchor"
touch "$NewAge/DotNet/Libs/lib/.anchor"

# Record this repository as a simple relative workspace entry.
add_repo_entry "$REPO_REL_PATH"

# The preferred decentralized layout is a physical checkout at:
#
#   $NewAge/JWCEssentials
#
# If configure is run from elsewhere, we try to register that compatibility path.
# On Windows this may require working junction support. If the user wants the
# least surprising Windows path, clone directly into $NewAge/JWCEssentials.
if same_path "$REPO_ROOT" "$EXPECTED_REPO_ROOT"; then
    log "Repository is already at expected workspace path."
elif [ -e "$EXPECTED_REPO_ROOT" ] || [ -L "$EXPECTED_REPO_ROOT" ]; then
    if same_path "$REPO_ROOT" "$EXPECTED_REPO_ROOT"; then
        log "Repository is already registered at expected workspace path."
    else
        fail "Expected repo path already exists and points elsewhere: $EXPECTED_REPO_ROOT"
    fi
else
    warn "Repository is not physically located at the expected workspace path."
    warn "Attempting registration: $EXPECTED_REPO_ROOT -> $REPO_ROOT"
    link_or_register "$REPO_ROOT" "$EXPECTED_REPO_ROOT" required
fi

# $NewAge/include/JWCEssentials is part of the build contract.
# Do not silently copy this directory; it must be live-registered or physically
# present as the correct source directory.
link_or_register "$REPO_ROOT/include/JWCEssentials" "$NewAge/include/JWCEssentials" required

# $NewAge/DotNet/Libs/JWCEssentials.net is intentionally no longer registered.
# Repositories now live directly under $NewAge, while $NewAge/DotNet/Libs/lib
# remains the managed artifact staging area.
log "Managed project location: $REPO_ROOT/Project/JWCEssentials.net"
log "Managed artifact staging: $NewAge/DotNet/Libs/lib"

# Expose JWCEssentials Bash tools through $NewAge/bin.
#
# These are copied instead of linked. Command scripts are artifacts/helpers, not
# live source-directory registrations.
if [ -d "$REPO_ROOT/Bash" ]; then
    while IFS= read -r script; do
        name="$(basename "$script")"
        install_script_to_bin "$script" "$NewAge/bin/$name"
    done < <(find "$REPO_ROOT/Bash" -maxdepth 1 -type f -name '*.sh' | sort)
else
    log "Skipping Bash tool exposure; Bash directory not found."
fi

log "Workspace setup complete."

cat <<EOF

JWCEssentials is configured for the NewAge workspace.

NewAge:
  $NewAge

Repository:
  $REPO_ROOT

Important paths:
  $NewAge/NewAgeRepo.lst
  $NewAge/include
  $NewAge/include/JWCEssentials
  $NewAge/bin
  $NewAge/lib
  $NewAge/DotNet/Libs/lib

For this shell session:
  export NewAge="$NewAge"
  export PATH="\$PATH:$NewAge/bin"

Suggested verification:
  echo "\$NewAge"
  cat "$NewAge/NewAgeRepo.lst"
  bash --version
  ls "$NewAge/include"
  ls "$NewAge/bin"

EOF