#!/usr/bin/env bash
# newage_export.sh — emit current NewAge environment in a format suitable
# for IDE configuration, shell upload, or cross-environment transfer.
#
# Usage:
#   newage_export.sh [--bash|--powershell|--json|--env] [--path]
#
# Formats:
#   --bash        export statements (default); source directly in bash
#   --powershell  $env: assignments for PowerShell
#   --json        JSON object; suitable for IDE env config or API payloads
#   --env         KEY=VALUE dotenv format; works with direnv, Docker, etc.
#
# Options:
#   --path        also emit PATH / LD_LIBRARY_PATH additions as
#                 NewAge_BinPath and NewAge_LibPath variables

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
            cat <<'EOF'
Usage: newage_export.sh [--bash|--powershell|--json|--env] [--path]

Formats:
  --bash        export VAR=value  (default, sourceable in bash)
  --powershell  $env:VAR = "value"
  --json        {"VAR": "value", ...}
  --env         VAR=value  (dotenv / Docker --env-file)

Options:
  --path        include NewAge_BinPath and NewAge_LibPath in output

Examples:
  newage_export.sh                      # bash format, no path
  newage_export.sh --json --path        # JSON with path entries
  newage_export.sh --powershell         # PowerShell format
  newage_export.sh --bash > newage.env.sh && source newage.env.sh
EOF
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg" >&2
            exit 1
            ;;
    esac
done

# --- Collect values ---

_newage="${NewAge:-}"
_config="${NewAge_Config:-Debug}"
_lane="${NewAge_Lane:-}"
_cc="${CC:-}"
_cxx="${CXX:-}"

if [ -z "$_newage" ]; then
    echo "ERROR: NewAge is not set. Run from within a NewAge context." >&2
    exit 1
fi

# Path fragments (only computed when needed)
_bin_path=""
_lib_path=""
if [ "$INCLUDE_PATH" = "1" ]; then
    if [ -n "$_config" ] && [ -n "$_lane" ]; then
        _bin_path="${_newage}/bin/${_config}/${_lane}:${_newage}/bin"
        _lib_path="${_newage}/lib/${_config}/${_lane}"
    else
        _bin_path="${_newage}/bin"
        _lib_path=""
    fi
fi

# --- Helper: quote a value for bash export ---
_bash_quote() {
    printf '%s' "$1" | sed "s/'/'\\\\''/g; s/^/'/; s/$/'/"
}

# --- Helper: escape a value for JSON ---
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
        [ -n "$_lane"   ] && echo "export NewAge_Lane=$(_bash_quote "$_lane")"
        [ -n "$_cc"     ] && echo "export CC=$(_bash_quote "$_cc")"
        [ -n "$_cxx"    ] && echo "export CXX=$(_bash_quote "$_cxx")"
        if [ "$INCLUDE_PATH" = "1" ]; then
            # Use nested variable references — $NewAge is the only concrete value;
            # Config and Lane compose portably from the already-exported vars.
            echo 'export PATH="$NewAge/bin/$NewAge_Config/$NewAge_Lane:$NewAge/bin:$PATH"'
            echo 'export LD_LIBRARY_PATH="$NewAge/lib/$NewAge_Config/$NewAge_Lane:${LD_LIBRARY_PATH:-}"'
        fi
        ;;

    powershell)
        echo "\$env:NewAge = \"$_newage\""
        echo "\$env:NewAge_Config = \"$_config\""
        [ -n "$_lane"   ] && echo "\$env:NewAge_Lane = \"$_lane\""
        [ -n "$_cc"     ] && echo "\$env:CC = \"$_cc\""
        [ -n "$_cxx"    ] && echo "\$env:CXX = \"$_cxx\""
        if [ "$INCLUDE_PATH" = "1" ]; then
            # Reference the already-assigned $env: vars rather than baking in expanded paths.
            echo "\$env:Path = \"\$env:NewAge\\bin\\\$env:NewAge_Config\\\$env:NewAge_Lane;\$env:NewAge\\bin;\" + \$env:Path"
            echo "\$env:Path = \"\$env:NewAge\\lib\\\$env:NewAge_Config\\\$env:NewAge_Lane;\" + \$env:Path"
        fi
        ;;

    json)
        # JSON is consumed by structured tools; emit fully-expanded values so
        # consumers that don't support nested variable expansion still work.
        entries=()
        entries+=("\"NewAge\": \"$(_json_escape "$_newage")\"")
        entries+=("\"NewAge_Config\": \"$(_json_escape "$_config")\"")
        [ -n "$_lane"   ] && entries+=("\"NewAge_Lane\": \"$(_json_escape "$_lane")\"")
        [ -n "$_cc"     ] && entries+=("\"CC\": \"$(_json_escape "$_cc")\"")
        [ -n "$_cxx"    ] && entries+=("\"CXX\": \"$(_json_escape "$_cxx")\"")
        if [ "$INCLUDE_PATH" = "1" ]; then
            [ -n "$_bin_path" ] && entries+=("\"NewAge_BinPath\": \"$(_json_escape "$_bin_path")\"")
            [ -n "$_lib_path" ] && entries+=("\"NewAge_LibPath\": \"$(_json_escape "$_lib_path")\"")
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
        [ -n "$_lane"   ] && echo "NewAge_Lane=$_lane"
        [ -n "$_cc"     ] && echo "CC=$_cc"
        [ -n "$_cxx"    ] && echo "CXX=$_cxx"
        if [ "$INCLUDE_PATH" = "1" ]; then
            # dotenv consumers vary; emit deferred notation where $VAR is portable.
            echo 'PATH=$NewAge/bin/$NewAge_Config/$NewAge_Lane:$NewAge/bin:$PATH'
            echo 'LD_LIBRARY_PATH=$NewAge/lib/$NewAge_Config/$NewAge_Lane:$LD_LIBRARY_PATH'
        fi
        ;;
esac
