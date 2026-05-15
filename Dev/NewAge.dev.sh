#!/usr/bin/env bash

# NewAge.dev.sh
#
# Shared NewAge-family development helpers.
#
# Intended to be sourced by configure.sh scripts:
#
#   . "$NewAge/JWCEssentials/Dev/NewAge.dev.sh"
#
# This file should not exit the shell directly unless a caller explicitly wants
# fail-fast behavior through helper functions.
#

# NewAge workspace/package doctrine
#
# Development workspace:
#   A NewAge workspace is an enter-able dependency supplier for NewAge-family
#   repositories. JWCEssentials owns the bootstrap helpers and establishes the
#   workspace floor.
#
# Package root:
#   A collected package may use the same NewAge-shaped layout without being a
#   Git checkout. It should be enter-able through in_this_context.sh and should
#   resolve native dependencies from its own staged lanes before any system
#   loader configuration.
#
# Native lanes:
#   Native artifacts are staged by:
#
#     bin/<Config>/<OS>/<Arch>/<Toolchain>
#     lib/<Config>/<OS>/<Arch>/<Toolchain>
#
# Managed outputs:
#   Managed project outputs preserve repo-relative project paths inside the
#   package root to avoid collisions.
#
# Application wrappers:
#   App-specific launchers such as main.sh/main.bat are optional and should be
#   generated only when an application needs a front door. The package root
#   itself is a substrate, not necessarily the final app shape.
newage_log() {
    local scope="${NEWAGE_CONFIGURE_SCOPE:-NewAge}"
    printf '[%s] %s\n' "$scope" "$*"
}

newage_warn() {
    local scope="${NEWAGE_CONFIGURE_SCOPE:-NewAge}"
    printf '[%s] WARNING: %s\n' "$scope" "$*" >&2
}

newage_fail() {
    local scope="${NEWAGE_CONFIGURE_SCOPE:-NewAge}"
    printf '[%s] ERROR: %s\n' "$scope" "$*" >&2
    return 1
}

newage_is_windows_shell() {
    case "$(uname -s)" in
        CYGWIN*|MINGW*|MSYS*) return 0 ;;
        *) return 1 ;;
    esac
}

newage_to_unix_path() {
    if newage_is_windows_shell && command -v cygpath >/dev/null 2>&1; then
        cygpath -u "$1"
    else
        printf '%s\n' "$1"
    fi
}

newage_to_windows_path() {
    if newage_is_windows_shell && command -v cygpath >/dev/null 2>&1; then
        cygpath -w "$1"
    else
        printf '%s\n' "$1"
    fi
}

