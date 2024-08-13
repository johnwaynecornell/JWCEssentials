#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 [-filelist | file_to_be_searched] grep_arguments ..."
    exit 1
fi

do_search() {
    # Capture the output of grep, including any potential line breaks
    output=$(grep "$@")

    # Check if output is non-empty
    if [[ -n "$output" ]]; then
        while [ $# -gt 1 ]; do
            shift
        done

        echo "$1"
    fi
}

if [ "$1" == "-filelist" ]; then
    shift
    while IFS= read -r file; do
        do_search "$@" "$file"
    done 
    exit 0
fi

file=$1
shift
do_search "$@" $file

