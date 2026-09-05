#!/bin/bash

if [ -z "$NewAge" ]; then
  echo "Error: \$NewAge environment variable is not set." >&2
  exit 1
fi

# Ensure the target bin directory exists
mkdir -p "$NewAge/bin"

# Find .git directories, prune them from further traversal for performance, and extract the parent path
find "$NewAge" -name ".git" -type d -prune -exec dirname {} \; > "$NewAge/bin/gits.lst"

echo "Saved $(wc -l < "$NewAge/bin/gits.lst") repositories to $NewAge/bin/gits.lst"
