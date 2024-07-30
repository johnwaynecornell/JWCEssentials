#!/bin/bash

items=($(git status -u -s))

# Check the number of items in the array
if [ ${#items[@]} -eq 0 ]; then
    echo "clean"
else
    echo "dirty"
fi
