#!/usr/bin/env bash
set -euo pipefail

NEWAGE_CONFIGURE_SCOPE="newage_run_in_context"
NEWAGE_ARG=""
LANE_ARGS=()

usage() {
    cat <<EOF
Usage:
  newage_run_in_context.sh [--newage PATH] [CONFIG] [TOOLCHAIN] -- COMMAND [ARG...]

Options:
  --newage PATH
      Use PATH as the NewAge workspace for this command.

Examples:
  newage_run_in_context.sh Debug -- bash -c 'echo "\$PATH"; split_arg a/b/c'
  newage_run_in_context.sh Release -- dotnet build Project/JWCCommandSpawn.net/ -c Release
  newage_run_in_context.sh --newage "\$HOME/NewAge" Debug -- bash

Notes:
  This script is a wrapper around set_lane_environment from NewAge.dev.sh.
  It is functionally similar to in_this_context.sh but can be run from anywhere
  if it is on the PATH, and supports the --newage option.
EOF
}

# Parse arguments before --
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

        *)
            LANE_ARGS+=("$1")
            ;;
    esac
    shift
done

if [ -n "$NEWAGE_ARG" ]; then
    export NewAge="$NEWAGE_ARG"
fi

if [ -z "${NewAge:-}" ]; then
    echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: NewAge is not set." >&2
    echo "[$NEWAGE_CONFIGURE_SCOPE] Run JWCEssentials/configure.sh first." >&2
    exit 1
fi

if [ ! -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
    echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: JWCEssentials development helpers were not found." >&2
    echo "[$NEWAGE_CONFIGURE_SCOPE] Expected: $NewAge/JWCEssentials/Dev/NewAge.dev.sh" >&2
    exit 1
fi

. "$NewAge/JWCEssentials/Dev/NewAge.dev.sh"

NewAge="$(realpath "$(newage_to_unix_path "$NewAge")")"
export NewAge

cd "$NewAge"

if [ "$#" -eq 0 ]; then
    echo "[$NEWAGE_CONFIGURE_SCOPE] ERROR: COMMAND is required after --." >&2
    usage >&2
    exit 1
fi

# Apply the lane environment
set_lane_environment "${LANE_ARGS[@]}"

# Execute the command
exec "$@"
