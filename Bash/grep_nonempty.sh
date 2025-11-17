#!/usr/bin/env bash

# Usage:
#   grep_nonempty.sh PATTERN file1 [file2 ...]
#
# Prints:
#   filename
#   matching lines
#
# Only if the grep output is *non-empty*.

pattern="$1"
shift

for file in "$@"; do
    # Capture grep output safely (no color, no filename prefix)
    matches=$(grep -Hn -- "$pattern" "$file" 2>/dev/null)

    if [[ -n "$matches" ]]; then
        echo -e "\e[36m=== $file ===\e[0m"
        echo "$matches"
        echo
    fi
done
