#!/usr/bin/env bash
set -euo pipefail

supports_color() {
    if [ -t 1 ] && [ -z "${NO_COLOR:-}" ]; then
        return 0
    else
        return 1
    fi
}

if supports_color; then
    if [ -z "${COLOR_GREEN+x}" ]; then
        # Try to find colors.sh relative to the script
        if [ -f "$(dirname "${BASH_SOURCE[0]}")/colors.sh" ]; then
            source "$(dirname "${BASH_SOURCE[0]}")/colors.sh"
        elif [ -f "colors.sh" ]; then
            source colors.sh
        fi
    fi

    if [ -n "${COLOR_YELLOW:-}" ]; then
        echo -e "${COLOR_YELLOW}$*${COLOR_RESET}"
    else
        echo "$*"
    fi
else
    echo "$*"
fi

exec "$@"

