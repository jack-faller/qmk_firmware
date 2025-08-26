#ifndef QMK_KEYBOARD_H
#include "fake-qmk.h"
#include <stdio.h>
#define FAKE_HARDWARE
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define eprintf(...)
#endif

#define LENGTH(A) (sizeof(A) / sizeof(A[0]))

#include "quantum_keycodes.h"
enum {
	_IGNORED = SAFE_RANGE,
	LOCK_START,
	LALT_LOCK = LOCK_START,
	LCTL_LOCK,
	LSFT_LOCK,
	LGUI_LOCK,
	PSCR_LOCK,
	LOCK_RELEASE,
};
static uint8_t lock_keys[] = {
	KC_LALT, KC_LGUI, KC_LCTL, KC_LSFT, KC_PSCR,
};
static bool is_lock(uint16_t code) {
	return LOCK_START <= code && code <= LOCK_START + LENGTH(lock_keys);
}
static uint16_t lock_code(uint16_t code) {
	return lock_keys[code - LOCK_START];
}

#include "configurator_keys.h"

static bool is_mod_tap(uint16_t code) {
	return QK_MOD_TAP <= code && code <= QK_MOD_TAP_MAX;
}
static bool is_layer_tap(uint16_t code) {
	return QK_LAYER_TAP <= code && code <= QK_LAYER_TAP_MAX;
}
static bool is_dual(uint16_t code) {
	return is_mod_tap(code) || is_layer_tap(code);
}
static uint16_t dual_primary(uint16_t code) {
	if (is_mod_tap(code))
		return QK_MOD_TAP_GET_TAP_KEYCODE(code);
	else if (is_layer_tap(code))
		return QK_LAYER_TAP_GET_TAP_KEYCODE(code);
	else
		return code;
}
static uint16_t dual_secondary(uint16_t code) {
	if (is_mod_tap(code))
		switch (QK_MOD_TAP_GET_MODS(code)) {
		case MOD_LGUI: return KC_LGUI;
		case MOD_LALT: return KC_LALT;
		case MOD_LSFT: return KC_LSFT;
		case MOD_LCTL: return KC_LCTL;
		case MOD_RGUI: return KC_RGUI;
		case MOD_RALT: return KC_RALT;
		case MOD_RSFT: return KC_RSFT;
		case MOD_RCTL: return KC_RCTL;
		default: return KC_NO;
		}
	else if (is_layer_tap(code))
		return MO(QK_LAYER_TAP_GET_LAYER(code));
	else
		return code;
}

#define BITSET(name, len) uint8_t name[((len) + 7) / 8]
static bool bitset_get(uint8_t *bitset, int i) {
	return (bitset[i / 8] & (1 << (i % 8))) != 0;
}
static void bitset_set(uint8_t *bitset, int i, bool value) {
	int index = i / 8, subindex = i % 8;
	uint8_t old_val = bitset[index];
	bool old = old_val & 1 << subindex;
	bitset[index] = old_val ^ ((old == 0) != (value == 0)) << subindex;
}

typedef uint8_t keynum;

#define KEY_COUNT (MATRIX_ROWS * MATRIX_COLS)
static struct {
	keynum keys[KEY_COUNT];
	BITSET(states, KEY_COUNT);
	// Assume the queue can never be full.
	uint8_t count[MATRIX_ROWS * MATRIX_COLS];
	uint8_t front, back;
} queue;

static uint16_t get_code(keynum key) {
	uint16_t out = KC_TRNS;
	for (int highest = get_highest_layer(layer_state);
	     out == KC_TRNS && highest >= 0;
	     --highest) {
		if (IS_LAYER_ON(highest)) {
			out = keymaps[highest][key / MATRIX_ROWS][key % MATRIX_ROWS];
		}
	}
	return out;
}
static keynum to_keynum(keypos_t keypos) {
	return keypos.col + keypos.row * MATRIX_ROWS;
}

static bool queue_empty(void) { return queue.front == queue.back; }
static uint8_t queue_next_index(uint8_t i) {
	return (i + 1) % LENGTH(queue.keys);
}
static bool front_pressed(void) {
	return bitset_get(queue.states, queue.front);
}
static keynum front_key(void) { return queue.keys[queue.front]; }

