#!/bin/bash

# Extract current git info
GIT_USER=$(git config user.name)
GIT_EMAIL=$(git config user.email)

echo "# Run the following commands in your VM to set your Git identity:"
echo "---------------------------------------------------------------"
echo "git config --global user.name \"$GIT_USER\""
echo "git config --global user.email \"$GIT_EMAIL\""
echo ""
echo "# Optional: Set a short credential cache timeout (1 hour)"
echo "git config --global credential.helper 'cache --timeout=3600'"
echo "---------------------------------------------------------------"