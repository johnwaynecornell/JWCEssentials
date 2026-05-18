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
CREATE_SYMLINK="$REPO_ROOT/Bash/create_symlink.sh"

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

install_newage_root_context_wrapper_from_source() {
    local source="$1"
    local root="$2"
    local wrapper="$root/in_this_context.sh"

    if [ ! -f "$source" ]; then
        newage_fail "Missing context wrapper source: $source"
        return 1
    fi

    mkdir -p "$root"
    cp "$source" "$wrapper"
    chmod +x "$wrapper"

    newage_log "Installed root context wrapper: $wrapper"
}
ensure_new_age() {
    if [ -z "${NewAge:-}" ]; then
        newage_fail "NewAge is not set.

Set NewAge to the shared workspace root, for example:

  Linux / Git Bash:
    export NewAge=\"\$HOME/NewAge\"

  Windows PowerShell:
    [Environment]::SetEnvironmentVariable(\"NewAge\", \"C:\\Users\\vboxuser\\NewAge\", \"User\")"
        return 1
    fi

    NewAge="$(newage_to_unix_path "$NewAge")"
    export NewAge

    mkdir -p "$NewAge"
    mkdir -p "$NewAge/include"
    mkdir -p "$NewAge/bin"
    mkdir -p "$NewAge/lib"
    mkdir -p "$NewAge/DotNet/Libs/lib"

    mkdir -p "$NewAge/lib/Debug"
    mkdir -p "$NewAge/lib/Release"
    mkdir -p "$NewAge/bin/Debug"
    mkdir -p "$NewAge/bin/Release"

    touch "$NewAge/include/.anchor"
    touch "$NewAge/bin/.anchor"
    touch "$NewAge/lib/.anchor"
    touch "$NewAge/DotNet/Libs/lib/.anchor"

    install_newage_root_context_wrapper_from_source \
        "$REPO_ROOT/Bash/in_this_context_Package.sh.src" \
        "$NewAge"

    return 0
}

newage_install_script_to_destination() {
    local source="$1"
    local destination="$2"

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



path_contains_dir() {
    local needle="$1"
    local needle_canon

    needle_canon="$(newage_canonical_path "$needle")"

    IFS=':' read -r -a path_parts <<< "${PATH:-}"

    local part
    for part in "${path_parts[@]}"; do
        [ -z "$part" ] && continue

        if [ -d "$part" ]; then
            if [ "$(newage_canonical_path "$part")" = "$needle_canon" ]; then
                return 0
            fi
        fi
    done

    return 1
}

warn_if_newage_bin_not_on_path() {
    if ! path_contains_dir "$NewAge/bin"; then
        newage_warn "\$NewAge/bin is not currently on PATH."
        newage_warn "Post-build commands may fail to find NewAge helper scripts."
        newage_warn "For this shell session:"
        newage_warn "  export PATH=\"\$PATH:$NewAge/bin\""
        newage_warn ""
        newage_warn "For Windows user PATH, add:"
        newage_warn "  $(newage_to_windows_path "$NewAge/bin")"
    else
        newage_log "\$NewAge/bin is on PATH."
    fi
}

require_tool() {
    local tool="$1"

    if ! command -v "$tool" >/dev/null 2>&1; then
        newage_fail "Required tool not found on PATH: $tool"
    fi
}


print_runtime_lane_path_advice() {
    local config="${NEWAGE_CONFIG:-Debug}"
    local platform_lane
    platform_lane="$(newage_resolve_platform_lane "${NEWAGE_NATIVE_TOOLCHAIN:-}")"

    local full_lane="$config/$platform_lane"

    if newage_is_windows_shell; then
    local staged_bin="$NewAge/bin/$full_lane"
    local staged_lib="$NewAge/lib/$full_lane"

    local newage_win
    local newage_bin_win
    local staged_bin_win
    local staged_lib_win

    newage_win="$(newage_to_windows_path "$NewAge")"
    newage_bin_win="$(newage_to_windows_path "$NewAge/bin")"
    staged_bin_win="$(newage_to_windows_path "$staged_bin")"
    staged_lib_win="$(newage_to_windows_path "$staged_lib")"

    cat <<EOF

Runtime path advice for Windows:

Recommended: Use the context wrapper to enter a lane environment:

  cd "$NewAge"
  ./in_this_context.sh $config $platform_lane -- bash

Alternatively, for the current Git Bash shell session:

  export NewAge="$NewAge"
  export NewAge_Config="$config"
  export NewAge_Lane="$platform_lane"
  export PATH="\$PATH:$NewAge/bin"
  export PATH="\$PATH:$staged_bin"
  export PATH="\$PATH:$staged_lib"

PowerShell, current shell session:

  \$env:NewAge = "$newage_win"
  \$env:NewAge_Config = "$config"
  \$env:NewAge_Lane = "$platform_lane"
  \$env:Path += ";$newage_bin_win"
  \$env:Path += ";$staged_bin_win"
  \$env:Path += ";$staged_lib_win"

PowerShell, persist for the current user:

  [Environment]::SetEnvironmentVariable("NewAge", "$newage_win", "User")
  [Environment]::SetEnvironmentVariable("NewAge_Config", "$config", "User")
  [Environment]::SetEnvironmentVariable("NewAge_Lane", "$platform_lane", "User")

  \$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
  \$add = @(
      "$newage_bin_win",
      "$staged_bin_win",
      "$staged_lib_win"
  )

  foreach (\$p in \$add) {
      if ((\$userPath -split ';') -notcontains \$p) {
          \$userPath = "\$userPath;\$p"
      }
  }

  [Environment]::SetEnvironmentVariable("Path", \$userPath, "User")

Windows note:
  Executables may need both the staged bin lane and staged lib lane on Path
  so dependent DLLs such as JWCEssentials.dll can be found at runtime.

Expected Windows native lane:
  $full_lane

After changing the persistent user environment:
  restart PowerShell, Git Bash, VS Code, Visual Studio, Rider, CLion, or any
  terminal/IDE that should inherit the updated environment.

EOF
    return
fi

    local staged_bin="$NewAge/bin/$full_lane"
    local staged_lib="$NewAge/lib/$full_lane"

   cat <<EOF

   Runtime path advice for this shell session:

   Recommended: Use the context wrapper to enter a lane environment:

     cd "$NewAge"
     ./in_this_context.sh $config $platform_lane -- bash

   Alternatively, for this shell session:

     export NewAge="$NewAge"
     export NewAge_Config="$config"
     export NewAge_Lane="$platform_lane"
     export PATH="\$PATH:\$NewAge/bin"
     export PATH="\$PATH:$staged_bin"
     export LD_LIBRARY_PATH="$staged_lib:\${LD_LIBRARY_PATH:-}"

   Linux note:
     PATH finds staged command executables.
     LD_LIBRARY_PATH lets the dynamic loader find staged shared libraries.

   Expected Linux native lane:
     $full_lane

   If building with clang instead of gcc:

     ./in_this_context.sh $config clang -- bash

   For use on a development system this helps find native libraries as oposed to a deployment
   where native/managed live side by side

    sudo bash -c "echo \"$NewAge/lib/$full_lane\" > /etc/ld.so.conf.d/newage.conf"

    sudo ldconfig
    #varying by system this may need to be called after native builds within this system
EOF
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"
REPO_REL_PATH="JWCEssentials"

REGISTER_REPO_ROOT="0"
NEWAGE_ARG=""

while [ "$#" -gt 0 ]; do
    case "$1" in
        --newage)
            shift
            [ "$#" -gt 0 ] || { echo "--newage requires a path" >&2; exit 1; }
            NEWAGE_ARG="$1"
            ;;
        --register-repo-root)
            REGISTER_REPO_ROOT="1"
            ;;
        -h|--help)
            cat <<EOF
