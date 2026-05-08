#!/usr/bin/env bash
set -euo pipefail

# Usage: create_symlink.sh <target_path> <link_name>
#
# On Unix-like systems this creates a symbolic link.
# On Windows/Git Bash it tries:
#   directory target: mklink /D, then mklink /J junction fallback
#   file target:      mklink file symlink
#
# Directory junction fallback avoids requiring Administrator privileges or
# Developer Mode for the common workspace-directory registration case.

usage() {
    echo "Usage: $0 <target_path> <link_name>" >&2
}

is_windows_shell() {
    case "$(uname -s)" in
        CYGWIN*|MINGW*|MSYS*) return 0 ;;
        *) return 1 ;;
    esac
}

cmd_run() {
    # Use /c with MSYS path conversion disabled for this process. This avoids
    # the ambiguity of cmd //C while still keeping Windows paths intact.
    MSYS2_ARG_CONV_EXCL='*' cmd /c "$*"
}

remove_existing_link_path() {
    local link="$1"

    if [ -L "$link" ] || [ -f "$link" ]; then
        rm "$link"
        return
    fi

    # Windows junctions are directories/reparse points. If we are on Windows,
    # remove the registration point with rmdir. This does not remove the target
    # directory contents.
    if is_windows_shell && [ -d "$link" ]; then
        local link_win
        link_win="$(cygpath -w "$link")"
        cmd_run "rmdir \"$link_win\"" >/dev/null 2>&1 && return
    fi
}

create_symlink_windows() {
    local target_unix="$1"
    local link_unix="$2"

    local target_win
    local link_win
    target_win="$(cygpath -w "$target_unix")"
    link_win="$(cygpath -w "$link_unix")"

    if [ -e "$link_unix" ] || [ -L "$link_unix" ]; then
        remove_existing_link_path "$link_unix"
    fi

    if [ -d "$target_unix" ]; then
        # Try directory symlink first. This may require Administrator privileges
        # or Developer Mode on Windows.
        if cmd_run "mklink /D \"$link_win\" \"$target_win\"" >/dev/null 2>&1; then
            return 0
        fi

        # Fall back to a directory junction. This usually works without
        # Administrator privileges and is sufficient for workspace registration.
        if cmd_run "mklink /J \"$link_win\" \"$target_win\"" >/dev/null 2>&1; then
            return 0
        fi

        echo "create_symlink.sh: failed to create Windows directory link/junction" >&2
        echo "  target: $target_win" >&2
        echo "  link:   $link_win" >&2
        return 1
    fi

    # File symlinks usually require Administrator privileges or Developer Mode.
    # Callers that need ordinary file availability should copy files instead.
    if cmd_run "mklink \"$link_win\" \"$target_win\"" >/dev/null 2>&1; then
        return 0
    fi

    echo "create_symlink.sh: failed to create Windows file symlink" >&2
    echo "  target: $target_win" >&2
    echo "  link:   $link_win" >&2
    return 1
}

create_symlink_unix() {
    local target="$1"
    local link="$2"

    if [ -e "$link" ] || [ -L "$link" ]; then
        rm -rf "$link"
    fi

    ln -s "$target" "$link"
}

create_symlink() {
    local target="$1"
    local link="$2"

    if [ ! -e "$target" ] && [ ! -L "$target" ]; then
        echo "create_symlink.sh: target does not exist: $target" >&2
        return 1
    fi

    mkdir -p "$(dirname "$link")"

    if is_windows_shell; then
        create_symlink_windows "$target" "$link"
    else
        create_symlink_unix "$target" "$link"
    fi
}

if [ "$#" -ne 2 ]; then
    usage
    exit 1
fi

create_symlink "$1" "$2"
