#!/bin/bash

supports_color() {
    if [ -t 1 ] || [ "$NO_COLOR" ]; then
        return 0
    else
        return 1
    fi
}


if [ supports_color ]; then
    if [ -z "${COLOR_GREEN+x}" ]; then
        source colors.sh
    fi


    echo -e "${COLOR_YELLOW}$@${COLOR_RESET}"
else
    echo "$@"
fi

"$@"

