keyboards/cantor/keymaps/base/keymap.c: | keyboards/cantor/keymaps/base/configurator_keys.h
keyboards/cantor/keymaps/base/configurator_keys.h: keyboards/cantor/keymaps/base/configurator_keys.json
	qmk json2c -o $@ $<
