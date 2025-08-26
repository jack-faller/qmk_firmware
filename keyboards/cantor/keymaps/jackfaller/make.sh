#!/bin/sh
INCLUDE="$(find ~/qmk_firmware/quantum -name '*.h' -exec dirname {} \; | sort | uniq | sed s/^/-I/)"
gcc $CFLAGS -ggdb -Wall -Werror -std=c99 $INCLUDE -I$HOME/qmk_firmware keymap.c "$@"