static void enqueue(keynum key, bool pressed) {
	++queue.count[key];
	queue.keys[queue.back] = key;
	bitset_set(queue.states, queue.back, pressed);
	queue.back = queue_next_index(queue.back);
}
static void dequeue(void) {
	--queue.count[queue.keys[queue.front]];
	queue.front = queue_next_index(queue.front);
}

static void write_key(keynum key, bool pressed, bool held) {
	static uint16_t cache[KEY_COUNT];
	static BITSET(lock_key_pressed, LENGTH(lock_keys));
	uint16_t code;
	if (pressed)
		code = cache[key]
			= (held ? dual_secondary : dual_primary)(get_code(key));
	else
		code = cache[key];

	if (is_lock(code)) {
		if (pressed) {
			pressed = bitset_get(lock_key_pressed, code - LOCK_START);
			bitset_set(lock_key_pressed, code - LOCK_START, !pressed);
			code = lock_code(code);
		} else {
			code = KC_NO;
		}
	}

	if (code == LOCK_RELEASE) {
		for (int i = 0; i < LENGTH(lock_keys); ++i) {
			if (bitset_get(lock_key_pressed, i)) {
				unregister_code16(lock_keys[i]);
				bitset_set(lock_key_pressed, i, false);
			}
		}
	} else {
		(pressed ? register_code16 : unregister_code16)(code);
	}
}

enum { PROCESSED = false, UNPROCESSED = true };

bool process_record_user(uint16_t _ignored, keyrecord_t *record) {
	keynum key = to_keynum(record->event.key);
	bool pressed = record->event.pressed;
	enqueue(key, pressed);
	// Should also clear if the key has no chance of being a mod tap key.
	if (queue.count[key] >= 2 && pressed == false) {
		while (!(queue.count[key] == 2 && front_key() == key)) {
			write_key(front_key(), front_pressed(), true);
			dequeue();
		}
		write_key(key, true, false);
		dequeue();
	}
	while (!queue_empty()
	       && !(front_pressed() && is_dual(get_code(front_key())))) {
		write_key(front_key(), front_pressed(), false);
		dequeue();
	}
	return PROCESSED;
}

#ifdef FAKE_HARDWARE

static keypos_t reverse_map[256];
static const char *code_names[256 * 256];
const char *code_name(uint16_t code) { return code_names[code]; }
static void fill_maps() {
	for (int row = 0; row < MATRIX_ROWS; ++row)
		for (int col = 0; col < MATRIX_COLS; ++col) {
			keypos_t pos = { .col = col, .row = row };
			reverse_map[dual_primary(keymaps[0][row][col])] = pos;
		}
#define ADD_KEY(X) code_names[X] = #X
	ADD_KEY(KC_A);
	ADD_KEY(KC_B);
	ADD_KEY(KC_C);
	ADD_KEY(KC_D);
	ADD_KEY(KC_E);
	ADD_KEY(KC_F);
	ADD_KEY(KC_G);
	ADD_KEY(KC_H);
	ADD_KEY(KC_I);
	ADD_KEY(KC_J);
	ADD_KEY(KC_K);
	ADD_KEY(KC_L);
	ADD_KEY(KC_M);
	ADD_KEY(KC_N);
	ADD_KEY(KC_O);
	ADD_KEY(KC_P);
	ADD_KEY(KC_Q);
	ADD_KEY(KC_R);
	ADD_KEY(KC_S);
	ADD_KEY(KC_T);
	ADD_KEY(KC_U);
	ADD_KEY(KC_V);
	ADD_KEY(KC_W);
	ADD_KEY(KC_X);
	ADD_KEY(KC_Y);
	ADD_KEY(KC_Z);
	ADD_KEY(KC_0);
	ADD_KEY(KC_1);
	ADD_KEY(KC_2);
	ADD_KEY(KC_3);
	ADD_KEY(KC_4);
	ADD_KEY(KC_5);
	ADD_KEY(KC_6);
	ADD_KEY(KC_7);
	ADD_KEY(KC_8);
	ADD_KEY(KC_9);
	ADD_KEY(KC_TAB);
	ADD_KEY(KC_SPACE);
	ADD_KEY(KC_ESCAPE);
	ADD_KEY(KC_ENTER);
	ADD_KEY(KC_BACKSPACE);
	ADD_KEY(KC_QUOTE);
	ADD_KEY(KC_LGUI);
	ADD_KEY(KC_LALT);
	ADD_KEY(KC_LSFT);
	ADD_KEY(KC_LCTL);
	ADD_KEY(KC_RGUI);
	ADD_KEY(KC_RALT);
	ADD_KEY(KC_RSFT);
	ADD_KEY(KC_RCTL);
	ADD_KEY(KC_NO);
	ADD_KEY(KC_INS);
	ADD_KEY(KC_END);
	ADD_KEY(KC_HOME);
	ADD_KEY(KC_LEFT);
	ADD_KEY(KC_DOWN);
	ADD_KEY(KC_UP);
	ADD_KEY(KC_RGHT);
	ADD_KEY(KC_PGUP);
	ADD_KEY(KC_PGDN);
	ADD_KEY(KC_PAUS);
	ADD_KEY(KC_PGDN);
	ADD_KEY(KC_PGUP);
	ADD_KEY(KC_NO);
	ADD_KEY(KC_TRNS);

	ADD_KEY(LGUI_LOCK);
	ADD_KEY(LALT_LOCK);
	ADD_KEY(LSFT_LOCK);
	ADD_KEY(LCTL_LOCK);
	ADD_KEY(PSCR_LOCK);
	ADD_KEY(LOCK_RELEASE);
	ADD_KEY(LOCK_RELEASE);
#undef ADD_KEY
#define ADD_LAYER(X) \
	code_names[MO(X)] = "MO(" #X ")"; \
	code_names[TG(X)] = "TG(" #X ")"

	ADD_LAYER(0);
	ADD_LAYER(1);
	ADD_LAYER(2);
	ADD_LAYER(3);
	ADD_LAYER(4);
	ADD_LAYER(5);
	ADD_LAYER(6);
	ADD_LAYER(7);
	ADD_LAYER(8);
	ADD_LAYER(9);
	ADD_LAYER(10);
	ADD_LAYER(11);
	ADD_LAYER(12);
	ADD_LAYER(13);
	ADD_LAYER(14);
	ADD_LAYER(15);
#undef ADD_LAYER
}

