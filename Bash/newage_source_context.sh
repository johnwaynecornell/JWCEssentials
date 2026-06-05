#!/usr/bin/env bash
# newage_source_context.sh — sourceable NewAge context entry.
#
# Sets the same environment that in_this_context.sh establishes, but
# modifies the current shell instead of launching a subprocess.
#
# MUST BE SOURCED:
#   . newage_source_context.sh [Config [Toolchain]]
#   source newage_source_context.sh Debug clang
#
# Arguments:
#   Config      Debug (default) | Release
#   Toolchain   gcc (default on Linux) | clang | msvc
#               Or a full lane: Debug/Linux/x86_64/gcc
#
# After sourcing, the following are set:
#   NewAge            workspace root (if not already set, inferred from script location)
#   NewAge_Config     Debug | Release
#   NewAge_Lane       OS/Arch/Toolchain
#   CC, CXX           compiler pair for chosen toolchain
#   PATH              prepended with $NewAge/bin and the lane bin path
#   LD_LIBRARY_PATH   prepended with the lane lib path (Linux)
#   PATH              prepended with the lane lib path (Windows, for DLL discovery)

# Guard: warn if executed directly instead of sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "ERROR: newage_source_context.sh must be sourced, not executed directly." >&2
    echo "Usage: . newage_source_context.sh [Config [Toolchain]]" >&2
    exit 1
fi

# --- Inlined helpers (kept independent; mirrors in_this_context.sh pattern) ---

_nsc_is_windows_shell() {
    case "$(uname -s)" in
        CYGWIN*|MINGW*|MSYS*) return 0 ;;
        *) return 1 ;;
    esac
}

_nsc_detect_os()       { _nsc_is_windows_shell && echo "Windows" && return; uname -s; }
_nsc_detect_arch()     { _nsc_is_windows_shell && echo "AMD64"   && return; uname -m; }
_nsc_detect_toolchain(){ _nsc_is_windows_shell && echo "msvc"    && return; echo "gcc"; }

_nsc_resolve_platform_lane() {
    local input="${1:-}"
    input="${input//\\//}"

    if [[ "$input" == */* ]]; then
        case "${input%%/*}" in
            Debug|debug|Release|release) echo "${input#*/}"; return ;;
            *) echo "$input"; return ;;
        esac
    fi

    local tc="${input:-$(_nsc_detect_toolchain)}"
    local os="$(_nsc_detect_os)"
    local arch="$(_nsc_detect_arch)"

    [ "$tc" = "clang" ] && [ "$os" = "Windows" ] && tc="clang-cl"

    echo "$os/$arch/$tc"
}

_nsc_set_cc_cxx() {
    case "$1" in
        msvc)     export CC=cl;       export CXX=cl       ;;
        clang-cl) export CC=clang-cl; export CXX=clang-cl ;;
        clang)    export CC=clang;    export CXX=clang++   ;;
        gcc)      export CC=gcc;      export CXX=g++       ;;
    esac
}

# --- Infer NewAge from script location if not already set ---

if [ -z "${NewAge:-}" ]; then
    # Script lives at $NewAge/bin/newage_source_context.sh after configure.
    # Walk up to find the workspace root (contains NewAgeRepo.lst).
    _nsc_script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -L)"
    if [ -f "$_nsc_script_dir/NewAgeRepo.lst" ]; then
        export NewAge="$_nsc_script_dir"
    elif [ -f "$_nsc_script_dir/../NewAgeRepo.lst" ]; then
        export NewAge="$(cd "$_nsc_script_dir/.." && pwd -L)"
    else
        echo "[newage_source_context] WARNING: NewAge is not set and could not be inferred." >&2
    fi
    unset _nsc_script_dir
fi

# --- Parse arguments ---

_nsc_arg1="${1:-}"
_nsc_arg2="${2:-}"
_nsc_arg1="${_nsc_arg1//\\//}"

_nsc_config="Debug"
_nsc_platform_lane=""

case "$_nsc_arg1" in
    Debug|debug|Release|release)
        _nsc_config="$_nsc_arg1"
        [[ "$_nsc_config" == "debug"   ]] && _nsc_config="Debug"
        [[ "$_nsc_config" == "release" ]] && _nsc_config="Release"
        _nsc_platform_lane="$(_nsc_resolve_platform_lane "${_nsc_arg2:-}")"
        ;;
    */*)
        case "${_nsc_arg1%%/*}" in
            Debug|debug|Release|release)
                _nsc_config="${_nsc_arg1%%/*}"
                [[ "$_nsc_config" == "debug"   ]] && _nsc_config="Debug"
                [[ "$_nsc_config" == "release" ]] && _nsc_config="Release"
                _nsc_platform_lane="${_nsc_arg1#*/}"
                ;;
            *)
                _nsc_config="${NewAge_Config:-Debug}"
                _nsc_platform_lane="$_nsc_arg1"
                ;;
        esac
        ;;
    "")
        _nsc_config="${NewAge_Config:-Debug}"
        _nsc_platform_lane="$(_nsc_resolve_platform_lane "${_nsc_arg2:-}")"
        ;;
    *)
        _nsc_config="${NewAge_Config:-Debug}"
        _nsc_platform_lane="$(_nsc_resolve_platform_lane "$_nsc_arg1")"
        ;;
esac

export NewAge_Config="$_nsc_config"
export NewAge_Lane="$_nsc_platform_lane"

_nsc_full_lane="$_nsc_config/$_nsc_platform_lane"
_nsc_lane_toolchain="${_nsc_platform_lane##*/}"

_nsc_set_cc_cxx "$_nsc_lane_toolchain"

if _nsc_is_windows_shell; then
    export PATH="${NewAge}/lib/${_nsc_full_lane}:${PATH}"
else
    export LD_LIBRARY_PATH="${NewAge}/lib/${_nsc_full_lane}:${LD_LIBRARY_PATH:-}"
fi

export PATH="${NewAge}/bin/${_nsc_full_lane}:${NewAge}/bin:${PATH}"

printf '[newage_source_context] Lane: %s / %s\n' "$NewAge_Config" "$NewAge_Lane"
[ -n "${CC:-}" ] && printf '[newage_source_context] Compiler: CC=%s  CXX=%s\n' "$CC" "$CXX"

# --- Cleanup temporaries ---
unset _nsc_arg1 _nsc_arg2 _nsc_config _nsc_platform_lane _nsc_full_lane _nsc_lane_toolchain
unset -f _nsc_is_windows_shell _nsc_detect_os _nsc_detect_arch _nsc_detect_toolchain \
         _nsc_resolve_platform_lane _nsc_set_cc_cxx
