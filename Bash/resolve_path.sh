#!/bin/bash

# Function to get the fully resolved path
get_resolved_path() {
    local path="$1"
    cd "$path" || exit 1  # Change directory to the given path
    pwd                    # Print the fully resolved path
}

# Usage: ./get_path.sh /path/to/directory
# Example: ./get_path.sh /c/Users/username

# Check for the correct number of arguments
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path>"
    exit 1
fi

# Get the fully resolved path
echo $(get_resolved_path "$1")
