#!/bin/bash
# NewAge_forward.sh — stage a built executable into $NewAge/bin/ as a callable wrapper.
#
# Usage (called from a PostBuild target):
#   bash NewAge_forward.sh ProjectName DestinationDir/ OutputPath/
#
# Creates:
#   $DestinationDir/ProjectName      — bash wrapper (Linux / Git Bash)
#   $DestinationDir/ProjectName.bat  — CMD/PowerShell wrapper (Windows builds only;
#                                       emitted when the binary is a .exe)

Project="$1"
MyReferencePath="$(cygpath $2)"
SourceDir="$(cygpath $3)"

if [ ! -d "$MyReferencePath" ]; then
    mkdir -p "$MyReferencePath"
    if [ $? -ne 0 ]; then
        echo "ERROR: directory \"$MyReferencePath\" doesn't exist and couldn't be created" 1>&2
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

    # Remove existing bash wrapper
    if [ -f "$O" ] || [ -L "$O" ]; then
        verbose.sh rm "$O"
    fi

    if [ -f "$I" ]; then
        echo input exists

        # Bash wrapper — works on Linux and from Git Bash on Windows
        echo "#!/bin/bash" > "$O"
        echo "\"$I\" \"\$@\"" >> "$O"
        chmod +110 "$O"

        # Bat wrapper — emitted only when the binary is a .exe (Windows build).
        # Allows the tool to be called from CMD.exe and PowerShell without a shell prefix.
        if [[ "$I" == *.exe ]]; then
            O_bat="${MyReferencePath}${1}.bat"
            I_win="$(cygpath -w "$I")"

            if [ -f "$O_bat" ] || [ -L "$O_bat" ]; then
                verbose.sh rm "$O_bat"
            fi

            printf '@echo off\r\n' > "$O_bat"
            printf '"%s" %%*\r\n' "$I_win" >> "$O_bat"

            echo "$1.bat staged"
        fi
    fi

    echo _______________________________
}

try_link "$Project"

echo $Project staged
