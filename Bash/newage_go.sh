#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<EOF
Usage:
  $(basename "$0") <target-dir> [RepoName...] [--build [build args...]]

Bootstrap or update a NewAge workspace.

Arguments:
  <target-dir>
      Directory to use as the NewAge workspace.

  RepoName
      Repository name under johnwaynecornell.
      Each RepoName expands to:
        newage_get.sh RepoName https://github.com/johnwaynecornell/RepoName

Options:
  --build [build args...]
      After get/configure, run:
        newage_all_build_coordinated.sh [build args...]

      If no build args are provided, defaults to:
        Debug

Examples:
  $(basename "$0") Home.NewAge JWCCommandSpawn
  $(basename "$0") Home.NewAge JWCCommandSpawn CrystalCatalystLibrary --build
  $(basename "$0") Home.NewAge JWCCommandSpawn CrystalCatalystLibrary --build Release
  $(basename "$0") "\$NewAge" JWCCommandSpawn --build Debug

Notes:
  This script uses <target-dir> as the workspace root.
  It does not switch behavior based on the current \$NewAge environment variable.
EOF
}

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ] || [ "$#" -lt 1 ]; then
    usage
    exit 0
fi

target_dir="$1"
shift

build="0"
repos=()
build_args=()

while [ "$#" -gt 0 ]; do
    case "$1" in
        --build)
            build="1"
            shift
            build_args=("$@")
            break
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --*)
            echo "[newage_go] ERROR: Unknown option before --build: $1" >&2
            usage >&2
            exit 1
            ;;
        *)
            repos+=("$1")
            shift
            ;;
    esac
done

if [ "$build" = "1" ] && [ "${#build_args[@]}" -eq 0 ]; then
    build_args=("Debug")
fi

mkdir -p "$target_dir"
target_dir="$(cd "$target_dir" && pwd)"

echo "[newage_go] Workspace: $target_dir"

if [ ! -d "$target_dir/JWCEssentials/.git" ]; then
    echo "[newage_go] Cloning JWCEssentials..."
    git clone https://github.com/johnwaynecornell/JWCEssentials "$target_dir/JWCEssentials"
else
    echo "[newage_go] JWCEssentials already present. Skipping clone."
fi

echo "[newage_go] Configuring workspace..."
"$target_dir/JWCEssentials/configure.sh" --newage "$target_dir"

ctx_args=("Debug")
if [[ -n "${NewAge_Lane:-}" ]]; then
    ctx_args+=("$NewAge_Lane")
fi

context_script="$target_dir/in_this_context.sh"
if [ ! -x "$context_script" ] && [ ! -f "$context_script" ]; then
    echo "[newage_go] ERROR: Context script not found: $context_script" >&2
    exit 1
fi

echo "[newage_go] Running in NewAge context..."

if [ "${#repos[@]}" -gt 0 ]; then
    "$context_script" "${ctx_args[@]}" -- bash -e -c '
        build="$1"
        repo_count="$2"
        shift 2

        repos=()
        for ((i = 0; i < repo_count; i++)); do
            repos+=("$1")
            shift
        done

        build_args=("$@")

        for repo in "${repos[@]}"; do
            echo "[newage_go] Getting repo: $repo"
            newage_get.sh "$repo" "https://github.com/johnwaynecornell/$repo"
        done

        newage_get_deps.sh
        newage_all_configure.sh

        if [ "$build" = "1" ]; then
            echo "[newage_go] Building coordinated: ${build_args[*]}"
            newage_all_build_coordinated.sh "${build_args[@]}"
        fi
    ' bash "$build" "${#repos[@]}" "${repos[@]}" "${build_args[@]}"
else
    "$context_script" "${ctx_args[@]}" -- bash -e -c '
        build="$1"
        shift

        build_args=("$@")

        newage_get_deps.sh
        newage_all_configure.sh

        if [ "$build" = "1" ]; then
            echo "[newage_go] Building coordinated: ${build_args[*]}"
            newage_all_build_coordinated.sh "${build_args[@]}"
        fi
    ' bash "$build" "${build_args[@]}"
fi

echo "[newage_go] Complete."