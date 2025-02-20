#include "modifiers.h"
#include <stdbool.h>
#include <stdint.h>
#define QMK_KEYBOARD_H "/dev/null"
#define PROGMEM
#define MATRIX_ROWS 8
#define MATRIX_COLS 6
#define LAYOUT_split_3x6_3(k0A, k0B, k0C, k0D, k0E, k0F, k4A, k4B, k4C, k4D, k4E, k4F, k1A, k1B, k1C, k1D, k1E, k1F, k5A, k5B, k5C, k5D, k5E, k5F, k2A, k2B, k2C, k2D, k2E, k2F, k6A, k6B, k6C, k6D, k6E, k6F, k3A, k3B, k3C, k7A, k7B, k7C) \
	{ \
		{ k0A, k0B, k0C, k0D, k0E, k0F }, { k1A, k1B, k1C, k1D, k1E, k1F }, \
			{ k2A, k2B, k2C, k2D, k2E, k2F }, \
			{ k3A, k3B, k3C, KC_NO, KC_NO, KC_NO }, \
			{ k4A, k4B, k4C, k4D, k4E, k4F }, \
			{ k5A, k5B, k5C, k5D, k5E, k5F }, \
			{ k6A, k6B, k6C, k6D, k6E, k6F }, { \
			k7A, k7B, k7C, KC_NO, KC_NO, KC_NO \
		} \
	}
typedef uint8_t layer_state_t;
typedef struct {
	uint8_t col;
	uint8_t row;
} keypos_t;
typedef struct {
	keypos_t key;
	uint16_t time;
	bool pressed;
} keyevent_t;
typedef struct {
	keyevent_t event;
	uint16_t keycode;
} keyrecord_t;
