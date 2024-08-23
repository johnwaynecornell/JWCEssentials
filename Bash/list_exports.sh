#!/bin/bash

root=.

if [ "$#" -gt "0" ]; then
    root="$1"
fi


temp=$(rand_identifier)

find $root -name '*.h' -exec grep "_EXPORT_" {} \; > "$temp"
grep -v "#" "$temp"
rm "$temp"
