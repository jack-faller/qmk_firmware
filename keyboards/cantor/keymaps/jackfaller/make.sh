#!/bin/sh
INCLUDE="$(find ~/qmk_firmware/quantum -name '*.h' -exec dirname {} \; | sort | uniq | sed s/^/-I/)"
gcc $INCLUDE -I$HOME/qmk_firmware keymap.c "$@"
