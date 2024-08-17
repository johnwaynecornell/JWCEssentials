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

    sep=" "
    extra=""

    if [ $ahead_count != "0" ] || [ $behind_count != "0" ]; then
	status=""

        if [ $ahead_count != "0" ]; then
	    status="${status}ahead $ahead_count"
	    sep=", "	    
        fi

        if [ $behind_count != "0" ]; then
	    status="$status${sep}behind $behind_count"	    
        fi
        extra=".blink.underline.overline"

    else	
	status="current"

    fi

    feffect -e "'branch: ' fg_bright_black('[' fg_bright_yellow$extra('$remote_branch') ']' ) ', work tree $status'"
    #echo "branch: $(feffect "fg_red('[' fg_bright_yellow('$remote_branch') ']')"), work tree $status"
}

# Show branch information only with ahead/behind counts
branch_info=$(git status -b --porcelain | grep '##')
if [ -n "$branch_info" ]; then
    count_ahead_behind
else
    echo "No branch information found."
fi