newage_canonical_path() {
    local path="$1"

    if [ -d "$path" ]; then
        local resolved
        resolved="$(cd "$path" && pwd -P)"

        if newage_is_windows_shell && command -v cygpath >/dev/null 2>&1; then
            cygpath -am "$resolved"
        else
            printf '%s\n' "$resolved"
        fi

        return
    fi

    if [ -f "$path" ] || [ -L "$path" ]; then
        local dir
        local base
        local resolved_dir

        dir="$(dirname "$path")"
        base="$(basename "$path")"

        if [ -d "$dir" ]; then
            resolved_dir="$(cd "$dir" && pwd -P)"

            if newage_is_windows_shell && command -v cygpath >/dev/null 2>&1; then
                cygpath -am "$resolved_dir/$base"
            else
                printf '%s/%s\n' "$resolved_dir" "$base"
            fi

            return
        fi
    fi

    if newage_is_windows_shell && command -v cygpath >/dev/null 2>&1; then
        cygpath -am "$path"
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

newage_same_path() {
    [ "$(newage_canonical_path "$1")" = "$(newage_canonical_path "$2")" ]
}

newage_add_repo_entry() {
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
        newage_log "Added repo entry: $rel_path"
    else
        newage_log "Repo entry already present: $rel_path"
    fi
}

newage_path_is_link_like() {
    local path="$1"

    if [ -L "$path" ]; then
        return 0
    fi

    if newage_is_windows_shell && command -v fsutil >/dev/null 2>&1; then
        local win_path
        win_path="$(newage_to_windows_path "$path")"

        if fsutil reparsepoint query "$win_path" >/dev/null 2>&1; then
            return 0
        fi
    fi

    return 1
}

newage_remove_link_like_path() {
    local path="$1"

    if [ -L "$path" ]; then
        rm "$path"
        return 0
    fi

    if newage_is_windows_shell; then
        local win_path
        win_path="$(newage_to_windows_path "$path")"

        cmd /c "rmdir \"$win_path\"" >/dev/null 2>&1 && return 0
    fi

    newage_fail "Refusing to remove non-symlink/non-junction path: $path"
}

newage_register_directory() {
    local target="$1"
    local link="$2"
    local required="${3:-required}"

    if [ ! -e "$target" ] && [ ! -L "$target" ]; then
        newage_fail "Cannot register missing target: $target"
        return 1
    fi

    mkdir -p "$(dirname "$link")"

    if [ -e "$link" ] || [ -L "$link" ]; then
        if newage_same_path "$target" "$link"; then
            newage_log "Already registered: $link"
            return 0
        fi

        if newage_path_is_link_like "$link"; then
            newage_log "Replacing existing link/junction: $link"
            newage_remove_link_like_path "$link" || return 1
        else
            if [ "$required" = "optional" ]; then
                newage_warn "Path exists and is not a link/junction: $link"
                newage_warn "Skipping optional registration for: $link"
                return 0
            fi

            newage_fail "Path exists and is not a link/junction: $link"
            return 1
        fi
    fi

    if [ -z "${NEWAGE_CREATE_SYMLINK:-}" ]; then
        NEWAGE_CREATE_SYMLINK="$NewAge/JWCEssentials/Bash/create_symlink.sh"
    fi

    if [ ! -f "$NEWAGE_CREATE_SYMLINK" ]; then
        newage_fail "Missing create_symlink helper: $NEWAGE_CREATE_SYMLINK"
        return 1
    fi

    if ! bash "$NEWAGE_CREATE_SYMLINK" "$target" "$link"; then
        if [ "$required" = "optional" ]; then
            newage_warn "Could not create optional directory registration: $link -> $target"
            return 0
        fi

        newage_fail "Could not create required directory registration: $link -> $target"
        return 1
    fi

    newage_log "Registered: $link -> $target"
}

newage_install_script_to_bin() {
    local source="$1"
    local destination="$NewAge/bin/$(basename "$source")"

    if [ ! -f "$source" ]; then
        newage_fail "Cannot install missing script: $source"
        return 1
    fi

    mkdir -p "$NewAge/bin"

    if [ -e "$destination" ] || [ -L "$destination" ]; then
        if newage_same_path "$source" "$destination"; then
            newage_log "Script already available: $destination"
            return 0
        fi

        if newage_path_is_link_like "$destination"; then
            newage_log "Replacing existing script link/junction: $destination"
            newage_remove_link_like_path "$destination" || return 1
        elif [ -f "$destination" ]; then
            newage_log "Updating script copy: $destination"
            rm "$destination"
        else
            newage_fail "Refusing to replace non-file path in bin: $destination"
            return 1
        fi
    fi

    cp "$source" "$destination"
    chmod +x "$destination" 2>/dev/null || true

    newage_log "Installed script: $destination"
}

newage_register_repo_root() {
    local repo_root="$1"
    local repo_rel_path="$2"
    local expected_repo_root="$NewAge/$repo_rel_path"

    newage_add_repo_entry "$repo_rel_path"

    if newage_same_path "$repo_root" "$expected_repo_root"; then
        newage_log "Repository is already at expected workspace path."
        return 0
    fi

    if [ -e "$expected_repo_root" ] || [ -L "$expected_repo_root" ]; then
        if newage_same_path "$repo_root" "$expected_repo_root"; then
            newage_log "Repository is already registered at expected workspace path."
            return 0
        fi

        newage_fail "Expected repo path already exists and points elsewhere: $expected_repo_root"
        return 1
    fi

    newage_warn "Repository is not physically located at expected NewAge workspace path."
    newage_warn "  expected: $expected_repo_root"
    newage_warn "  actual:   $repo_root"

    if [ "${NEWAGE_REGISTER_REPO_ROOT:-0}" != "1" ]; then
        newage_fail "Detached repo-root registration requires explicit opt-in with --register-repo-root.

Preferred option:
  clone this repository directly at:
    $expected_repo_root

Intentional detached checkout option:
  rerun configure with:
    ./configure.sh --register-repo-root

What this does:
  creates a live workspace registration:
    $expected_repo_root -> $repo_root"
        return 1
    fi

    newage_warn "Explicit opt-in received."
    newage_warn "Creating live workspace registration:"
    newage_warn "  $expected_repo_root -> $repo_root"

    newage_register_directory "$repo_root" "$expected_repo_root" required
}

set_lane_environment() {
    local config="$1"

    if newage_is_windows_shell; then
        lane="$config/Windows/AMD64/msvc"
        export PATH="$NewAge/lib/$lane:$PATH"
    else
        lane="$config/Linux/x86_64/gcc"
        export LD_LIBRARY_PATH="$NewAge/lib/$lane:${LD_LIBRARY_PATH:-}"
    fi

    export PATH="$NewAge/bin/$lane:$NewAge/bin:$PATH"
}
