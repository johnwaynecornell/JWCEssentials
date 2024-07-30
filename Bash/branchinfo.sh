#!/bin/bash

# Function to count commits ahead and behind
count_ahead_behind() {
    local_branch=$(git rev-parse --abbrev-ref HEAD)
    remote_branch=$(git rev-parse --abbrev-ref @{u} 2>/dev/null)

    if [ $? -ne 0 ]; then
        echo "No remote tracking branch found for the current branch."
        return
    fi

    ahead_count=$(git rev-list --count ${remote_branch}..${local_branch})
    behind_count=$(git rev-list --count ${local_branch}..${remote_branch})

    echo "Branch: ${local_branch} (ahead ${ahead_count}, behind ${behind_count})"
}

# Show branch information only with ahead/behind counts
branch_info=$(git status -b --porcelain | grep '##')
if [ -n "$branch_info" ]; then
    count_ahead_behind
else
    echo "No branch information found."
fi
