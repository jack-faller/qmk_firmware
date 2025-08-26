#!/bin/sh
set -ev
FLIPPER=$(dirname $(realpath "$0"))/json_flipper
LOCATION=$(realpath "$1")
KEYMAP=$(basename "$LOCATION")
cd $(dirname "$LOCATION")
# You can plug the USB into either half if you flash the right half with a map
# that has all the keys mirrored along the vertical axis.
rm -rf "$LOCATION"_flipped
cp -r "$LOCATION" "$LOCATION"_flipped
rm "$LOCATION"_flipped/"$KEYMAP".json
"$FLIPPER" "$LOCATION"/"$KEYMAP".json > "$LOCATION"_flipped/"$KEYMAP"_flipped.json
