#!/usr/bin/env bash
set -euo pipefail

# beta expect platform agnostic wrapping around portable runnable dll
# and removal of dotnet platform specific executables in
# collect_managed_bins_from_repo_list

SCRIPT_NAME="newage_collect"

usage() {
    cat <<EOF
Usage:
  $SCRIPT_NAME SOURCE_NEWAGE PACKAGE_ROOT [Config...]

Examples:
  $SCRIPT_NAME "\$NewAge" /tmp/CommandSpawnPackage
  $SCRIPT_NAME "\$NewAge" /tmp/CommandSpawnPackage Debug
  $SCRIPT_NAME "\$NewAge" /tmp/CommandSpawnPackage Debug Release

Arguments:
  SOURCE_NEWAGE
      Existing NewAge workspace containing built repos and staged outputs.

  PACKAGE_ROOT
      Target directory to populate as a NewAge-shaped package/dependency root.

  Config
      Optional build configurations to collect. Defaults to Debug.
      Examples: Debug, Release.

Behavior:
  - Creates a NewAge-shaped PACKAGE_ROOT.
  - Copies include, bin, lib, and DotNet/Libs/lib surfaces.
  - Copies managed project bin directories while preserving repo-relative paths.
  - Uses SOURCE_NEWAGE/NewAgeRepo.lst to discover repos.
EOF
}

log() {
    printf '[%s] %s\n' "$SCRIPT_NAME" "$*"
}

warn() {
    printf '[%s] WARNING: %s\n' "$SCRIPT_NAME" "$*" >&2
}

fail() {
    printf '[%s] ERROR: %s\n' "$SCRIPT_NAME" "$*" >&2
    exit 1
}

is_windows_shell() {
    case "$(uname -s)" in
        CYGWIN*|MINGW*|MSYS*) return 0 ;;
        *) return 1 ;;
    esac
}

