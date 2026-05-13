#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -L)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd -L)"
REPO_NAME="$(basename "$REPO_ROOT")"
NEWAGE_ROOT="$(cd "$REPO_ROOT/.." && pwd -L)"

usage() {
    cat <<EOF
Usage:
  Workspace/JWCEssentials/Dev/in_this_context.sh CONTEXT -- COMMAND [ARG...]

Examples:
  DryRun/JWCEssentials/Dev/in_this_context.sh NewAge -- bash -c 'echo "\$NewAge"'
  NewAge/JWCEssentials/Dev/in_this_context.sh Debug -- bash -c 'echo "\$PATH"; split_arg a/b/c'
  MyNativeDepends/JWCEssentials/Dev/in_this_context.sh Release -- dotnet build Project/JWCEssentials.net/ -c Release

Notes:
  This helper infers NewAge from the repository location on logical path:

    repo root: $REPO_ROOT
    NewAge:    $NEWAGE_ROOT

  It then delegates to:

    \$NewAge/bin/newage_run_in_context.sh
EOF
}

if [ "$#" -eq 0 ]; then
    usage >&2
    exit 1
fi

export NewAge="$NEWAGE_ROOT"

RUNNER="$NewAge/bin/newage_run_in_context.sh"

if [ ! -x "$RUNNER" ]; then
    echo "[in_this_context] ERROR: NewAge context runner was not found or is not executable." >&2
    echo "[in_this_context] Expected: $RUNNER" >&2
    echo "[in_this_context] This helper must be called through the NewAge workspace path, for example:" >&2
    echo "[in_this_context]   \$NewAge/JWCEssentials/Dev/in_this_context.sh Debug -- COMMAND" >&2
    exit 1
fi

exec "$RUNNER" --newage "$NewAge" "$@"