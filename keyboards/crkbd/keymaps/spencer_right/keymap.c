/*
Copyright 2019 @foostan
Copyright 2020 Drashna Jaelre <@drashna>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H
#include "quantum.h"

#ifdef PIMORONI_TRACKBALL_ENABLE
#include "drivers/sensors/pimoroni_trackball.h"
#include "pointing_device.h"
#endif

// ─────────────────────────────────────────────────────────────────────
// Keycode aliases {{{

#define _v_     KC_TRNS
#define ___     KC_NO
#define x__ENT  LCTL_T(KC_ENT)     /* ctrl(hold) or enter(tap) */
#define x__Q    LCTL_T(KC_Q)       /* ctrl(hold) or q(tap) */
#define x__CBSP C(KC_BSPC)         /* Delete word */
#define x__LMB  KC_MS_BTN1         /* Left mouse button */
#define x__RMB  KC_MS_BTN2         /* Right mouse button */
#define x__MMB  KC_MS_BTN3         /* Mid mouse button */
#define x__CTAB RCTL(KC_TAB)       /* Next tab */
#define x__STAB RCTL(LSFT(KC_TAB)) /* Previous tab */

const uint16_t PROGMEM copy_combo[] = {KC_Z, KC_C, COMBO_END};
const uint16_t PROGMEM paste_combo[] = {KC_X, KC_V, COMBO_END};

combo_t key_combos[COMBO_COUNT] = {
    COMBO(copy_combo, LCTL(KC_C)),
    COMBO(paste_combo, LCTL(KC_V)), // keycodes with modifiers are possible too!
};

/* Macros and stuff */
enum custom_keycodes {
  MC_MUTE = SAFE_RANGE,
  MC_HAND,
  MC_SHOT,
  CKC_COMU,
  CKC_DOTQ,
  CKC_MIN
};

enum layers {
  _BASE = 0, _SYM, _NAV, _FUN
};


enum trackball_keycodes {
    BALL_LC = SAFE_RANGE,
    BALL_SCR
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BASE] = LAYOUT_split_3x5_3(
    KC_Q, KC_W, KC_E, KC_R, KC_T, /**/ KC_Y, KC_U, KC_I,    KC_O,   KC_P,
    KC_A, KC_S, KC_D, KC_F, KC_G, /**/ KC_H, KC_J, KC_K,    KC_L,   KC_QUOT,
    KC_LSFT, KC_Z, KC_X, KC_C, KC_V, /**/ KC_B, KC_N, KC_M, CKC_DOTQ, CKC_DOTQ,
    /**/  /**/  MT(MOD_LCTL, KC_TAB), MO(_SYM), LT(3, KC_SPC), /**/ LT(4, KC_ENT), LT(1, KC_BSPC), MT(MOD_LALT, KC_SCLN)     /**/    /**/
  ),
  [_SYM] = LAYOUT_split_3x5_3(
    KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_GRV,  /**/ KC_LBRC, KC_LT,    KC_EQL,  KC_GT,   KC_RBRC,
    KC_PERC, KC_CIRC,   KC_AMPR, KC_ASTR,  KC_TILD, /**/ KC_LCBR, KC_LPRN,  KC_COLN, KC_RPRN, KC_RCBR,
    KC_NO, KC_NO, KC_PIPE, KC_NO, KC_BSLS, /**/ KC_UNDS, KC_MINS,  KC_NO, KC_PLUS, KC_QUES,
    /**/     /**/     KC_BSPC,     KC_TRNS,     KC_TRNS,   /**/ KC_TRNS,     KC_TRNS, KC_TRNS      /**/     /**/
  ),
  [_NAV] = LAYOUT_split_3x5_3(
    KC_1,    KC_2,    KC_3,      KC_4,    KC_VOLU,  /**/ KC_PGUP, KC_HOME, KC_UP,   KC_END,  x__CBSP,
    KC_5,    KC_6,    KC_7,      KC_8,    KC_VOLD, /**/  KC_PGDN, KC_LEFT, KC_DOWN, KC_RGHT, KC_NO,
    KC_TRNS, KC_LCTL, KC_9,      KC_0,    KC_ESC,  /**/  KC_SLSH,  KC_NO,  KC_NO, KC_NO,  KC_DEL,
    /**/     /**/     KC_TRNS,       KC_TRNS, KC_TRNS,      /**/ KC_TRNS, KC_BSPC, KC_LGUI      /**/     /**/
  ),    
  [_FUN] = LAYOUT_split_3x5_3(
    KC_F11, KC_F12, ___,   MC_SHOT, ___,    /**/ KC_WH_U, x__LMB,  KC_MS_U, x__RMB,  KC_BRIU,
    KC_F1,  KC_F2,  KC_F3, KC_F4,   KC_F5,  /**/ KC_WH_D, KC_MS_L, KC_MS_D, KC_MS_R, KC_BRID,
    KC_F6,  KC_F7,  KC_F8, KC_F9,   KC_F10, /**/ KC_VOLD, KC_VOLU, KC_MPLY, KC_MNXT, RESET,
    /**/    /**/    _v_,   _v_,     _v_,    /**/ _v_,     _v_,     _v_      /**/     /**/
  ),
};

bool get_ignore_mod_tap_interrupt(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case MT(MOD_LCTL, KC_ENT):
            // Do not force the mod-tap key press to be handled as a modifier
            // if any other key was pressed while the mod-tap key is held down.
            return true;
        default:
            // Force the mod-tap key press to be handled as a modifier if any
            // other key was pressed while the mod-tap key is held down.
            return false;
    }
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LT(4, KC_SPC):   
            // Do not select the hold action when another key is pressed.
            return false;
            
        default:
            // Immediately select the hold action when another key is pressed.
            return true;
    }
}

void keyboard_post_init_user(void) {

#ifdef PIMORONI_TRACKBALL_ENABLE
    trackball_set_rgbw(0,0,0,80);
#endif
}


bool process_record_user(uint16_t keycode, keyrecord_t *record){
  switch (keycode){
#ifdef PIMORONI_TRACKBALL_ENABLE
  case BALL_LC:
     record->event.pressed?register_code(KC_BTN1):unregister_code(KC_BTN1);
     break;
  case BALL_SCR:
    if(record->event.pressed){
      trackball_set_scrolling(true);
    } else{
      trackball_set_scrolling(false);
    }
    break;
#endif
  default:
    break;
  }
  return true;
}


#ifdef PIMORONI_TRACKBALL_ENABLE
layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
    case _BASE:
        trackball_set_rgbw(0,0,0,80);
        break;
    case _NAV:
        trackball_set_rgbw(0,153,95,0);
        break;
    case _SYM:
        trackball_set_rgbw(153,113,0,0);
        break;
    default: //  for any other layers, or the default layer
        trackball_set_rgbw(0,0,0,80);
        break;
    }
  return state;
}
#endif
