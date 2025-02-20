#ifndef QMK_KEYBOARD_H
#include "fake-qmk.h"
#define FAKE_HARDWARE
#endif

#define LENGTH(A) (sizeof(A) / sizeof(A[0]))

#include "quantum_keycodes.h"
enum {
	_IGNORED = SAFE_RANGE,
	LALT_LOCK,
	LCTL_LOCK,
	LSFT_LOCK,
	LGUI_LOCK,
	PSCR_LOCK,
	LOCK_RELEASE,

	LOCK_START = LALT_LOCK,
};
#include "jackfaller.h"

static uint8_t lock_keys[] = {
	KC_LALT, KC_LGUI, KC_LCTL, KC_LSFT, KC_PSCR,
};
static bool is_lock(uint16_t code) {
	return LOCK_START <= code && code <= LOCK_START + LENGTH(lock_keys);
}

static bool is_mod_tap(uint16_t code) {
	return (code & QK_MOD_TAP) == QK_MOD_TAP;
}
static bool is_layer_tap(uint16_t code) {
	return (code & QK_LAYER_TAP) == QK_LAYER_TAP;
}
static bool is_dual(uint16_t code) {
	return is_mod_tap(code) || is_layer_tap(code);
}
static uint8_t dual_primary(uint16_t code) {
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
bool bitset_get(uint8_t *bitset, int i) {
	return (bitset[i / 8] & (1 << (i % 8))) != 0;
}
void bitset_set(uint8_t *bitset, int i, bool value) {
	int index = i / 8, subindex = i % 8;
	uint8_t old_val = bitset[index];
	bool old = old_val & 1 << subindex;
	bitset[index] = old_val ^ ((old == 0) != (value == 0)) << subindex;
}

typedef uint8_t keynum;

static layer_state_t layer_on_press[MATRIX_ROWS * MATRIX_COLS];
static BITSET(key_held, MATRIX_ROWS *MATRIX_COLS);
static struct {
	keynum keys[MATRIX_ROWS * MATRIX_COLS];
	BITSET(states, MATRIX_ROWS *MATRIX_COLS);
	// Assume the queue can never be full.
	uint8_t count[MATRIX_ROWS * MATRIX_COLS];
	uint8_t front, back;
} queue;

#define KEY_AT(keynum) keynum % MATRIX_ROWS][keynum / MATRIX_ROWS
static keynum to_keynum(keypos_t keypos) {
	return keypos.col + keypos.row * MATRIX_ROWS;
}

static bool in_queue(keynum key) { return queue.count[key] != 0; }
static bool queue_empty(void) { return queue.front == queue.back; }
static uint8_t queue_next_index(uint8_t i) {
	return (i + 1) % LENGTH(queue.keys);
}
static void enqueue(keynum key, bool pressed) {
	++queue.count[key];
	queue.keys[queue.back] = key;
	bitset_set(queue.states, key, pressed);
	queue.back = queue_next_index(queue.back);
}
static void dequeue() {
	--queue.count[queue.keys[queue.front]];
	queue.front = queue_next_index(queue.front);
}
static bool front_pressed() { return bitset_get(queue.states, queue.front); }
static bool front_key() { return queue.keys[queue.front]; }

static void write_key(keynum key, bool pressed, bool held) {}

enum { PROCESSED = false, UNPROCESSED = true };

bool process_record_user(uint16_t _ignored, keyrecord_t *record) {
	keynum num = to_keynum(record->event.key);
	bool pressed = record->event.pressed;
	return PROCESSED;
}

#ifdef FAKE_HARDWARE
#include <stdio.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
static keypos_t reverse_map[256];
static keypos_t getpos(uint8_t code) { return reverse_map[code]; }
static uint8_t getcode(keypos_t pos) {
	return dual_primary(keymaps[0][pos.row][pos.col]);
}
static void fill_reverse_map() {
	for (int row = 0; row < MATRIX_ROWS; ++row)
		for (int col = 0; col < MATRIX_COLS; ++col) {
			keypos_t pos = { .col = col, .row = row };
			reverse_map[getcode(pos)] = pos;
		}
}
const char *code_name(uint8_t code) {
	switch (code) {
#define CASE(X) \
	case X: return #X
		CASE(KC_A);
		CASE(KC_B);
		CASE(KC_C);
		CASE(KC_D);
		CASE(KC_E);
		CASE(KC_F);
		CASE(KC_G);
		CASE(KC_H);
		CASE(KC_I);
		CASE(KC_J);
		CASE(KC_K);
		CASE(KC_L);
		CASE(KC_M);
		CASE(KC_N);
		CASE(KC_O);
		CASE(KC_P);
		CASE(KC_Q);
		CASE(KC_R);
		CASE(KC_S);
		CASE(KC_T);
		CASE(KC_U);
		CASE(KC_V);
		CASE(KC_W);
		CASE(KC_X);
		CASE(KC_Y);
		CASE(KC_Z);
		CASE(KC_0);
		CASE(KC_1);
		CASE(KC_2);
		CASE(KC_3);
		CASE(KC_4);
		CASE(KC_5);
		CASE(KC_6);
		CASE(KC_7);
		CASE(KC_8);
		CASE(KC_9);
		CASE(KC_TAB);
		CASE(KC_SPACE);
		CASE(KC_ESCAPE);
		CASE(KC_ENTER);
		CASE(KC_BACKSPACE);
		CASE(KC_QUOTE);
#undef case
	default: return "UNKNOWN";
	}
}
static void key(uint8_t code, bool pressed) {
	keyrecord_t record
		= { .event = { .key = getpos(code), .pressed = pressed } };
	eprintf("key(%s, %d);\n", code_name(code), (int)pressed);
	process_record_user(0, &record);
}
int main(int argc, char **argv) {
	fill_reverse_map();
	key(KC_K, 1);
	key(KC_A, 1);
	key(KC_A, 0);
	key(KC_K, 0);
	return 0;
}
#endif
