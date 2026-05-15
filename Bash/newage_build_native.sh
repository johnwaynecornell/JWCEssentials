#!/usr/bin/env bash
set -euo pipefail


usage() {
    cat <<EOF
Usage:
  ./newage_build_native.sh REPO_DIR [Debug|Release|Both|All] [--fresh] [--clean]

Examples:
  ./newage_build_native.sh JWCEssentials Debug
  ./newage_build_native.sh JWCCommandSpawn Release
  ./newage_build_native.sh CrystalCatalystLibrary Both --fresh
  ./newage_build_native.sh JWCCommandSpawn All --fresh --clean

Arguments:
  REPO_DIR
      Repo to build natives for.

Build mode:
  Debug
      Build Debug only. This is the default.

  Release
      Build Release only.

  Both, All
      Build Debug and Release.

Options:
  
  --fresh
      Remove CMake configure/cache files before configuring each native build.

  --clean, --target-clean
      Run the generated build system's clean target before building.

Notes:
  natives from REPO_DIR will be created.
EOF
}

REPO_DIR=""
BUILD_MODE="Debug"
FRESH="0"
CLEAN="0"

while [ "$#" -gt 0 ]; do
    case "$1" in
        -h|--help|help)
            usage
            exit 0
            ;;

        --fresh)
            FRESH="1"
            ;;

        --clean|--target-clean)
            CLEAN="1"
            ;;

        Debug|debug|Release|release|Both|both|All|all)
            BUILD_MODE="$1"
            ;;

        --*)
            echo "[newage_build_native] ERROR: Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;

        *)
            if [ -z "$REPO_DIR" ]; then
                REPO_DIR="$1"
            else
                echo "[newage_build_native] ERROR: Unexpected argument: $1" >&2
                usage >&2
                exit 1
            fi
            ;;
    esac
    shift
done

if [ -z "$REPO_DIR" ]; then
    echo "[newage_build_native] ERROR: REPO_DIR is required." >&2
    usage >&2
    exit 1
fi

case "$BUILD_MODE" in
    Debug|debug)
        BUILD_CONFIGS=("Debug")
        ;;
    Release|release)
        BUILD_CONFIGS=("Release")
        ;;
    Both|both|All|all)
        BUILD_CONFIGS=("Debug" "Release")

        # In-place CMake builds can preserve stale cache/configuration state
        # across Debug/Release transitions, so make Both/All safe by default.
        FRESH="1"
        ;;
    *)
        echo "[newage_build_native] ERROR: Unknown build mode: $BUILD_MODE" >&2
        usage >&2
        exit 1
        ;;
esac

if [ ! -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
  echo "Expected run from NewAge workspace root" >&2
  return 1
fi

. $NewAge/JWCEssentials/Dev/NewAge.dev.sh

for config in "${BUILD_CONFIGS[@]}"; do
    echo
    echo "[newage_build_native] Building configuration: $config"
    echo

    set_lane_environment "$config"
    
    cd "$NewAge/$REPO_DIR"

    EXTRA_ARGS=()
    [ "$FRESH" = "1" ] && EXTRA_ARGS+=(--fresh)
    [ "$CLEAN" = "1" ] && EXTRA_ARGS+=(--clean)
    
    Dev/build_native.sh "$config" "${EXTRA_ARGS[@]}"
done

echo
echo "[newage_build_native] Complete."
echo
echo "NewAge:"
echo "  $NewAge"
echo
echo "Build configurations:"
for config in "${BUILD_CONFIGS[@]}"; do
    echo "  $config"
done