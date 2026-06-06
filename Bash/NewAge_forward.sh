#!/bin/bash
# NewAge_forward.sh — stage a built executable into $NewAge/bin/ as a callable wrapper.
#
# Usage (called from a PostBuild target):
#   bash NewAge_forward.sh ProjectName DestinationDir/ OutputPath/
#
# Creates (DLL present — .NET tool, portable):
#   $DestinationDir/ProjectName      — bash wrapper using dotnet + script-relative DLL path
#   $DestinationDir/ProjectName.bat  — CMD/PowerShell wrapper using dotnet + %~dp0-relative path
#   $DestinationDir/ProjectName.dll  — relative symlink to the DLL
#
#   The DLL symlink is relative (e.g. ../JWCEssentials/.../Project.dll) so that
#   cp -a into a collected workspace preserves it and it resolves correctly —
#   newage_collect.sh copies managed bins under the same repo-relative paths.
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

# Compute the relative path from base directory to target path.
# Mirrors relative_path_from_to in newage_collect.sh — keeping them in sync
# ensures collect and forward agree on the same relative layout.
relative_path_from_to() {
    local base="$1"
    local path="$2"

    if command -v realpath >/dev/null 2>&1; then
        realpath --relative-to="$base" "$path"
        return
    fi

    # Fallback: strip base prefix (assumes path starts with base).
    path="${path#"$base"/}"
    printf '%s\n' "$path"
}

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

    # Remove existing bash wrapper
    if [ -f "$O" ] || [ -L "$O" ]; then
        verbose.sh rm "$O"
    fi

    if [ -f "$I_dll" ]; then
        echo "dll found — staging dotnet wrappers"

        # Relative symlink: $NewAge/bin/Project.dll -> ../Repo/Project/.../Project.dll
        # Preserved correctly by cp -a during collection; resolves in the collected
        # workspace because collect copies managed bins under the same relative paths.
        local O_dll="${MyReferencePath}${name}.dll"
        if [ -f "$O_dll" ] || [ -L "$O_dll" ]; then
            verbose.sh rm "$O_dll"
        fi
        local rel_dll
        rel_dll="$(relative_path_from_to "${MyReferencePath%/}" "$I_dll")"
        ln -s "$rel_dll" "$O_dll"
        echo "  symlink: $name.dll -> $rel_dll"

        # Bash wrapper — resolves DLL relative to the script's own directory.
        # Works on Linux, Mac, and Git Bash on Windows.
        {
            echo '#!/bin/bash'
            echo 'SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"'
            echo "dotnet \"\$SCRIPT_DIR/${name}.dll\" \"\$@\""
        } > "$O"
        chmod +x "$O"

        # Bat wrapper — %~dp0 expands to the script's directory with trailing backslash.
        # Works from CMD.exe and PowerShell without needing a bash prefix.
        local O_bat="${MyReferencePath}${name}.bat"
        if [ -f "$O_bat" ] || [ -L "$O_bat" ]; then
            verbose.sh rm "$O_bat"
        fi
        printf '@echo off\r\n'                         > "$O_bat"
        printf 'dotnet "%%~dp0%s.dll" %%*\r\n' "$name" >> "$O_bat"

        echo "$name dotnet wrappers staged"

    elif [ -f "$I_bin" ]; then
        echo "binary found — staging binary wrapper"

        # Bash wrapper — absolute path; binary is platform-specific
        echo "#!/bin/bash"         > "$O"
        echo "\"$I_bin\" \"\$@\"" >> "$O"
        chmod +110 "$O"

        # Bat wrapper for Windows .exe builds
        if [[ "$I_bin" == *.exe ]]; then
            local O_bat="${MyReferencePath}${name}.bat"
            local I_win
            I_win="$(cygpath -w "$I_bin")"
            if [ -f "$O_bat" ] || [ -L "$O_bat" ]; then
                verbose.sh rm "$O_bat"
            fi
            printf '@echo off\r\n'        > "$O_bat"
            printf '"%s" %%*\r\n' "$I_win" >> "$O_bat"
            echo "$name.bat staged"
        fi

    else
        echo "WARNING: no DLL or binary found for $name" >&2
    fi

    echo _______________________________
}

try_link "$Project"

echo "$Project staged"
