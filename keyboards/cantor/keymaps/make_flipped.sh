#!/bin/sh
set -ev
# You can plug the USB into either half if you flash the right half with a map
# that has all the keys mirrored along the vertical axis.
KEYMAP=$(basename "$1")
rm -rf "$1"_flipped
cp -r "$1" "$1"_flipped
rm "$1"_flipped/"$KEYMAP".json
$(dirname "$0")/json_flipper "$1"/"$KEYMAP".json > "$1"_flipped/"$KEYMAP"_flipped.json
