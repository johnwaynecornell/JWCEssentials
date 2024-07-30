#!/bin/bash


# Get the current remote tracking branch
current_remote=$(git branch -vv | grep -o "\[.*\]" | sed 's/^\[\([^:]*\).*\]$/\1/')

remote="$1"
branch=$(git branch | sed 's/\* //g')

shift

git branch --set-upstream-to="$remote/$branch" $branch > /dev/null
"$@"

git branch --set-upstream-to="$current_remote" $branch > /dev/null
