/* Copyright 2025 Jing Huang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

/* Simultaneous Opposing Cardinal Directions (SOCD) filtering.
 * Based on Pascal Getreuer's implementation.
 */

enum socd_cleaner_resolution {
    // Disable SOCD filtering for this key pair.
    SOCD_CLEANER_OFF,
    // Last input priority with reactivation.
    SOCD_CLEANER_LAST,
    // Neutral resolution. When both keys are pressed, they cancel.
    SOCD_CLEANER_NEUTRAL,
    // Former key always wins.
    SOCD_CLEANER_FORMER,
    // Latter key always wins.
    SOCD_CLEANER_LATTER,
    // Sentinel to count the number of resolution strategies.
    SOCD_CLEANER_NUM_RESOLUTIONS,
};

typedef struct {
    uint8_t code : 7; // Basic keycodes for the two opposing keys.
    bool    held : 1; // Tracks which keys are physically held.
} socd_key;

typedef struct {
    socd_key keys[2];
    uint8_t  resolution; // Resolution strategy.
} socd_cleaner_t;

bool process_socd_cleaner(uint16_t keycode, keyrecord_t* record, socd_cleaner_t* state) {
    if (!state->resolution || !(keycode == state->keys[0].code || keycode == state->keys[1].code)) {
        return true; // Quick return when disabled or on unrelated events.
    }
    // The current event corresponds to index `i`, 0 or 1, in the SOCD key pair.
    const uint8_t i        = (keycode == state->keys[1].code);
    const uint8_t opposing = i ^ 1; // Index of the opposing key.

    // Track which keys are physically held (vs. keys in the report).
    state->keys[i].held = record->event.pressed;

    // Perform SOCD resolution for events where the opposing key is held.
    if (state->keys[opposing].held) {
        switch (state->resolution) {
            case SOCD_CLEANER_LAST: // Last input priority with reactivation.
                // If the current event is a press, then release the opposing key.
                // Otherwise if this is a release, then press the opposing key.
                if (state->keys[i].held)
                    del_key(state->keys[opposing].code);
                else
                    add_key(state->keys[opposing].code);
                break;

            case SOCD_CLEANER_NEUTRAL: // Neutral resolution.
                // Same logic as SOCD_CLEANER_LAST, but skip default handling so that
                // the current key has no effect while the opposing key is held.
                if (state->keys[i].held)
                    del_key(state->keys[opposing].code);
                else
                    add_key(state->keys[opposing].code);
                // Send updated report (normally, default handling would do this).
                send_keyboard_report();
                return false; // Skip default handling.

            case SOCD_CLEANER_FORMER: // Former wins.
            case SOCD_CLEANER_LATTER: // Latter wins.
                if (opposing == (state->resolution - SOCD_CLEANER_FORMER)) {
                    // The opposing key is the winner. The current key has no effect.
                    return false; // Skip default handling.
                } else {
                    // The current key is the winner. Update logic is same as above.
                    if (state->keys[i].held)
                        del_key(state->keys[opposing].code);
                    else
                        add_key(state->keys[opposing].code);
                }
                break;
        }
    }
    return true; // Continue default handling to press/release current key.
}

/* Definition and configuration of the actual keymap.
 */

enum layer_literal { _BASE, _EXTN };
enum custom_keycode { _SOCD = SAFE_RANGE, _DYMC };

#define LY_EXTN MO(_EXTN)
#define GET_ORIG_KEY(record) pgm_read_word(&keymaps[_BASE][record->event.key.row][record->event.key.col])

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┐   ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┐
     * │Esc│   │F1 │F2 │F3 │F4 │ │F5 │F6 │F7 │F8 │ │F9 │F10│F11│F12│ │PSc│Scr│Pse│
     * └───┘   └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┘
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐ ┌───┬───┬───┐
     * │ ` │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │ - │ = │ Backsp│ │Ins│Hom│PgU│
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤ ├───┼───┼───┤
     * │ Tab │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │ [ │ ] │  \  │ │Del│End│PgD│
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤ └───┴───┴───┘
     * │ Caps │ A │ S │ D │ F │ G │ H │ J │ K │ L │ ; │ ' │  Enter │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤     ┌───┐
     * │ Shift  │ Z │ X │ C │ V │ B │ N │ M │ , │ . │ / │    Shift │     │ ↑ │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┼───┴┬────┬────┤ ┌───┼───┼───┐
     * │Ctrl│GUI │Alt │                        │ Alt│ GUI│Menu│Ctrl│ │ ← │ ↓ │ → │
     * └────┴────┴────┴────────────────────────┴────┴────┴────┴────┘ └───┴───┴───┘
     */
    [_BASE] = LAYOUT_tkl_ansi(
            KC_ESC,           KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,     KC_PSCR, KC_SCRL, KC_PAUS,

            KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,    KC_INS,  KC_HOME, KC_PGUP,
            KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,    KC_DEL,  KC_END,  KC_PGDN,
            KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,
            KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT,             KC_UP,
            KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, LY_EXTN, KC_APP,  KC_RCTL,    KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_EXTN] = LAYOUT_tkl_ansi(
            KC_TRNS,          KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,

            KC_TRNS, _DYMC,   _DYMC,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
            KC_TRNS, KC_TRNS, KC_TRNS, EE_CLR,  QK_BOOT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
            KC_TRNS, KC_TRNS, _SOCD,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, QK_LOCK, KC_TRNS, KC_TRNS,          KC_TRNS,
            KC_TRNS,          KC_TRNS, KC_TRNS, _DYMC,   KC_TRNS, QK_RBT,  NK_TOGG, DM_RSTP, KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS,             KC_TRNS,
            KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS,                            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS
    ),
};
// clang-format on

socd_cleaner_t socd_v = {{{KC_W}, {KC_S}}, SOCD_CLEANER_NEUTRAL};
socd_cleaner_t socd_h = {{{KC_A}, {KC_D}}, SOCD_CLEANER_LAST};

static bool    socd_cleaner_enabled = false;
static uint8_t dynamic_macro_status = false;

bool dynamic_macro_record_start_user(int8_t direction) {
    if (direction == 1) {
        dynamic_macro_status |= 1;
        dynamic_macro_status &= ~(1 << 1);
    } else {
        dynamic_macro_status |= (1 << 2);
        dynamic_macro_status &= ~(1 << 3);
    }
    return true;
}

bool dynamic_macro_record_end_user(int8_t direction) {
    if (direction == 1) {
        dynamic_macro_status |= (1 << 1);
        dynamic_macro_status &= ~1;
    } else {
        dynamic_macro_status |= (1 << 3);
        dynamic_macro_status &= ~(1 << 2);
    }
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    if (socd_cleaner_enabled) {
        if (!process_socd_cleaner(keycode, record, &socd_v)) return false;
        if (!process_socd_cleaner(keycode, record, &socd_h)) return false;
    }

    switch (keycode) {
        case _SOCD:
            if (record->event.pressed) socd_cleaner_enabled ^= true;
            break;
        case _DYMC:
            if (record->event.pressed) {
                switch (GET_ORIG_KEY(record)) {
                    case KC_1:
                        if (dynamic_macro_status & (1 << 1))
                            tap_code16(DM_PLY1);
                        else
                            tap_code16(DM_REC1);
                        break;
                    case KC_2:
                        if (dynamic_macro_status & (1 << 3))
                            tap_code16(DM_PLY2);
                        else
                            tap_code16(DM_REC2);
                        break;
                    default:
                        dynamic_macro_status = false;
                }
            }
            break;
    }

    return true;
}
