#!/bin/bash
# NewAge_forward.sh — stage a built executable into $NewAge/bin/ as a callable wrapper.
#
# Usage (called from a PostBuild target):
#   bash NewAge_forward.sh ProjectName DestinationDir/ OutputPath/
#
# Creates (DLL present — .NET tool):
#   $DestinationDir/ProjectName      — bash wrapper: dotnet "$NewAge/path/to/Project.dll" "$@"
#   $DestinationDir/ProjectName.bat  — CMD/PowerShell: dotnet "%NewAge%\path\to\Project.dll" %*
#
#   If the DLL is inside $NewAge, the path is $NewAge-relative (portable across collections;
#   in_this_context.sh sets $NewAge to the workspace root on entry).
#   If the DLL is outside $NewAge, the absolute path is used with a portability warning.
#
# Creates (no DLL — native binary):
#   $DestinationDir/ProjectName      — bash wrapper with absolute path to binary
#   $DestinationDir/ProjectName.bat  — CMD/PowerShell wrapper (Windows .exe builds only)

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

    local name="$1"
    local O="${MyReferencePath}${name}"
    local pth="$SourceDir"
    local src
    src="$(resolve_path.sh "$pth")"

    local I_dll="${src}/${name}.dll"
    local I_bin="${src}/${name}"
    [ ! -f "$I_bin" ] && I_bin="${src}/${name}.exe"

    echo "O=$O"
    echo "pth=$pth"
    echo "I_dll=$I_dll"
    echo "I_bin=$I_bin"

    # Remove existing bash wrapper and any stale .dll symlink/file from prior staging
    if [ -f "$O" ] || [ -L "$O" ]; then
        verbose.sh rm "$O"
    fi
    local O_dll_stale="${MyReferencePath}${name}.dll"
    if [ -f "$O_dll_stale" ] || [ -L "$O_dll_stale" ]; then
        verbose.sh rm "$O_dll_stale"
    fi

    if [ -f "$I_dll" ]; then
        echo "dll found — staging dotnet wrappers"

        # Determine the DLL path to embed in wrappers.
        # If the DLL is inside $NewAge, use $NewAge-relative path — portable across
        # collections since in_this_context.sh sets $NewAge to the workspace root.
        # If outside $NewAge, fall back to absolute path with a warning.
        local newage_canon
        newage_canon="$(cd "$NewAge" && pwd -P)"
        local dll_canon
        dll_canon="$(cd "$(dirname "$I_dll")" && pwd -P)/$(basename "$I_dll")"

        local dll_bash   # path string for bash wrapper (uses $NewAge variable)
        local dll_bat    # path string for bat wrapper  (uses %NewAge% variable)

        if [[ "$dll_canon" == "$newage_canon/"* ]]; then
            # Inside $NewAge — workspace-relative, portable
            local rel="${dll_canon#"$newage_canon/"}"
            local rel_win
            rel_win="$(printf '%s' "$rel" | sed 's|/|\\|g')"
            dll_bash="\$NewAge/${rel}"
            dll_bat="%NewAge%\\${rel_win}"
            echo "  path: \$NewAge/${rel}"
        else
            # Outside $NewAge — absolute path, warn
            local abs_win
            abs_win="$(cygpath -w "$I_dll" 2>/dev/null || echo "$I_dll")"
            dll_bash="$I_dll"
            dll_bat="$abs_win"
            echo "[NewAge_forward] WARNING: $name is outside \$NewAge — wrapper will not be portable." >&2
            echo "  path (absolute): $I_dll"
        fi

        # Bash wrapper — $NewAge is expanded at call time, not generation time
        {
            echo '#!/bin/bash'
            echo "dotnet \"${dll_bash}\" \"\$@\""
        } > "$O"
        chmod +x "$O"

        # Bat wrapper — %NewAge% is expanded at call time by CMD/PowerShell
        local O_bat="${MyReferencePath}${name}.bat"
        if [ -f "$O_bat" ] || [ -L "$O_bat" ]; then
            verbose.sh rm "$O_bat"
        fi
        printf '@echo off\r\n'                       > "$O_bat"
        printf 'dotnet "%s" %%*\r\n' "${dll_bat}"   >> "$O_bat"

        echo "$name dotnet wrappers staged"

    elif [ -f "$I_bin" ]; then
        echo "binary found — staging binary wrapper"

        echo "#!/bin/bash"         > "$O"
        echo "\"$I_bin\" \"\$@\"" >> "$O"
        chmod +110 "$O"

        if [[ "$I_bin" == *.exe ]]; then
            local O_bat="${MyReferencePath}${name}.bat"
            local I_win
            I_win="$(cygpath -w "$I_bin")"
            if [ -f "$O_bat" ] || [ -L "$O_bat" ]; then
                verbose.sh rm "$O_bat"
            fi
            printf '@echo off\r\n'           > "$O_bat"
            printf '"%s" %%*\r\n' "$I_win"  >> "$O_bat"
            echo "$name.bat staged"
        fi

    else
        echo "WARNING: no DLL or binary found for $name" >&2
    fi

    echo _______________________________
}

try_link "$Project"

echo "$Project staged"