static bool is_momentary(uint16_t code) {
	return QK_MOMENTARY <= code && code <= QK_MOMENTARY_MAX;
}
static bool is_toggle(uint16_t code) {
	return QK_TOGGLE_LAYER <= code && code <= QK_TOGGLE_LAYER_MAX;
}

static void print_key(uint16_t code, bool pressed) {
	eprintf("OUTPUT %s, %d\n", code_name(code), (int)pressed);
}

layer_state_t layer_state = 1;
uint8_t get_highest_layer(layer_state_t state) {
	uint8_t n = 0;
	if (state >> 8) {
		state >>= 8;
		n += 8;
	}
	if (state >> 4) {
		state >>= 4;
		n += 4;
	}
	if (state >> 2) {
		state >>= 2;
		n += 2;
	}
	if (state >> 1) {
		state >>= 1;
		n += 1;
	}
	return n;
}
void register_code16(uint16_t code) {
	if (is_momentary(code))
		layer_state |= (1 << QK_MOMENTARY_GET_LAYER(code));
	else if (is_toggle(code))
		layer_state ^= 1 << QK_TOGGLE_LAYER_GET_LAYER(code);
	print_key(code, true);
}
void unregister_code16(uint16_t code) {
	if (is_momentary(code))
		layer_state &= ~(1 << QK_MOMENTARY_GET_LAYER(code));
	print_key(code, false);
}

static void key(uint8_t code, bool pressed) {
	keypos_t pos = reverse_map[code];
	keyrecord_t record = { .event = { .key = pos, .pressed = pressed } };
	eprintf(
		"key(%s (%d, %d), %d);\n",
		code_name(code),
		(int)pos.col,
		(int)pos.row,
		(int)pressed
	);
	process_record_user(0, &record);
}
int main(int argc, char **argv) {
	fill_maps();
	key(KC_ESC, 1);
	key(KC_B, 1);
	key(KC_B, 0);
	key(KC_ESC, 0);
	key(KC_D, 1);
	key(KC_D, 0);
	key(KC_ENTER, 1);
	key(KC_ENTER, 0);
	key(KC_D, 1);
	key(KC_D, 0);
	return 0;
}
#endif
