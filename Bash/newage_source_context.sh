#!/usr/bin/env bash
# newage_source_context.sh — sourceable NewAge context entry.
#
# Sets the same environment that in_this_context.sh establishes, but
# modifies the current shell instead of launching a subprocess.
# Safe to source multiple times; path entries are not duplicated.
#
# MUST BE SOURCED:
#   . newage_source_context.sh [Config [Toolchain]]
#   source newage_source_context.sh Debug clang
#
# Arguments:
#   Config      Debug (default) | Release
#   Toolchain   gcc (default on Linux) | clang | msvc
#               Or a full lane:  Debug/Linux/x86_64/gcc
#                                Linux/x86_64/gcc
#
# After sourcing, the following are set:
#   NewAge            workspace root (inferred from script location if not set)
#   NewAge_Config     Debug | Release
#   NewAge_Lane       OS/Arch/Toolchain  (e.g. Linux/x86_64/gcc)
#   CC, CXX           compiler pair for chosen toolchain
#   PATH              lane bin path prepended, then NewAge bin (no duplicates)
#   LD_LIBRARY_PATH   lane lib path prepended (Linux; no duplicates)
#   PATH              lane lib path prepended (Windows, for DLL discovery)
#
# Examples:
#   . newage_source_context.sh                     # Debug, detected toolchain
#   . newage_source_context.sh Debug clang         # Debug with clang
#   . newage_source_context.sh Release             # Release, detected toolchain
#   . newage_source_context.sh Debug/Linux/x86_64/gcc   # full lane

# Guard: fail if executed directly instead of sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "ERROR: newage_source_context.sh must be sourced, not executed directly." >&2
    echo "Usage: . newage_source_context.sh [Config [Toolchain]]" >&2
    exit 1
fi

# --- Inlined helpers (independent; mirrors in_this_context.sh pattern) ---

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
    local os; os="$(_nsc_detect_os)"
    local arch; arch="$(_nsc_detect_arch)"

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

# Prepend $1 to PATH only if not already present.
_nsc_prepend_path() {
    case ":${PATH}:" in
        *":$1:"*) ;;
        *) export PATH="$1:${PATH}" ;;
    esac
}

# Prepend $1 to LD_LIBRARY_PATH only if not already present.
_nsc_prepend_ldpath() {
    local _cur="${LD_LIBRARY_PATH:-}"
    case ":${_cur}:" in
        *":$1:"*) ;;
        *) export LD_LIBRARY_PATH="$1:${_cur}" ;;
    esac
}

# --- Infer NewAge from script location if not already set ---

if [ -z "${NewAge:-}" ]; then
    _nsc_script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -L)"
    if [ -f "$_nsc_script_dir/NewAgeRepo.lst" ]; then
        export NewAge="$_nsc_script_dir"
    elif [ -f "$_nsc_script_dir/../NewAgeRepo.lst" ]; then
        export NewAge="$(cd "$_nsc_script_dir/.." && pwd -L)"
    fi
    unset _nsc_script_dir
fi

if [ -z "${NewAge:-}" ]; then
    echo "[newage_source_context] ERROR: NewAge is not set and could not be inferred." >&2
    echo "[newage_source_context] Set NewAge before sourcing, or source from \$NewAge/bin/." >&2
    return 1
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

# Prepend paths in order: root bin first, lane bin second (takes precedence).
# Idempotent — re-sourcing with the same lane is a no-op.
if _nsc_is_windows_shell; then
    _nsc_prepend_path "${NewAge}/lib/${_nsc_full_lane}"
else
    _nsc_prepend_ldpath "${NewAge}/lib/${_nsc_full_lane}"
fi

_nsc_prepend_path "${NewAge}/bin"
_nsc_prepend_path "${NewAge}/bin/${_nsc_full_lane}"

printf '[newage_source_context] Lane: %s / %s\n' "$NewAge_Config" "$NewAge_Lane"
[ -n "${CC:-}" ] && printf '[newage_source_context] Compiler: CC=%s  CXX=%s\n' "$CC" "$CXX"

# --- Cleanup temporaries ---
unset _nsc_arg1 _nsc_arg2 _nsc_config _nsc_platform_lane _nsc_full_lane _nsc_lane_toolchain
unset -f _nsc_is_windows_shell _nsc_detect_os _nsc_detect_arch _nsc_detect_toolchain \
         _nsc_resolve_platform_lane _nsc_set_cc_cxx \
         _nsc_prepend_path _nsc_prepend_ldpath
