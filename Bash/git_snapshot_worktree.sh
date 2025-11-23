#!/bin/bash

# 1. Locate Git Root and Jump there
#    This ensures the script works even if you run it from a subfolder.
ROOT_DIR=$(git rev-parse --show-toplevel 2>/dev/null)

if [ -z "$ROOT_DIR" ]; then
    echo "❌ Error: You are not inside a Git repository."
    exit 1
fi

cd "$ROOT_DIR" || exit
echo "📍 Working from repository root: $PWD"

# 2. Detect OS
OS_TYPE="$(uname -s)"
case "${OS_TYPE}" in
    CYGWIN*|MINGW*|MSYS*) OS="WINDOWS" ;;
    Darwin*)              OS="MAC" ;;
    *)                    OS="LINUX" ;;
esac

# 3. Define Output Name and Temp Directory
PROJECT_NAME=$(basename "$PWD")
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
ZIP_NAME="${PROJECT_NAME}_Snapshot_${TIMESTAMP}.zip"
TEMP_DIR="./.temp_zip_staging"

# 4. Clean/Create Temp Directory
rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR"

# 5. Generate File List
#    - git ls-files: grabs tracked AND untracked files
#    - exclude-standard: respects .gitignore
#    - grep -v: Excludes only files matching the "_Snapshot_....zip" pattern
echo "📝 Indexing files (excluding .zip files)..."
echo "📝 Indexing files (excluding previous Snapshots)..."
git ls-files --cached --exclude-standard | grep -v '_Snapshot_.*_.*\.zip$' | grep -v '\.temp_zip_staging' > "$TEMP_DIR/filelist.txt"

# 6. Copy Files to Staging
echo "📂 Staging files..."
while IFS= read -r file; do
    # Only copy if file exists (handles rare edge cases with deleted-but-staged files)
    if [ -f "$file" ]; then
        mkdir -p "$TEMP_DIR/$(dirname "$file")"
        cp "$file" "$TEMP_DIR/$file"
    fi
done < "$TEMP_DIR/filelist.txt"

# Remove the filelist so it's not in the final zip
rm "$TEMP_DIR/filelist.txt"

# 7. Zip the Directory
echo "📦 Compressing..."

if [ "$OS" == "WINDOWS" ]; then
    # PowerShell Compression for Windows/Git Bash
    ABS_TEMP_DIR=$(cd "$TEMP_DIR" && pwd -W)
    ABS_ZIP_PATH=$(pwd -W)/$ZIP_NAME

    # We use -Force to overwrite if a file with the same name exists (unlikely due to timestamp)
    powershell -Command "Compress-Archive -Path '$ABS_TEMP_DIR\*' -DestinationPath '$ABS_ZIP_PATH' -Force"
else
    # Standard Zip for Linux/Mac
    (cd "$TEMP_DIR" && zip -r -q "../$ZIP_NAME" .)
fi

# 8. Cleanup
rm -rf "$TEMP_DIR"

if [ -f "$ZIP_NAME" ]; then
    echo "✅ Snapshot Created: $ZIP_NAME"
else
    echo "❌ Error: Zip file creation failed."
fi
