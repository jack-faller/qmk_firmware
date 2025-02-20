keyboards/cantor/keymaps/jackfaller/keymap.c: | keyboards/cantor/keymaps/jackfaller/jackfaller.h
keyboards/cantor/keymaps/jackfaller/jackfaller.h: keyboards/cantor/keymaps/jackfaller/jackfaller.json
	qmk json2c -o $@ $<
