// Copyright 2025 Andrew Kannan (awkannan)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#ifdef OLED_ENABLE
#    include <stdio.h>
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

[0] = LAYOUT(
  KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,  KC_5,                     KC_6,    KC_7,   KC_8,    KC_9,    KC_0,    KC_BSLS,
  KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,  KC_T,                     KC_Y,    KC_U,   KC_I,    KC_O,    KC_P,    KC_BSPC,
  KC_LCTL,  KC_A,    KC_S,    KC_D,    KC_F,  KC_G,                     KC_H,    KC_J,   KC_K,    KC_L,    KC_SCLN, KC_QUOT,
  KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,  KC_B,   KC_MUTE, KC_MPLY, KC_N,    KC_M,   KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
           KC_LCTL, KC_LALT, KC_LGUI, MO(1), KC_ENT,                   KC_SPC,  MO(2),  KC_RGUI, KC_RALT, KC_RCTL
),

[1] = LAYOUT(
  KC_GRV, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                     KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
  KC_TRNS,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_F12,
  KC_WREF, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC,                   KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_PIPE,
  KC_LSFT, KC_EQL,  KC_MINS, KC_PLUS, KC_LCBR, KC_RCBR, KC_TRNS, KC_TRNS, KC_LBRC, KC_RBRC, KC_DOT,  KC_COLN, KC_BSLS, KC_RSFT,
           KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
),

[2] = LAYOUT(
  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, LSG(KC_4), LSG(KC_5),                  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
  QK_BOOT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                    KC_TRNS, KC_UP, KC_UP,   KC_TRNS, KC_TRNS, KC_DEL,
  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                    KC_LEFT, KC_DOWN, KC_RIGHT, KC_TRNS, KC_TRNS, KC_TRNS,
  KC_TRNS, KC_TRNS, KC_TRNS, LCTL(LGUI(KC_C)), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
           KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
)
};

// --- FIXED ENCODER LOGIC ---
// Handles BOTH encoders (Index 0 = Left, Index 1 = Right)
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        // Left Encoder: Volume
        tap_code(clockwise ? KC_VOLU : KC_VOLD);
    } else if (index == 1) {
        // Right Encoder: Brightness
        tap_code(clockwise ? KC_BRIU : KC_BRID);
    }
    return true; // Return true to allow default behavior if needed
}

#ifdef OLED_ENABLE

// Helper to map layer names
static const char *layer_name(uint8_t layer) {
    switch (layer) {
        case 0: return "Base";
        case 1: return "Sym";
        case 2: return "Nav";
        default: return "Other";
    }
}

static void render_master_status(void) {
    char line[24];

    // Layer Name (Max 6 chars)
    snprintf(line, sizeof(line), "%s", layer_name(get_highest_layer(layer_state)));
    oled_write_ln(line, false);

    // WPM (Max ~6 chars)
    snprintf(line, sizeof(line), "WPM:%u", (unsigned)get_current_wpm());
    oled_write_ln(line, false);

    // Mods Indicator
    uint8_t mods = get_mods() | get_oneshot_mods();
    oled_write_P(PSTR("M:"), false);
    oled_write_char((mods & MOD_MASK_SHIFT) ? 'S' : '.', false);
    oled_write_char((mods & MOD_MASK_CTRL) ? 'C' : '.', false);
    oled_write_char((mods & MOD_MASK_ALT) ? 'A' : '.', false);
    oled_write_char((mods & MOD_MASK_GUI) ? 'G' : '.', false);
    oled_write_ln_P(PSTR(""), false);

    // Status Line (Shortened to prevent wrap)
    oled_write_ln_P(PSTR("Vol|Br"), false);

    // Caps Lock
    led_t led = host_keyboard_led_state();
    snprintf(line, sizeof(line), "CAP:%s", led.caps_lock ? "ON" : " ");
    oled_write_ln(line, false);
    
    // Host Status
    oled_write_ln_P(PSTR("Host"), false);
}

static void render_slave_idle(void) {
    oled_write_ln_P(PSTR(" LINK"), false);
    oled_write_ln_P(PSTR(" LTL"), false);
    oled_write_ln_P(PSTR("  -------"), false); // Shortened dashes
}

static void render_slave_nav_mode(void) {
    oled_write_ln_P(PSTR("NAV"), false);
    oled_write_ln_P(PSTR("Arrows"), false);
    oled_write_ln_P(PSTR("Boot"), false);
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        render_master_status();
    } else if (get_highest_layer(layer_state) >= 2) {
        render_slave_nav_mode();
    } else {
        render_slave_idle();
    }
    // Return false: stock link.c oled_task_kb() renders its own master footer /
    // slave QMK logo when oled_task_user() returns true. Returning false suppresses
    // that so only this keymap's OLED content is drawn on stock firmware.
    return false;
}

#endif