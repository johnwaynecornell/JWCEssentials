#!/usr/bin/env bash
set -euo pipefail

if [ -z "${NewAge:-}" ]; then
  echo "ERROR: \$NewAge environment variable is not set." >&2
  exit 1
fi

if [ ! -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
  echo "ERROR: Expected NewAge dev helpers at \$NewAge/JWCEssentials/Dev/NewAge.dev.sh" >&2
  exit 1
fi

. "$NewAge/JWCEssentials/Dev/NewAge.dev.sh"

if [ ! -f "CMakeLists.txt" ]; then
  echo "Expected run from repository root" >&2
  exit 1
fi

config=""
FRESH="0"
CLEAN="0"

usage() {
    cat <<EOF
Usage:
  $(basename "$0") [Debug|Release] [--fresh] [--clean]

Options:
  --fresh
      Remove CMake configure/cache files before configuring.

  --clean, --target-clean
      Run the generated build system's clean target before building.
EOF
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        -h|--help|help)
            usage
            exit 0
            ;;
        --fresh) FRESH="1" ;;
        --clean|--target-clean) CLEAN="1" ;;
        --*)
            echo "ERROR: Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
        *)
           if [ -z "$config" ]; then
                config="$1"
            else
                echo "ERROR: Unexpected argument: $1" >&2
                usage >&2
                exit 1
            fi
            ;;
    esac
    shift
done

if [ -z "$config" ]; then
    config="${NewAge_Config:-Debug}"
fi

export NewAge_Config="$config"
if [ -z "${NewAge_Lane:-}" ]; then
    export NewAge_Lane="$(newage_resolve_platform_lane)"
fi

export NEWAGE_BUILD_FRESH="$FRESH"
export NEWAGE_BUILD_CLEAN="$CLEAN"

# Bootstrap note:
# has_cs is declared by JWCEssentials as an optional composition with JWCCommandSpawn.
# Because this proof target lives inside JWCEssentials, it must ensure JWCCommandSpawn
# has been built before building Tools/spawn_bash_probe. Downstream repositories should
# not need this special case because dependency build order should already satisfy it.
newage_build_native.sh JWCCommandSpawn "$config"


# This script proves has_cs by building the JWCCommandSpawn bash probe.
newage_native_build_directory "Tools/spawn_bash_probe"
