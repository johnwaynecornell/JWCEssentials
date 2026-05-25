#!/bin/bash

Project="$1"
MyReferencePath="$(cygpath $2)"
SourceDir="$(cygpath $3)"

if [ ! -d "$MyReferencePath" ]; then
    mkdir -p "$MyReferencePath"
    if [  $? -ne 0 ]; then
        echo ERROR directory \"$MyReferencePath\" doesn\'t exist and couldn\'t be created 1>&2
        exit -1
    fi

fi

try_link() {
    echo _______________________________

    O="$MyReferencePath$1"
    pth="$SourceDir"
    I="$(resolve_path.sh $pth)/$1"

    
    if [ ! -f "$I" ]; then
        I="$(resolve_path.sh $pth)/$1.exe"
    fi

    echo "O=$O"
    echo "pth=$pth"
    echo "I=$I"


    # Check if the output path already exists as a file or symbolic link and remove it if necessary
    if [ -f "$O" ] || [ -L "$O" ]; then
        verbose.sh rm "$O"
    fi

    # Check if the input path exists and create a symbolic link
    if [ -f "$I" ]; then
	echo input exists
        
        echo "#!/bin/bash" > "$O"
        echo "\"$I\" \"\$@\"" >> "$O"
	chmod +110 "$O"
    fi

    echo _______________________________
}

try_link "$Project"

echo $Project staged

