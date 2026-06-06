#!/bin/bash
# NewAge_forward.sh — stage a built executable into $NewAge/bin/ as a callable wrapper.
#
# Usage (called from a PostBuild target):
#   bash NewAge_forward.sh ProjectName DestinationDir/ OutputPath/
#
# Creates (DLL present — .NET tool, portable):
#   $DestinationDir/ProjectName      — bash wrapper; resolves DLL via BASH_SOURCE-relative path
#   $DestinationDir/ProjectName.bat  — CMD/PowerShell wrapper; resolves DLL via %~dp0-relative path
#
#   No symlink is created. The relative path to the DLL is computed once and embedded
#   in both wrappers. This survives cp -a into a collected workspace and works on
#   Windows where symlinks are not reliably available.
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
# Mirrors relative_path_from_to in newage_collect.sh.
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

        # Relative path from $NewAge/bin/ to the DLL.
        # Embedded directly in both wrappers — no symlink needed.
        # Survives cp -a into a collected workspace and works on Windows.
        local rel_dll
        rel_dll="$(relative_path_from_to "${MyReferencePath%/}" "$I_dll")"

        # Windows bat path: forward slashes → backslashes
        local rel_dll_win
        rel_dll_win="$(printf '%s' "$rel_dll" | sed 's|/|\\|g')"

        echo "  relative dll path: $rel_dll"

        # Bash wrapper — BASH_SOURCE gives the script's own directory at call time.
        # Works on Linux, Mac, and Git Bash on Windows.
        {
            echo '#!/bin/bash'
            echo 'SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"'
            echo "dotnet \"\$SCRIPT_DIR/${rel_dll}\" \"\$@\""
        } > "$O"
        chmod +x "$O"

        # Bat wrapper — %~dp0 expands to the script's directory with trailing backslash.
        # Works from CMD.exe and PowerShell without needing a bash prefix.
        local O_bat="${MyReferencePath}${name}.bat"
        if [ -f "$O_bat" ] || [ -L "$O_bat" ]; then
            verbose.sh rm "$O_bat"
        fi
        printf '@echo off\r\n'                              > "$O_bat"
        printf 'dotnet "%%~dp0%s" %%*\r\n' "$rel_dll_win" >> "$O_bat"

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
