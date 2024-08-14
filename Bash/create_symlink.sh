#!/bin/bash

# Function to create a symbolic link on Windows
create_symlink_windows() {
    local link="$(cygpath -w $1)"
    local target="$(cygpath -w $2)"

    # Check if it's a directory or file
    if [ -d "$target" ]; then
        cmd <<< "mklink /D \"$link\" \"$target\""
    else
        cmd <<< "mklink /H \"$link\" \"$target\""
    fi
}

# Function to create a symbolic link on Linux
create_symlink_linux() {
    local link="$1"
    local target="$2"

    ln -s "$target" "$link"
}

# Main function to detect platform and create the link
create_symlink() {
    local link="$1"
    local target="$2"

    case "$(uname -s)" in
        CYGWIN*|MINGW*|MSYS*)
            create_symlink_windows $(cygpath -w "$link") $(cygpath -w "$target")
            ;;
        *)
            create_symlink_linux "$link" "$target"
            ;;
    esac
}

# Usage: ./create_symlink.sh <link_name> <target_path>
# Example: ./create_symlink.sh mylink /path/to/target

# Check for the correct number of arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <target_path> <link_name>"
    exit 1
fi

# Call the main function
create_symlink "$2" "$1"
