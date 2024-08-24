#!/bin/bash

pool="$1"
src="$(cygpath "$2")"
mod="$3"
pedigree="$(cygpath "$4")"

echo pool=\"$pool\" src=\"$src\" mod=\"$mod\" pedigree=\"$pedigree\"

if [ "$#" -gt 4 ]; then
    split_command="$(cygpath "$5")"
    echo "$split_command"
else
    split_command="split_arg"
fi

pwd

if [ -d "$pool" ]; then
    path="$(realpath "$pool")"
fi

acum="."

while [ $(realpath "$acum/..") != "/" ]; do
    acum="$acum/.."

    if [ -d "$acum/$pool" ]; then
        path="$(realpath "$acum/$pool")"

        if [ -f "$path/.anchor" ]; then
            break
        fi
    fi
done

echo "$path"
cd "$path"

${split_command} ${pedigree} |

while IFS= read -e line; do
    use="true"

    line=$(echo $line | grep "\S.")

    if [ -d $line ]; then
        use="false"
    fi

    if [ "$use" == "true" ]; then
        mkdir "$line"
    fi

    echo "******$line*********"
    cd "$line"
done

do_link(){
    file="$1"

    if [ -f "$src/$file" ]; then
        create_symlink.sh "$src/$file" "$path/$pedigree/$file"
    fi
}

do_link "$mod"
do_link "$mod.exe"
do_link "lib$mod.so"
do_link "$mod.lib"
do_link "$mod.dll"
do_link "$mod.pdb"

# echo $pool $src $pedigree $(realpath "$path") "$path"
