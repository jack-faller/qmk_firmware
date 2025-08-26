thisdir := $(dir $(lastword $(MAKEFILE_LIST)))
keymap := $(notdir $(abspath $(thisdir)))
$(thisdir)keymap.c: | $(thisdir)configurator_keys.h
$(thisdir)configurator_keys.h: $(thisdir)$(keymap).json
	qmk json2c -o $@ $<

KEY_LOCK_ENABLE = yes
