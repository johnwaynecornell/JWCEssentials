#!/bin/bash


# Get the current remote tracking branch
#current_remote=$(git branch -vv | grep -o "\[.*\]" | sed 's/^\[\([^]]*\)\].*$/\1/')
current_remote=$(git rev-parse --abbrev-ref --symbolic-full-name @{u} 2>/dev/null)
if [ $? -ne 0 ]; then
    echo set the current remote for this script to work
    echo '     git branch --set-upstream-to="remote/branch"'
fi


echo The current remote \'$current_remote\'

# Loop through each remote
git remote | while IFS= read -r remote; do
    echo "Switching to $remote..."
    git branch --set-upstream-to="$remote/main"
    verbose.sh "$@"
done


# Revert back to the original remote tracking branch
echo "Reverting back to the original remote..."
git branch --set-upstream-to="$current_remote"
