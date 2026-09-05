#!/bin/bash

# Note: The target list ($NewAge/bin/gits.lst) can be generated and refreshed using newage_create_gits_lst.sh

if [ ! -f "$NewAge/bin/gits.lst" ]; then
  echo "Error: $NewAge/bin/gits.lst not found. Run newage_create_gits_lst.sh to populate it." >&2
  exit 1
fi

cat "$NewAge/bin/gits.lst" | xargs -i{} sh -c 'verbose.sh in_dir.sh "{}" "$@"' sh "$@"
