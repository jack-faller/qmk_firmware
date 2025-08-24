keyboards/cantor/keymaps/jackfaller/keymap.c: | keyboards/cantor/keymaps/jackfaller/configurator_keys.h
keyboards/cantor/keymaps/jackfaller/configurator_keys.h: keyboards/cantor/keymaps/jackfaller/configurator_keys.json
	qmk json2c -o $@ $<