Usage:
  ./configure.sh [--newage PATH] [--register-repo-root]

Options:
  --newage PATH
      Use PATH as the NewAge workspace for this configure run.

  --register-repo-root
      Allow configure to create a live registration from:
        \$NewAge/JWCEssentials
      to this checkout when this checkout is not physically located there.
EOF
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 1
            ;;
    esac
    shift
done

if [ -n "$NEWAGE_ARG" ]; then
    export NewAge="$NEWAGE_ARG"
fi

export NEWAGE_REGISTER_REPO_ROOT="$REGISTER_REPO_ROOT"
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

# Bootstrap source before $NewAge/JWCEssentials exists.
NEWAGE_CONFIGURE_SCOPE="JWCEssentials configure"
. "$REPO_ROOT/Dev/NewAge.dev.sh"

NewAge="$(newage_to_unix_path "$NewAge")"
export NewAge

EXPECTED_REPO_ROOT="$NewAge/$REPO_REL_PATH"

newage_log "Repository root: $REPO_ROOT"
newage_log "NewAge workspace: $NewAge"
newage_log "Expected repo root: $EXPECTED_REPO_ROOT"

require_tool bash

ensure_new_age

NEWAGE_CREATE_SYMLINK="$REPO_ROOT/Bash/create_symlink.sh"

newage_register_repo_root "$REPO_ROOT" "$REPO_REL_PATH"


newage_register_directory \
    "$REPO_ROOT/include/JWCEssentials" \
    "$NewAge/include/JWCEssentials" \
    required

# $NewAge/DotNet/Libs/JWCEssentials.net is intentionally no longer registered.
# Repositories now live directly under $NewAge, while $NewAge/DotNet/Libs/lib
# remains the managed artifact staging area.
newage_log "Managed project location: $REPO_ROOT/Project/JWCEssentials.net"
newage_log "Managed artifact staging: $NewAge/DotNet/Libs/lib"

# Expose JWCEssentials Bash tools through $NewAge/bin.
#
# These are copied instead of linked. Command scripts are artifacts/helpers, not
# live source-directory registrations.
for script in "$REPO_ROOT"/Bash/*; do
    [ -f "$script" ] || continue
    newage_install_script_to_bin "$script"
done

if ! newage_is_windows_shell; then
        newage_install_script_to_destination "$REPO_ROOT/cygpath.sh.linux" "$NewAge/bin/cygpath"
fi

warn_if_newage_bin_not_on_path
print_runtime_lane_path_advice

newage_log "Workspace setup complete."

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