canonical_path() {
    local path="$1"

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

copy_dir_contents() {
    local source_dir="$1"
    local dest_dir="$2"

    if [ ! -d "$source_dir" ]; then
        warn "Skipping missing directory: $source_dir"
        return 0
    fi

    mkdir -p "$dest_dir"

    # Use cp -a for the first simple version.
    # The trailing '/.' copies contents into the existing destination directory.
    cp -a "$source_dir/." "$dest_dir/"

    log "Copied:"
    log "  $source_dir"
    log "  -> $dest_dir"
}

copy_dir_replace() {
    local source_dir="$1"
    local dest_dir="$2"

    if [ ! -d "$source_dir" ]; then
        warn "Skipping missing directory: $source_dir"
        return 0
    fi

    rm -rf "$dest_dir"
    mkdir -p "$(dirname "$dest_dir")"
    cp -a "$source_dir" "$dest_dir"

    log "Copied:"
    log "  $source_dir"
    log "  -> $dest_dir"
}

install_newage_root_context_wrapper_from_source() {
    local source="$1"
    local root="$2"
    local wrapper="$root/in_this_context.sh"

    if [ ! -f "$source" ]; then
        fail "Missing context wrapper source: $source"
        return 1
    fi

    mkdir -p "$root"
    cp "$source" "$wrapper"
    chmod +x "$wrapper"

    log "Installed root context wrapper: $wrapper"
}

ensure_package_newage_shape() {
    local package_root="$1"

    mkdir -p "$package_root"
    mkdir -p "$package_root/include"
    mkdir -p "$package_root/bin"
    mkdir -p "$package_root/lib"
    mkdir -p "$package_root/DotNet/Libs/lib"

    touch "$package_root/include/.anchor"
    touch "$package_root/bin/.anchor"
    touch "$package_root/lib/.anchor"
    touch "$package_root/DotNet/Libs/lib/.anchor"

    if [ ! -f "$package_root/NewAgeRepo.lst" ]; then
        {
            echo "# NewAgeRepo.lst"
            echo "# Relative repository paths included in this package root."
            echo
        } > "$package_root/NewAgeRepo.lst"
    fi
}

add_package_repo_entry() {
    local package_root="$1"
    local repo_rel="$2"
    local list_file="$package_root/NewAgeRepo.lst"

    if ! grep -Fxq "$repo_rel" "$list_file"; then
        echo "$repo_rel" >> "$list_file"
        log "Added package repo entry: $repo_rel"
    fi
}

collect_include_surface() {
    local source_newage="$1"
    local package_root="$2"

    copy_dir_contents "$source_newage/include" "$package_root/include"
}

collect_dotnet_lib_surface() {
    local source_newage="$1"
    local package_root="$2"

    copy_dir_contents "$source_newage/DotNet/Libs/lib" "$package_root/DotNet/Libs/lib"
}

collect_native_lanes() {
    local source_newage="$1"
    local package_root="$2"
    shift 2

    local configs=("$@")
    local config

    for config in "${configs[@]}"; do
        if [ -d "$source_newage/bin/$config" ]; then
            copy_dir_contents "$source_newage/bin/$config" "$package_root/bin/$config"
        else
            warn "No staged bin config found: $source_newage/bin/$config"
        fi

        if [ -d "$source_newage/lib/$config" ]; then
            copy_dir_contents "$source_newage/lib/$config" "$package_root/lib/$config"
        else
            warn "No staged lib config found: $source_newage/lib/$config"
        fi
    done
}

relative_path_from_to() {
    local base="$1"
    local path="$2"

    if command -v realpath >/dev/null 2>&1; then
        realpath --relative-to="$base" "$path"
        return
    fi

    # Fallback: this assumes path starts with base.
    path="${path#"$base"/}"
    printf '%s\n' "$path"
}

transform_bin_dir() {
    local repo_root="$1"
    local repo_rel="$2"
    local package_root="$3"
    local project_dir="$4"

    local source_bin="$project_dir/bin"

    if [ ! -d "$source_bin" ]; then
        return 0
    fi

    local rel_project_dir
    rel_project_dir="$(relative_path_from_to "$repo_root" "$project_dir")"

    local dest_bin="$package_root/$repo_rel/$rel_project_dir/bin"

    rm -rf "$dest_bin"
    mkdir -p "$(dirname "$dest_bin")"
    cp -a "$source_bin" "$dest_bin"

    log "Copied managed bin:"
    log "  $source_bin"
    log "  -> $dest_bin"
}

collect_managed_bins_for_repo() {
    local source_newage="$1"
    local package_root="$2"
    local repo_rel="$3"

    local repo_root="$source_newage/$repo_rel"

    if [ ! -d "$repo_root" ]; then
        warn "Skipping missing repo: $repo_root"
        return 0
    fi

    add_package_repo_entry "$package_root" "$repo_rel"

    while IFS= read -r csproj; do
        local project_dir
        project_dir="$(dirname "$csproj")"

        transform_bin_dir "$repo_root" "$repo_rel" "$package_root" "$project_dir"
    done < <(find "$repo_root" -name '*.csproj' -type f | sort)
}

collect_managed_bins_from_repo_list() {
    local source_newage="$1"
    local package_root="$2"
    local repo_list="$source_newage/NewAgeRepo.lst"

    if [ ! -f "$repo_list" ]; then
        fail "Missing repo list: $repo_list"
    fi

    while IFS= read -r repo_rel; do
        # Trim simple whitespace.
        repo_rel="$(printf '%s' "$repo_rel" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"

        [ -z "$repo_rel" ] && continue

        case "$repo_rel" in
            \#*) continue ;;
        esac

        collect_managed_bins_for_repo "$source_newage" "$package_root" "$repo_rel"
    done < "$repo_list"
}

write_package_note() {
    local source_newage="$1"
    local package_root="$2"
    shift 2
    local configs=("$@")

    {
        echo "NewAge package root"
        echo
        echo "Source NewAge:"
        echo "  $source_newage"
        echo
        echo "Package root:"
        echo "  $package_root"
        echo
        echo "Collected configurations:"
        for config in "${configs[@]}"; do
            echo "  $config"
        done
        echo
        echo "Created:"
        date
    } > "$package_root/PACKAGE-INFO.txt"
}

if [ "$#" -lt 2 ]; then
    usage >&2
    exit 1
fi

SOURCE_NEWAGE="$1"
PACKAGE_ROOT="$2"
shift 2

if [ "$#" -eq 0 ]; then
    CONFIGS=("Debug")
else
    CONFIGS=("$@")
fi

SOURCE_NEWAGE="$(canonical_path "$SOURCE_NEWAGE")"

mkdir -p "$PACKAGE_ROOT"
PACKAGE_ROOT="$(canonical_path "$PACKAGE_ROOT")"

if [ ! -d "$SOURCE_NEWAGE" ]; then
    fail "SOURCE_NEWAGE does not exist: $SOURCE_NEWAGE"
fi

log "Source NewAge: $SOURCE_NEWAGE"
log "Package root:  $PACKAGE_ROOT"
log "Configurations: ${CONFIGS[*]}"

ensure_package_newage_shape "$PACKAGE_ROOT"
install_newage_root_context_wrapper_from_source \
    "$SOURCE_NEWAGE/JWCEssentials/Bash/in_this_context_Package.sh.src" \
    "$PACKAGE_ROOT"

collect_include_surface "$SOURCE_NEWAGE" "$PACKAGE_ROOT"
collect_dotnet_lib_surface "$SOURCE_NEWAGE" "$PACKAGE_ROOT"
collect_native_lanes "$SOURCE_NEWAGE" "$PACKAGE_ROOT" "${CONFIGS[@]}"
collect_managed_bins_from_repo_list "$SOURCE_NEWAGE" "$PACKAGE_ROOT"

write_package_note "$SOURCE_NEWAGE" "$PACKAGE_ROOT" "${CONFIGS[@]}"

log "Package collection complete."