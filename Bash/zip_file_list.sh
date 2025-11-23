#!/bin/bash

# 1. Detect Argument (Input Source)
INPUT_SOURCE="$1"

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
TARGET_LIST="$TEMP_DIR/filelist.txt"

if [ "$INPUT_SOURCE" == "-stdin" ]; then
    # CASE A: Read from Standard Input
    echo "📝 Reading file list from STDIN..."
    cat /dev/stdin > "$TARGET_LIST"

elif [ -n "$INPUT_SOURCE" ] && [ -f "$INPUT_SOURCE" ]; then
    # CASE B: Read from provided filename
    echo "📝 Using provided file list: $INPUT_SOURCE"
    # Copy content to temp list (stripping potential Windows CR chars just in case)
    tr -d '\r' < "$INPUT_SOURCE" > "$TARGET_LIST"

else
    # CASE C: Default (Git Auto-detection)
    echo "📝 Indexing files (excluding .zip files)..."
    echo "📝 Indexing files (excluding previous Snapshots)..."

    git ls-files --cached --exclude-standard | \
    grep -v '_Snapshot_.*_.*\.zip$' | \
    grep -v '\.temp_zip_staging' > "$TARGET_LIST"
fi

# Check if list is empty before proceeding
if [ ! -s "$TARGET_LIST" ]; then
    echo "⚠️  Warning: File list is empty. Nothing to zip."
    rm -rf "$TEMP_DIR"
    exit 1
fi

# 6. Copy Files to Staging
echo "📂 Staging files..."
while IFS= read -r file; do
    # Skip empty lines
    [ -z "$file" ] && continue

    # Only copy if file exists (handles rare edge cases or bad paths in custom lists)
    if [ -f "$file" ]; then
        mkdir -p "$TEMP_DIR/$(dirname "$file")"
        cp "$file" "$TEMP_DIR/$file"
    else
        echo "⚠️  Skipping missing file: $file"
    fi
done < "$TARGET_LIST"

# Remove the filelist so it's not in the final zip
rm "$TARGET_LIST"

# 7. Zip the Directory
echo "📦 Compressing..."

if [ "$OS" == "WINDOWS" ]; then
    # PowerShell Compression for Windows/Git Bash
    ABS_TEMP_DIR=$(cd "$TEMP_DIR" && pwd -W)
    ABS_ZIP_PATH=$(pwd -W)/$ZIP_NAME

    # We use -Force to overwrite if a file with the same name exists
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
