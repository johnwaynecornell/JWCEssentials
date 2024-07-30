#!/bin/bash
if [ $(worktree_dirty.sh) == "dirty" ]; then
    "$@"
fi
