#!/usr/bin/env bash
# newage_export.sh — serialize the current NewAge environment for IDE
# configuration, shell upload, CI, or cross-environment transfer.
#
# Requires NewAge, NewAge_Config, and NewAge_Lane to all be set.
# Establish them first with newage_source_context.sh or in_this_context.sh.
#
# Usage:
#   newage_export.sh [--bash|--powershell|--json|--env] [--path]
#
# Formats:
#   --bash        export VAR=value statements (default); sourceable in bash
#   --powershell  $env:VAR = "value" assignments for PowerShell
#   --json        {"VAR": "value"} object; suitable for IDE env config or API payloads
#   --env         VAR=value dotenv format; works with direnv, Docker --env-file, etc.
#
# --path behavior per format:
#   --bash        appends: export PATH=... and export LD_LIBRARY_PATH=...
#                 uses nested variable references ($NewAge/$NewAge_Config/$NewAge_Lane)
#   --powershell  mutates $env:Path (lib lane then bin, so bin takes precedence)
#                 uses $env:NewAge/$env:NewAge_Config/$env:NewAge_Lane references
#   --json        emits NewAge_BinPath and NewAge_LibPath as fully-expanded strings
#   --env         appends PATH=... and LD_LIBRARY_PATH=... using $VAR notation
#
# Examples:
#   newage_export.sh                               # bash vars, no paths
#   newage_export.sh --bash --path                 # bash vars + PATH lines; sourceable
#   newage_export.sh --bash --path > newage.env.sh && source newage.env.sh
#   newage_export.sh --json --path                 # JSON with expanded path fields
#   newage_export.sh --powershell --path           # PowerShell with $env:Path mutation
#   newage_export.sh --env                         # dotenv format

set -euo pipefail

FORMAT="bash"
INCLUDE_PATH="0"

for arg in "$@"; do
    case "$arg" in
        --bash)       FORMAT="bash" ;;
        --powershell) FORMAT="powershell" ;;
        --json)       FORMAT="json" ;;
        --env)        FORMAT="env" ;;
        --path)       INCLUDE_PATH="1" ;;
        -h|--help)
            sed -n '2,/^set -/{ /^set -/d; s/^# \{0,1\}//; p }' "$0"
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg" >&2
            exit 1
            ;;
    esac
done

# --- Require all three core variables ---

_newage="${NewAge:-}"
_config="${NewAge_Config:-}"
_lane="${NewAge_Lane:-}"
_cc="${CC:-}"
_cxx="${CXX:-}"

_missing=""
[ -z "$_newage" ] && _missing="$_missing NewAge"
[ -z "$_config" ] && _missing="$_missing NewAge_Config"
[ -z "$_lane"   ] && _missing="$_missing NewAge_Lane"

if [ -n "$_missing" ]; then
    echo "ERROR: required variable(s) not set:$_missing" >&2
    echo "Establish the NewAge environment first:" >&2
    echo "  . newage_source_context.sh Debug" >&2
    echo "  ./in_this_context.sh Debug -- $0 $*" >&2
    exit 1
fi

# --- Helpers ---

_bash_quote() {
    printf '%s' "$1" | sed "s/'/'\\\\''/g; s/^/'/; s/$/'/"
}

_json_escape() {
    printf '%s' "$1" \
        | sed 's/\\/\\\\/g; s/"/\\"/g; s/	/\\t/g' \
        | awk '{printf "%s", $0}'
}

# --- Emit ---

case "$FORMAT" in

    bash)
        echo "export NewAge=$(_bash_quote "$_newage")"
        echo "export NewAge_Config=$(_bash_quote "$_config")"
        echo "export NewAge_Lane=$(_bash_quote "$_lane")"
        [ -n "$_cc"  ] && echo "export CC=$(_bash_quote "$_cc")"
        [ -n "$_cxx" ] && echo "export CXX=$(_bash_quote "$_cxx")"
        if [ "$INCLUDE_PATH" = "1" ]; then
            echo 'export PATH="$NewAge/bin/$NewAge_Config/$NewAge_Lane:$NewAge/bin:$PATH"'
            echo 'export LD_LIBRARY_PATH="$NewAge/lib/$NewAge_Config/$NewAge_Lane:${LD_LIBRARY_PATH:-}"'
        fi
        ;;

    powershell)
        echo "\$env:NewAge = \"$_newage\""
        echo "\$env:NewAge_Config = \"$_config\""
        echo "\$env:NewAge_Lane = \"$_lane\""
        [ -n "$_cc"  ] && echo "\$env:CC = \"$_cc\""
        [ -n "$_cxx" ] && echo "\$env:CXX = \"$_cxx\""
        if [ "$INCLUDE_PATH" = "1" ]; then
            echo "\$env:Path = \"\$env:NewAge\\lib\\\$env:NewAge_Config\\\$env:NewAge_Lane;\" + \$env:Path"
            echo "\$env:Path = \"\$env:NewAge\\bin\\\$env:NewAge_Config\\\$env:NewAge_Lane;\$env:NewAge\\bin;\" + \$env:Path"
        fi
        ;;

    json)
        _bin_path="$_newage/bin/$_config/$_lane:$_newage/bin"
        _lib_path="$_newage/lib/$_config/$_lane"
        entries=()
        entries+=("\"NewAge\": \"$(_json_escape "$_newage")\"")
        entries+=("\"NewAge_Config\": \"$(_json_escape "$_config")\"")
        entries+=("\"NewAge_Lane\": \"$(_json_escape "$_lane")\"")
        [ -n "$_cc"  ] && entries+=("\"CC\": \"$(_json_escape "$_cc")\"")
        [ -n "$_cxx" ] && entries+=("\"CXX\": \"$(_json_escape "$_cxx")\"")
        if [ "$INCLUDE_PATH" = "1" ]; then
            entries+=("\"NewAge_BinPath\": \"$(_json_escape "$_bin_path")\"")
            entries+=("\"NewAge_LibPath\": \"$(_json_escape "$_lib_path")\"")
        fi
        printf '{\n'
        for i in "${!entries[@]}"; do
            if [ "$i" -lt "$(( ${#entries[@]} - 1 ))" ]; then
                printf '  %s,\n' "${entries[$i]}"
            else
                printf '  %s\n' "${entries[$i]}"
            fi
        done
        printf '}\n'
        ;;

    env)
        echo "NewAge=$_newage"
        echo "NewAge_Config=$_config"
        echo "NewAge_Lane=$_lane"
        [ -n "$_cc"  ] && echo "CC=$_cc"
        [ -n "$_cxx" ] && echo "CXX=$_cxx"
        if [ "$INCLUDE_PATH" = "1" ]; then
            echo 'PATH=$NewAge/bin/$NewAge_Config/$NewAge_Lane:$NewAge/bin:$PATH'
            echo 'LD_LIBRARY_PATH=$NewAge/lib/$NewAge_Config/$NewAge_Lane:${LD_LIBRARY_PATH:-}'
        fi
        ;;
esac
