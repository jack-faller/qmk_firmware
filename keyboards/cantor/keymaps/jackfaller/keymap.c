#ifndef QMK_KEYBOARD_H
#include "fake-qmk.h"
#include <stdio.h>
#define TESTING
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define eprintf(...)
#endif

#define LENGTH(A) (sizeof(A) / sizeof(A[0]))

#include "quantum_keycodes.h"

bool process_keycode_any(uint16_t keycode, const bool pressed);
#ifndef TESTING
// This is not how you're supposed to use modules but I can't find any other
// way.
#define NO_ACTION_ONESHOT
#undef ASSERT_COMMUNITY_MODULES_MIN_API_VERSION
#define ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(A, B, C)
#include "modules/stephen_ostermiller/process_keycode_any/process_keycode_any.c"
#endif

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
#define KEYNUM_MAX (MATRIX_ROWS * MATRIX_COLS)
static keynum to_keynum(keypos_t keypos) {
	return keypos.col + keypos.row * MATRIX_COLS;
}

static struct {
	keynum keys[KEYNUM_MAX];
	BITSET(states, KEYNUM_MAX);
	// Assume the queue can never be full.
	uint8_t count[KEYNUM_MAX];
	uint8_t front, back;
} queue;

static uint16_t get_code(keynum key) {
	uint16_t out = KC_TRNS;
	for (int highest = get_highest_layer(layer_state);
	     out == KC_TRNS && highest >= 0;
	     --highest) {
		if (IS_LAYER_ON(highest)) {
			out = keymaps[highest][key / MATRIX_COLS][key % MATRIX_COLS];
		}
	}
	return out;
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
	static uint16_t cache[KEYNUM_MAX];
	uint16_t code;
	if (pressed)
		code = cache[key]
			= (held ? dual_secondary : dual_primary)(get_code(key));
	else
		code = cache[key];

	process_keycode_any(code, pressed);
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

#ifdef TESTING

#define MAP_BASE_KEYS(F, JOIN) \
	F(KC_A) JOIN F(KC_B) \
	JOIN F(KC_C) \
	JOIN F(KC_D) \
	JOIN F(KC_E) \
	JOIN F(KC_F) \
	JOIN F(KC_G) \
	JOIN F(KC_H) \
	JOIN F(KC_I) \
	JOIN F(KC_J) \
	JOIN F(KC_K) \
	JOIN F(KC_L) \
	JOIN F(KC_M) \
	JOIN F(KC_N) \
	JOIN F(KC_O) \
	JOIN F(KC_P) \
	JOIN F(KC_Q) \
	JOIN F(KC_R) \
	JOIN F(KC_S) \
	JOIN F(KC_T) \
	JOIN F(KC_U) \
	JOIN F(KC_V) \
	JOIN F(KC_W) \
	JOIN F(KC_X) \
	JOIN F(KC_Y) \
	JOIN F(KC_Z) \
	JOIN F(KC_TAB) \
	JOIN F(KC_SPACE) \
	JOIN F(KC_ESCAPE) \
	JOIN F(KC_ENTER) \
	JOIN F(KC_BACKSPACE) \
	JOIN F(KC_QUOTE)

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
	MAP_BASE_KEYS(ADD_KEY, ;);
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

	ADD_KEY(QK_LOCK);
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

static void print_key(const char *prefix, uint16_t code, bool pressed) {
	eprintf("%s %s %s\n", prefix, code_name(code), (pressed ? "down" : "up"));
}

static int output_states[UINT16_MAX];
bool process_keycode_any(uint16_t code, bool pressed) {
	if (pressed) {
		if (is_momentary(code))
			layer_state |= (1 << QK_MOMENTARY_GET_LAYER(code));
		else if (is_toggle(code))
			layer_state ^= 1 << QK_TOGGLE_LAYER_GET_LAYER(code);
	} else {
		if (is_momentary(code))
			layer_state &= ~(1 << QK_MOMENTARY_GET_LAYER(code));
	}

	output_states[code] += (pressed ? 1 : -1);
	print_key("  OUTPUT", code, pressed);
	return true;
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

static BITSET(base_key_states, UINT8_MAX);
static void key(uint8_t code) {
	bool pressed = !bitset_get(base_key_states, code);
	bitset_set(base_key_states, code, pressed);
	keypos_t pos = reverse_map[code];
	keyrecord_t record = { .event = { .key = pos, .pressed = pressed } };
	print_key("INPUT", code, pressed);
	process_record_user(0, &record);
}

#include <stdlib.h>

int main(int argc, char **argv) {
	const static uint8_t base_keys[] = {
#define F(x) x
#define COMMA ,
		MAP_BASE_KEYS(F, COMMA)
#undef F
#undef COMMA
	};
	fill_maps();
	if (argc != 2) {
		key(KC_K);
		key(KC_W);
		key(KC_W);
		key(KC_K);
		key(KC_K);
		key(KC_W);
		key(KC_K);
		key(KC_W);
	} else {
		int length = atoi(argv[1]);

		eprintf("%d\n", length);
		for (;;) {
			for (int i = 0; i < length; ++i)
				key(base_keys[rand() % LENGTH(base_keys)]);
			eprintf("\n");

			int on_count = 0;
			for (int code = 0; code < UINT8_MAX; ++code)
				on_count += bitset_get(base_key_states, code) ? 1 : 0;
			for (; on_count > 0; --on_count) {
				int turn_off = rand() % on_count;
				for (int code = 0; code < UINT8_MAX; ++code)
					if (bitset_get(base_key_states, code)) {
						if (turn_off == 0) {
							key(code);
							break;
						} else {
							--turn_off;
						}
					}
			}

			eprintf("\n");
			for (int i = 0; i < LENGTH(output_states); ++i)
				if (output_states[i] != 0) {
					eprintf("FAILED with key %s.\n", code_name(i));
					return 1;
				}
			eprintf("\n");
		}
	}
	return 0;
}
#endif
