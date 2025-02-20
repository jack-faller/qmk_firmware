keyboards/cantor/keymaps/base/keymap.c: | keyboards/cantor/keymaps/base/json_output.h
keyboards/cantor/keymaps/base/json_output.h: keyboards/cantor/keymaps/base/cantor_base.json
	qmk json2c -o $@ $<
