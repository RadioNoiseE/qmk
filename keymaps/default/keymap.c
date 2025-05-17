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
    SOCD_CLEANER_OFF,
    SOCD_CLEANER_LAST,
    SOCD_CLEANER_NEUTRAL,
    SOCD_CLEANER_FORMER,
    SOCD_CLEANER_LATTER,
    SOCD_CLEANER_NUM_RESOLUTIONS,
};

typedef struct {
    uint8_t code : 7;
    bool    held : 1;
} socd_key;

typedef struct {
    socd_key keys[2];
    uint8_t  resolution;
} socd_cleaner_t;

bool process_socd_cleaner(uint16_t keycode, keyrecord_t* record, socd_cleaner_t* state) {
    if (!state->resolution || !(keycode == state->keys[0].code || keycode == state->keys[1].code)) {
        return true;
    }

    const uint8_t i        = (keycode == state->keys[1].code);
    const uint8_t opposing = i ^ 1;

    state->keys[i].held = record->event.pressed;

    if (state->keys[opposing].held) {
        switch (state->resolution) {
            case SOCD_CLEANER_LAST:
                if (state->keys[i].held)
                    del_key(state->keys[opposing].code);
                else
                    add_key(state->keys[opposing].code);
                break;
            case SOCD_CLEANER_NEUTRAL:
                if (state->keys[i].held)
                    del_key(state->keys[opposing].code);
                else
                    add_key(state->keys[opposing].code);
                send_keyboard_report();
                return false;
            case SOCD_CLEANER_FORMER:
            case SOCD_CLEANER_LATTER:
                if (opposing == (state->resolution - SOCD_CLEANER_FORMER)) {
                    return false;
                } else {
                    if (state->keys[i].held)
                        del_key(state->keys[opposing].code);
                    else
                        add_key(state->keys[opposing].code);
                }
                break;
        }
    }
    return true;
}

/* Definition and configuration of the actual keymap.
 * Speed keys are defined based on LdBeth's approach.
 */

enum layer_literal { _BASE, _EXTN };
enum custom_keycode { _SOCD = SAFE_RANGE, _DYMC };

#define LY_EXTN MO(_EXTN)
#define GET_ORIG_KEY(record) keymap_key_to_keycode(_BASE, record->event.key)
#define MAY_WANT_OUT(process)       \
    do {                            \
        if (!process) return false; \
    } while (false)
#define DEF_ESPD_KEY(key)                                                \
    case key: {                                                          \
        static deferred_token token = INVALID_DEFERRED_TOKEN;            \
        static uint8_t        repeat_count;                              \
        if (!record->event.pressed) {                                    \
            cancel_deferred_exec(token);                                 \
            token = INVALID_DEFERRED_TOKEN;                              \
        } else if (!token) {                                             \
            tap_code(key);                                               \
            repeat_count = 0;                                            \
            uint32_t key##_C(uint32_t trigger_time, void* cb_arg) {      \
                tap_code(key);                                           \
                if (repeat_count < sizeof(repeat_delay)) ++repeat_count; \
                return pgm_read_byte(repeat_delay + repeat_count - 1);   \
            }                                                            \
            token = defer_exec(300, key##_C, NULL);                      \
        }                                                                \
    }                                                                    \
        return false

// clang-format off

const uint8_t repeat_delay[] PROGMEM = {
  99, 79, 65, 57, 49, 43, 40, 35, 33, 30, 28, 26, 25, 23, 22, 20,
  20, 19, 18, 17, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10
};

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
            KC_TRNS,          KC_TRNS, KC_TRNS, CL_TOGG, KC_TRNS, QK_RBT,  NK_TOGG, _DYMC,   KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS,             KC_TRNS,
            KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS,                            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS
    ),
};

// clang-format on

socd_cleaner_t socd_v = {{{KC_W}, {KC_S}}, SOCD_CLEANER_NEUTRAL};
socd_cleaner_t socd_h = {{{KC_A}, {KC_D}}, SOCD_CLEANER_LAST};

static uint8_t keyboard_status;

#define SOCD_TG (1 << 0)
#define DM_RNG1 (1 << 1)
#define DM_RED1 (1 << 2)
#define DM_REL1 (1 << 3)
#define DM_RNG2 (1 << 4)
#define DM_RED2 (1 << 5)
#define DM_REL2 (1 << 6)

bool dynamic_macro_record_start_user(int8_t direction) {
    if (direction == 1) {
        keyboard_status |= DM_RNG1;
        keyboard_status &= ~DM_RED1;
    } else {
        keyboard_status |= DM_RNG2;
        keyboard_status &= ~DM_RED2;
    }
    return true;
}

bool dynamic_macro_record_end_user(int8_t direction) {
    if (direction == 1) {
        keyboard_status |= DM_RED1;
        keyboard_status &= ~DM_RNG1;
    } else {
        keyboard_status |= DM_RED2;
        keyboard_status &= ~DM_RNG2;
    }
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    if (keyboard_status & SOCD_TG) {
        MAY_WANT_OUT(process_socd_cleaner(keycode, record, &socd_v));
        MAY_WANT_OUT(process_socd_cleaner(keycode, record, &socd_h));
    }

    switch (keycode) {
        DEF_ESPD_KEY(KC_BSPC);
        DEF_ESPD_KEY(KC_ENT);
        case _SOCD:
            if (record->event.pressed) keyboard_status ^= SOCD_TG;
            break;
        case _DYMC:
            switch (GET_ORIG_KEY(record)) {
                case KC_1:
                    if (keyboard_status & DM_RNG1)
                        MAY_WANT_OUT(process_dynamic_macro(DM_RSTP, record));
                    else if (keyboard_status & DM_RED1) {
                        if (keyboard_status & DM_REL1)
                            MAY_WANT_OUT(process_dynamic_macro(DM_PLY1, record));
                        else
                            keyboard_status |= DM_REL1;
                    } else
                        MAY_WANT_OUT(process_dynamic_macro(DM_REC1, record));
                    break;
                case KC_2:
                    if (keyboard_status & DM_RNG2)
                        MAY_WANT_OUT(process_dynamic_macro(DM_RSTP, record));
                    else if (keyboard_status & DM_RED2) {
                        if (keyboard_status & DM_REL2)
                            MAY_WANT_OUT(process_dynamic_macro(DM_PLY2, record));
                        else
                            keyboard_status |= DM_REL2;
                    } else
                        MAY_WANT_OUT(process_dynamic_macro(DM_REC2, record));
                    break;
                default:
                    if (record->event.pressed) keyboard_status &= ~(DM_RNG1 | DM_RED1 | DM_REL1 | DM_RNG2 | DM_RED2 | DM_REL2);
            }
            break;
    }

    return true;
}
