#!/bin/bash

items=$(git status -s)

if [ -z "$items" ]; then
    echo clean
    exit
fi

echo dirty
