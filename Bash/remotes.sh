#!/bin/bash


# Get the current remote tracking branch
#current_remote=$(git branch -vv | grep -o "\[.*\]" | sed 's/^\[\([^]]*\)\].*$/\1/')
current_remote=$(git rev-parse --abbrev-ref --symbolic-full-name @{u} 2>/dev/null)
if [ $? -ne 0 ]; then
    echo set the current remote for this script to work
    echo '     git branch --set-upstream-to="remote/branch"'
fi

# Loop through each remote
git remote | while IFS= read -r remote; do
    on_remote.sh $remote "$@"
done

git branch --set-upstream-to="$current_remote" > /dev/null
