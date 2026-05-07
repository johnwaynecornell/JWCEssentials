#!/bin/bash

Project="$1"
MyReferencePath="$(cygpath "$2")"
SourceDir="$3"

if [ ! -d "$MyReferencePath" ]; then
    mkdir -p "$MyReferencePath"
    if [  $? -ne 0 ]; then
        echo ERROR directory \"$MyReferencePath\" doesn\'t exist and couldn\'t be created 1>&2
        exit -1
    fi

fi

try_link() {
    echo _______________________________

    O="$MyReferencePath/$1"
    pth="$SourceDir"
    I="$(resolve_path.sh $pth)/$1"

    
    echo "O=$O"
    echo "pth=$pth"
    echo "I=$I"

    pwd


    # Check if the output path already exists as a file or symbolic link and remove it if necessary
    if [ -f "$O" ] || [ -L "$O" ]; then
        verbose.sh rm "$O"
    fi

    # Check if the input path exists and create a symbolic link
    if [ -f "$I" ]; then
	echo input exists
        verbose.sh create_symlink.sh "$I" "$O"
    fi

    echo _______________________________
}

try_link "$Project.exe"
try_link "$Project.dll"
try_link "$Project.pdb"
try_link "$Project.deps.json"

echo $Project staged

