#!/usr/bin/env bash
set -euo pipefail

NEWAGE_CONFIGURE_SCOPE="newage_run_in_context"
NEWAGE_ARG=""
CONTEXT=""

usage() {
    cat <<EOF
Usage:
  newage_run_in_context.sh [--newage PATH] CONTEXT -- COMMAND [ARG...]

Context:
  NewAge, Base
      Use the base NewAge workspace environment.

  Debug
      Use the Debug native lane.

  Release
      Use the Release native lane.

Options:
  --newage PATH
      Use PATH as the NewAge workspace for this command.

Examples:
  newage_run_in_context.sh Debug -- bash -c 'echo "\$PATH"; split_arg a/b/c'

  newage_run_in_context.sh Release -- dotnet build Project/JWCCommandSpawn.net/ -c Release

  newage_run_in_context.sh --newage "\$HOME/NewAge" NewAge -- bash -c 'echo "\$NewAge"'

Notes:
  preferred execution is from the context's bin directory

  NewAge_context_path/bin/newage_run_in_context.sh --newage "\$HOME/NewAge" NewAge_context_path -- bash

EOF
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --newage)
            shift
            [ "$#" -gt 0 ] || {
                echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: --newage requires a path." >&2
                usage >&2
                exit 1
            }
            NEWAGE_ARG="$1"
            ;;

        -h|--help|help)
            usage
            exit 0
            ;;

        --)
            shift
            break
            ;;

        NewAge|newage|Base|base|Debug|debug|Release|release)
            if [ -n "$CONTEXT" ]; then
                echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: Context was already set: $CONTEXT" >&2
                usage >&2
                exit 1
            fi
            CONTEXT="$1"
            ;;

        --*)
            echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;

        *)
            echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: Unexpected argument before --: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
    shift
done

if [ -n "$NEWAGE_ARG" ]; then
    export NewAge="$NEWAGE_ARG"
fi

if [ -z "$CONTEXT" ]; then
    echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: CONTEXT is required." >&2
    usage >&2
    exit 1
fi

if [ "$#" -eq 0 ]; then
    echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: COMMAND is required after --." >&2
    usage >&2
    exit 1
fi

if [ -z "${NewAge:-}" ]; then
    echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: NewAge is not set." >&2
    echo "[$NEWAGE_CONFIGURE_SCOPE] Run JWCEssentials/configure.sh first." >&2
    echo "[$NEWAGE_CONFIGURE_SCOPE] See: https://github.com/johnwaynecornell/JWCEssentials" >&2
    exit 1
fi

if [ ! -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
    echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: JWCEssentials development helpers were not found." >&2
    echo "[$NEWAGE_CONFIGURE_SCOPE] Expected: $NewAge/JWCEssentials/Dev/NewAge.dev.sh" >&2
    echo "[$NEWAGE_CONFIGURE_SCOPE] Run JWCEssentials/configure.sh first." >&2
    echo "[$NEWAGE_CONFIGURE_SCOPE] See: https://github.com/johnwaynecornell/JWCEssentials" >&2
    exit 1
fi

. "$NewAge/JWCEssentials/Dev/NewAge.dev.sh"

NewAge="$(realpath "$(newage_to_unix_path "$NewAge")")"
export NewAge

cd "$NewAge"

newage_default_lane_for_context() {
    local context="$1"

    case "$context" in
        NewAge|newage|Base|base)
            printf '%s\n' ""
            ;;

        Debug|debug)
            if newage_is_windows_shell; then
                printf '%s\n' "Debug/Windows/AMD64/msvc"
            else
                printf '%s\n' "Debug/Linux/x86_64/gcc"
            fi
            ;;

        Release|release)
            if newage_is_windows_shell; then
                printf '%s\n' "Release/Windows/AMD64/msvc"
            else
                printf '%s\n' "Release/Linux/x86_64/gcc"
            fi
            ;;

        *)
            newage_fail "Unknown context: $context"
            return 1
            ;;
    esac
}

lane="$(newage_default_lane_for_context "$CONTEXT")"

export PATH="$NewAge/bin:$PATH"

if [ -n "$lane" ]; then
    export PATH="$NewAge/bin/$lane:$PATH"

    if newage_is_windows_shell; then
        export PATH="$NewAge/lib/$lane:$PATH"
    else
        export LD_LIBRARY_PATH="$NewAge/lib/$lane:${LD_LIBRARY_PATH:-}"
    fi
fi

exec "$@"
