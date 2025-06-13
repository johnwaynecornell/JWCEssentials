#!/bin/bash

# Get the current remote tracking branch of the current branch
current_remote=$(git branch -vv | grep '^\*' | grep -o '\[.*\]' | sed 's/^\[\([^:]*\).*\]$/\1/')

remote="$1"
branch=$(git branch --show-current)

shift

# Temporarily set the upstream to new remote
git branch --set-upstream-to="$remote/$branch" "$branch" > /dev/null

# Run the command(s)
"$@"

# Restore original upstream
git branch --set-upstream-to="$current_remote" "$branch" > /dev/null
