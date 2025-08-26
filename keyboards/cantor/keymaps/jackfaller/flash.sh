#!/bin/sh
qmk flash -j 4 -kb cantor -km "$1" --bootloader dfu-util-split-left
sleep 5
qmk flash -j 4 -kb cantor -km "$1"_flipped --bootloader dfu-util-split-right
