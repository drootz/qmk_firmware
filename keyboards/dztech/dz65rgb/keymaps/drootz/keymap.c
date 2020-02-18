#include QMK_KEYBOARD_H

/**************** SOME GLOBALS *********************/
 
bool onMac = false;
bool isLeader = false;
bool isBlinking = false;
const float ledDimRatio = 0.50; 
bool isRecording = false;
bool isPlaying = false;
static uint16_t blink_cycle_timer, 
                blink_fade_in_timer, 
                blink_fade_out_timer, 
                blink_hsv_value,
                macro_play_blink_timer = 2000,
                macro_one_play_timer,
                macro_two_play_timer;
static uint8_t  fade_in_step_counter, 
                fade_out_step_counter, 
                recording_r, 
                recording_g, 
                recording_b;

/**************** HELPER FUNCTIONS *********************/

uint8_t get_blink_rgb(uint16_t h, uint16_t s, uint16_t v, uint8_t output) {
    HSV hsv = {h, s, v * ((ledDimRatio / 2) + ledDimRatio)};
    RGB rgb = hsv_to_rgb(hsv);
    const uint8_t HSV_RGB[3] = {rgb.r, rgb.g, rgb.b};
    switch (output) {
        case 0:
            return HSV_RGB[0];
            break;
        case 1:
            return HSV_RGB[1];
            break;
        case 2:
            return HSV_RGB[2];
            break;
        default:
            return false;
    }
}

void set_blink_rgb(uint16_t h, uint16_t s, uint16_t v) {
    recording_r = get_blink_rgb(h, s, v, 0);
    recording_g = get_blink_rgb(h, s, v, 1);
    recording_b = get_blink_rgb(h, s, v, 2);
}

void reset_blink_cycle(void) {
    blink_cycle_timer = timer_read();
    blink_fade_in_timer = timer_read();
    blink_fade_out_timer = timer_read();
    blink_hsv_value = 0;
    fade_in_step_counter = 0;
    fade_out_step_counter = 0;
}

void reset_blink_status(void) {
    isRecording = false;
    isPlaying = false;
    isBlinking = false;
}

void get_this_led_blinking(uint16_t led_index, bool speed, uint16_t hue, uint16_t sat) {
    const uint16_t static_on_time = speed ? 200 : 500;
    const uint16_t static_off_time = speed ? 200 : 500;
    const uint8_t fade_timing = speed ? 100 : 150;
    const uint8_t fade_step = speed ? 10 : 15;
    const uint8_t fade_value_decrement_size = rgb_matrix_config.hsv.v / fade_step;
    const uint8_t fade_cycle_time_elapsed = fade_timing / fade_step;
    if (timer_elapsed(blink_cycle_timer) < static_on_time) {
        if (timer_elapsed(blink_fade_in_timer) > fade_cycle_time_elapsed && fade_in_step_counter < fade_step) {
            blink_hsv_value = blink_hsv_value + fade_value_decrement_size;
            set_blink_rgb(hue, sat, blink_hsv_value);
            fade_in_step_counter = fade_in_step_counter + 1;
            blink_fade_in_timer = timer_read();
        }
    } else {
        if (timer_elapsed(blink_fade_out_timer) > fade_cycle_time_elapsed && fade_out_step_counter < fade_step) {
            blink_hsv_value = blink_hsv_value - fade_value_decrement_size;
            set_blink_rgb(hue, sat, blink_hsv_value);
            fade_out_step_counter = fade_out_step_counter + 1;
            blink_fade_out_timer = timer_read();
        }
    }
    if (timer_elapsed(blink_cycle_timer) > static_on_time + static_off_time) {
        reset_blink_cycle();
    }
    rgb_matrix_set_color(led_index, recording_r, recording_g, recording_b);
}

/* Mac require a short delay before Capslock register in macros */
void registerCapsLock(void) {
    register_code(KC_CAPSLOCK);
    wait_ms(80);
    unregister_code(KC_CAPSLOCK);
}

/**************** LAYOUT *********************/

/*
[LEDS]
 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14
15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29
30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,       42,  43
44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,       56,  57
58,  59,  60,            61,                 62,  63,  64,  65,  66,  67

[KEYS]
ESC, 1,   2,   3,   4,   5,   6,   7,   8,   9,   0,   -,   =,  BCK, INS
TAB, Q,   W,   E,   R,   T,   Y,   U,   I,   O,   P,   [,   ],    \,PGUP
CPS, A,   S,   D,   F,   G,   H,   J,   K,   L,   COL, QOT,  RETURN,PGDN
SFT, Z,   X,   C,   V,   B,   N,   M,   COM, DOT, SLS, SHIFT,    UP, DEL
CTL, GUI, ALT,           SPACEBAR,           ALT, FN, CTL, LFT, DWN, RIT
*/

enum layers {
    _MAIN,
    _MAC,
    _FN
}

const layers_leds_map[] = {
    [_MAIN] = 45,
    [_MAC] = 46,
    [_FN] = 63
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_MAIN] = LAYOUT_65_ansi(
        KC_GESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC, KC_INS, 
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP, 
        KC_LEAD, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT, KC_PGDN, 
        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_UP, KC_DEL, 
        KC_LCTL, KC_LGUI, KC_LALT, KC_SPC, KC_RALT, MO(_FN), KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_MAC] = LAYOUT_65_ansi(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_LALT, KC_LGUI, KC_TRNS, KC_TRNS, MO(_FN), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),
    [_FN] = LAYOUT_65_ansi(
        DYN_REC_STOP, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_NO, KC_NO, 
        KC_TRNS, KC_MUTE, KC_VOLU, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RGB_SAD, RGB_SAI, DYN_MACRO_PLAY2, DYN_REC_START2, 
        KC_TRNS, KC_BRID, KC_VOLD, KC_BRIU, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RGB_HUD, RGB_HUI, DYN_MACRO_PLAY1, DYN_REC_START1, 
        KC_TRNS, TO(_MAIN), TO(_MAC), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RGB_SPD, RGB_SPI, KC_TRNS, RGB_VAI, KC_NO, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RGB_RMOD, RGB_VAD, RGB_MOD
    ),
}; 

//**************** MATRIX SCANS *********************//

void rgb_matrix_indicators_user(void) { 

    #ifdef RGB_MATRIX_ENABLE

    HSV hsvConfig = {rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v};
    RGB rgbConfig = hsv_to_rgb(hsvConfig);
    const uint8_t CONFIG_HSV[3] = {rgbConfig.r, rgbConfig.g, rgbConfig.b};

    HSV hsvConfigDim = {rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v * ledDimRatio};
    RGB rgbConfigDim = hsv_to_rgb(hsvConfigDim);
    const uint8_t CONFIG_HSV_DIM[3] = {rgbConfigDim.r, rgbConfigDim.g, rgbConfigDim.b};

    HSV hsvWhiteDim = {rgb_matrix_config.hsv.h, 0, rgb_matrix_config.hsv.v * ledDimRatio};
    RGB rgbWhiteDim = hsv_to_rgb(hsvWhiteDim);
    const uint8_t WHITE_HSV_DIM[3] = {rgbWhiteDim.r, rgbWhiteDim.g, rgbWhiteDim.b};

    HSV hsvRed = {0, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v};
    RGB rgbRed = hsv_to_rgb(hsvRed);
    const uint8_t RED_HSV[3] = {rgbRed.r, rgbRed.g, rgbRed.b};

    HSV hsvRedDim = {0, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v * ledDimRatio};
    RGB rgbRedDim = hsv_to_rgb(hsvRedDim);
    const uint8_t RED_HSV_DIM[3] = {rgbRedDim.r, rgbRedDim.g, rgbRedDim.b};

    HSV hsvGreenDim = {85, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v * ledDimRatio};
    RGB rgbGreenDim = hsv_to_rgb(hsvGreenDim);
    const uint8_t GREEN_HSV_DIM[3] = {rgbGreenDim.r, rgbGreenDim.g, rgbGreenDim.b};

    /* CapsLock LED indicator */
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        rgb_matrix_set_color(30, 0xFF, 0xFF, 0xFF);
    } 

    /* Current layer LED indicator */
    rgb_matrix_set_color(layers_leds_map[biton32(layer_state)], WHITE_HSV_DIM[0], WHITE_HSV_DIM[1], WHITE_HSV_DIM[2]); 

    /* Leader Key LED under-glow */
    if (isLeader) {
        rgb_matrix_set_color(14, CONFIG_HSV[0], CONFIG_HSV[1], CONFIG_HSV[2]);
        rgb_matrix_set_color(30, CONFIG_HSV[0], CONFIG_HSV[1], CONFIG_HSV[2]);
    } else {
        rgb_matrix_set_color(14, CONFIG_HSV_DIM[0], CONFIG_HSV_DIM[1], CONFIG_HSV_DIM[2]);
    }

    /* Blinking LED indicator when recording Dynamic Macro */
    if (isRecording && isBlinking) {
        get_this_led_blinking(0, false, 0, 255);
    }

    /* Blinking LED indicator when playing Dynamic Macro */
    if (isPlaying && isBlinking) {
        if (timer_elapsed(macro_one_play_timer) < macro_play_blink_timer || timer_elapsed(macro_two_play_timer) < macro_play_blink_timer) {
            get_this_led_blinking(0, true, 85, 255);
        } else {
            if (isBlinking && isPlaying) {
                reset_blink_status();
                reset_blink_cycle();
            }
        }
    }

    switch (biton32(layer_state)) {
        case _FN:
            /* Dynamic Macro LED indicator */
            if (isRecording) {
                rgb_matrix_set_color(0, RED_HSV[0], RED_HSV[1], RED_HSV[2]); /* macro stop */
            } else {
                rgb_matrix_set_color(43, RED_HSV_DIM[0], RED_HSV_DIM[1], RED_HSV_DIM[2]); /* macro 1 record */
                rgb_matrix_set_color(29, RED_HSV_DIM[0], RED_HSV_DIM[1], RED_HSV_DIM[2]); /* macro 2 record */
                rgb_matrix_set_color(42, GREEN_HSV_DIM[0], GREEN_HSV_DIM[1], GREEN_HSV_DIM[2]); /* macro 1 play */
                rgb_matrix_set_color(28, GREEN_HSV_DIM[0], GREEN_HSV_DIM[1], GREEN_HSV_DIM[2]); /* macro 2 play */
            }
            /* Layer LED indicators */
            rgb_matrix_set_color(45, WHITE_HSV_DIM[0], WHITE_HSV_DIM[1], WHITE_HSV_DIM[2]); /* Layer _MAIN */
            rgb_matrix_set_color(46, WHITE_HSV_DIM[0], WHITE_HSV_DIM[1], WHITE_HSV_DIM[2]); /* Layer _MAC */
            break; 
    }

    #endif /* RGB_MATRIX */
}
 
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool pressed = record->event.pressed;
    if (pressed) {
        switch (keycode) {
            case TO(_MAC):
                onMac = true;
                break;
            case TO(_MAIN):
                onMac = false;      
                break;
        }
    }
    return true;
} 

//**************** LEADER *********************//

#ifdef LEADER_ENABLE

LEADER_EXTERNS();

/* éÉ */
void send_e_aigu(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (onMac) {
            SEND_STRING(SS_TAP(X_CAPSLOCK) SS_LALT("e") SS_LSFT("e"));
            registerCapsLock();
        } else {
            SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P0) SS_TAP(X_P2) SS_TAP(X_P0) SS_TAP(X_P1) SS_UP(X_LALT));
        }
    } else {
        onMac ? SEND_STRING(SS_LALT("e") SS_TAP(X_E)) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P3) SS_TAP(X_P0) SS_UP(X_LALT));
    }
}

/* àÀ */
void lk_send_a_grave(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (onMac) {
            SEND_STRING(SS_TAP(X_CAPSLOCK) SS_LALT("`") SS_LSFT("a"));
            registerCapsLock();
        } else {
            SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P0) SS_TAP(X_P1) SS_TAP(X_P9) SS_TAP(X_P2) SS_UP(X_LALT));
        }
    } else {
        onMac ? SEND_STRING(SS_LALT("`") SS_TAP(X_A)) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P3) SS_TAP(X_P3) SS_UP(X_LALT));
    }
}

/* èÈ */
void lk_send_e_grave(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (onMac) {
            SEND_STRING(SS_TAP(X_CAPSLOCK) SS_LALT("`") SS_LSFT("e"));
            registerCapsLock();
        } else {
            SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P0) SS_TAP(X_P2) SS_TAP(X_P0) SS_TAP(X_P0) SS_UP(X_LALT));
        }
    } else {
        onMac ? SEND_STRING(SS_LALT("`") SS_TAP(X_E)) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P3) SS_TAP(X_P8) SS_UP(X_LALT));
    }
}

/* ùÙ */
void lk_send_u_grave(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (onMac) {
            SEND_STRING(SS_TAP(X_CAPSLOCK) SS_LALT("`") SS_LSFT("u"));
            registerCapsLock();
        } else {
            SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P0) SS_TAP(X_P2) SS_TAP(X_P1) SS_TAP(X_P7) SS_UP(X_LALT));
        }
    } else {
        onMac ? SEND_STRING(SS_LALT("`") SS_TAP(X_U)) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P5) SS_TAP(X_P1) SS_UP(X_LALT));
    }
}

/* âÂ */
void lk_send_a_circonflexe(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (onMac) {
            SEND_STRING(SS_TAP(X_CAPSLOCK) SS_LALT("i") SS_LSFT("a"));
            registerCapsLock();
        } else {
            SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P0) SS_TAP(X_P1) SS_TAP(X_P9) SS_TAP(X_P4) SS_UP(X_LALT));
        }
    } else {
        onMac ? SEND_STRING(SS_LALT("i") SS_TAP(X_A)) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P3) SS_TAP(X_P1) SS_UP(X_LALT));
    }
}

/* êÊ */
void lk_send_e_circonflexe(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (onMac) {
            SEND_STRING(SS_TAP(X_CAPSLOCK) SS_LALT("i") SS_LSFT("e"));
            registerCapsLock();
        } else {
            SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P0) SS_TAP(X_P2) SS_TAP(X_P0) SS_TAP(X_P2) SS_UP(X_LALT));
        }
    } else {
        onMac ? SEND_STRING(SS_LALT("i") SS_TAP(X_E)) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P3) SS_TAP(X_P6) SS_UP(X_LALT));
    }
}

/* íÍ */
void lk_send_i_circonflexe(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (onMac) {
            SEND_STRING(SS_TAP(X_CAPSLOCK) SS_LALT("i") SS_LSFT("i"));
            registerCapsLock();
        } else {
            SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P0) SS_TAP(X_P2) SS_TAP(X_P0) SS_TAP(X_P6) SS_UP(X_LALT));
        }
    } else {
        onMac ? SEND_STRING(SS_LALT("i") SS_TAP(X_I)) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P4) SS_TAP(X_P0) SS_UP(X_LALT));
    }
}

/* ôÔ */
void lk_send_o_circonflexe(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (onMac) {
            SEND_STRING(SS_TAP(X_CAPSLOCK) SS_LALT("i") SS_LSFT("o"));
            registerCapsLock();
        } else {
            SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P0) SS_TAP(X_P2) SS_TAP(X_P1) SS_TAP(X_P2) SS_UP(X_LALT));
        }
    } else {
        onMac ? SEND_STRING(SS_LALT("i") SS_TAP(X_O)) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P4) SS_TAP(X_P7) SS_UP(X_LALT));
    }
}

/* ûÛ */
void lk_send_u_circonflexe(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (onMac) {
            SEND_STRING(SS_TAP(X_CAPSLOCK) SS_LALT("i") SS_LSFT("u"));
            registerCapsLock();
        } else {
            SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P0) SS_TAP(X_P2) SS_TAP(X_P1) SS_TAP(X_P9) SS_UP(X_LALT));
        }
    } else {
        onMac ? SEND_STRING(SS_LALT("i") SS_TAP(X_U)) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P5) SS_TAP(X_P0) SS_UP(X_LALT));
    }
}

/* çÇ */
void lk_send_c_cedille(void) {
    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) 
    {
        onMac ? SEND_STRING(SS_LALT("c")) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P2) SS_TAP(X_P8) SS_UP(X_LALT));
    } else 
    {
        onMac ? SEND_STRING(SS_LALT("c")) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P1) SS_TAP(X_P3) SS_TAP(X_P5) SS_UP(X_LALT));
    }
}

/* (|) */
void lk_send_paranthesis_wrap_ini(void) {
    SEND_STRING("()" SS_TAP(X_LEFT));
}

/* (X) */
void lk_send_paranthesis_wrap_word(void) {
    onMac ? SEND_STRING(SS_LALT(SS_TAP(X_LEFT)) "(" SS_LALT(SS_TAP(X_RIGHT)) ")") : SEND_STRING(SS_LCTL(SS_TAP(X_LEFT)) "(" SS_LCTL(SS_TAP(X_RIGHT)) ")");
}

/* [|] */
void lk_send_bracket_wrap_ini(void) {
    SEND_STRING("[]" SS_TAP(X_LEFT));
}

/* [X] */
void lk_send_bracket_wrap_word(void) {
    onMac ? SEND_STRING(SS_LALT(SS_TAP(X_LEFT)) "[" SS_LALT(SS_TAP(X_RIGHT)) "]") : SEND_STRING(SS_LCTL(SS_TAP(X_LEFT)) "[" SS_LCTL(SS_TAP(X_RIGHT)) "]");
}

/* [|] */
void lk_send_curlybrace_wrap_ini(void) {
    SEND_STRING("{}" SS_TAP(X_LEFT));
}

/* [X] */
void lk_send_curlybrace_wrap_word(void) {
    onMac ? SEND_STRING(SS_LALT(SS_TAP(X_LEFT)) "{" SS_LALT(SS_TAP(X_RIGHT)) "}") : SEND_STRING(SS_LCTL(SS_TAP(X_LEFT)) "{" SS_LCTL(SS_TAP(X_RIGHT)) "}");
}


void matrix_scan_user(void) 
{
    LEADER_DICTIONARY() 
    {
        leading = false;
        leader_end();

        /* Sequences on layer _MAIN & _MAC */
        /*  éÉ      => LdrKey > / */
        SEQ_ONE_KEY(KC_SLSH) {
            send_e_aigu();
        }
        /*  éÉ      => LdrKey > E > E > E */
        SEQ_THREE_KEYS(KC_E, KC_E, KC_E) {
            send_e_aigu();
        }
        /*  àÀ      => LdrKey > ; > A */
        SEQ_TWO_KEYS(KC_SCLN, KC_A) {
            lk_send_a_grave();
        }
        /*  àÀ      => LdrKey > A */
        SEQ_ONE_KEY(KC_A) {
            lk_send_a_grave();
        }
        /*  èÈ      => LdrKey > ; > E */
        SEQ_TWO_KEYS(KC_SCLN, KC_E) {
            lk_send_e_grave();
        }
        /*  èÈ      => LdrKey > E */
        SEQ_ONE_KEY(KC_E) {
            lk_send_e_grave();
        }
        /*  ùÙ      => LdrKey > ; > U */
        SEQ_TWO_KEYS(KC_SCLN, KC_U) {
            lk_send_u_grave();
        }
        /*  ùÙ      => LdrKey > U */
        SEQ_ONE_KEY(KC_U) {
            lk_send_u_grave();
        }
        /*  âÂ      => LdrKey > [ > A */
        SEQ_TWO_KEYS(KC_LBRC, KC_A) {
            lk_send_a_circonflexe();
        }
        /*  âÂ      => LdrKey > A > A */
        SEQ_TWO_KEYS(KC_A, KC_A) {
            lk_send_a_circonflexe();
        }
        /*  êÊ      => LdrKey > [ > E */
        SEQ_TWO_KEYS(KC_LBRC, KC_E) {
            lk_send_e_circonflexe();
        }
        /*  êÊ      => LdrKey > E > E */
        SEQ_TWO_KEYS(KC_E, KC_E) {
            lk_send_e_circonflexe();
        }
        /*  îÎ      => LdrKey > [ > I */
        SEQ_TWO_KEYS(KC_LBRC, KC_I) {
            lk_send_i_circonflexe();
        }
        /*  îÎ      => LdrKey > I > I */
        SEQ_TWO_KEYS(KC_I, KC_I) {
            lk_send_i_circonflexe();
        }
        /*  ôÔ      => LdrKey > [ > O */
        SEQ_TWO_KEYS(KC_LBRC, KC_O) {
            lk_send_o_circonflexe();
        }
        /*  ôÔ      => LdrKey > O > O */
        SEQ_TWO_KEYS(KC_O, KC_O) {
            lk_send_o_circonflexe();
        }
        /*  ûÛ      => LdrKey > [ > U */
        SEQ_TWO_KEYS(KC_LBRC, KC_U) {
            lk_send_u_circonflexe();
        }
        /*  ûÛ      => LdrKey > U > U */
        SEQ_TWO_KEYS(KC_U, KC_U) {
            lk_send_u_circonflexe();
        }
        /*  çÇ      => LdrKey > ] > C */
        SEQ_TWO_KEYS(KC_RBRC, KC_C) {
            lk_send_c_cedille();
        }
        /*  çÇ      => LdrKey > C */
        SEQ_ONE_KEY(KC_C) {
            lk_send_c_cedille();
        }
        /*  CapsLock */
        SEQ_ONE_KEY(KC_LEAD) {
            registerCapsLock();
        }
        /*  ±       => LdrKey > = > - */
        SEQ_TWO_KEYS(KC_EQL, KC_MINS) { 
            onMac ? SEND_STRING(SS_LALT(SS_LSFT(SS_TAP(X_EQL)))) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P2) SS_TAP(X_P4) SS_TAP(X_P1) SS_UP(X_LALT)); 
        }
        /*  ≤       => LdrKey > - > = */
        SEQ_TWO_KEYS(KC_MINS, KC_EQL) { 
            onMac ? SEND_STRING(SS_LALT(SS_TAP(X_COMM))) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P2) SS_TAP(X_P4) SS_TAP(X_P3) SS_UP(X_LALT)); 
        }
        /*  ≥       => LdrKey > = > = */
        SEQ_TWO_KEYS(KC_EQL, KC_EQL) { 
            onMac ? SEND_STRING(SS_LALT(SS_TAP(X_DOT))) : SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_P2) SS_TAP(X_P4) SS_TAP(X_P2) SS_UP(X_LALT)); 
        }
        /*  <=      => LdrKey > , > , */
        SEQ_TWO_KEYS(KC_COMM, KC_COMM) { 
            SEND_STRING(SS_LSFT(SS_TAP(X_COMM)) SS_TAP(X_SPC) SS_TAP(X_EQL) SS_TAP(X_LEFT) SS_TAP(X_BSPC) SS_TAP(X_RIGHT)); 
        }
        /*  =>      => LdrKey > . > . */
        SEQ_TWO_KEYS(KC_DOT, KC_DOT) { 
            SEND_STRING("=>"); 
        }
        /*  ", "    => LdrKey > " " */
        SEQ_ONE_KEY(KC_SPC) { 
            SEND_STRING(", "); 
        }
        /*  ". "    => LdrKey > " " > " " */
        SEQ_TWO_KEYS(KC_SPC, KC_SPC) { 
            SEND_STRING(". "); 
        }
        /*  Backward delete current word (on cursor) */
        SEQ_TWO_KEYS(KC_BSPC, KC_BSPC) { 
            onMac ? SEND_STRING(SS_LALT(SS_TAP(X_RIGHT)) SS_LALT(SS_LSFT(SS_TAP(X_LEFT))) SS_TAP(X_BSPC)) : SEND_STRING(SS_LCTL(SS_TAP(X_RIGHT)) SS_LCTL(SS_LSFT(SS_TAP(X_LEFT))) SS_TAP(X_BSPC)); 
        }
        /*  Previous word delete */
        SEQ_ONE_KEY(KC_BSPC) { 
            onMac ? SEND_STRING(SS_LALT(SS_LSFT(SS_TAP(X_LEFT))) SS_TAP(X_BSPC)) : SEND_STRING(SS_LCTL(SS_LSFT(SS_TAP(X_LEFT))) SS_TAP(X_BSPC)); 
        }
        /*  Forward delete current word (on cursor) */
        SEQ_TWO_KEYS(KC_DEL, KC_DEL) { 

            onMac ? SEND_STRING(SS_LALT(SS_TAP(X_LEFT)) SS_LALT(SS_LSFT(SS_TAP(X_RIGHT))) SS_TAP(X_DEL)) : SEND_STRING(SS_LCTL(SS_TAP(X_LEFT)) SS_LCTL(SS_LSFT(SS_TAP(X_RIGHT))) SS_TAP(X_DEL)); 
        }
        /*  Next word delete */
        SEQ_ONE_KEY(KC_DEL) { 
            onMac ? SEND_STRING(SS_LALT(SS_LSFT(SS_TAP(X_RIGHT))) SS_TAP(X_DEL)): SEND_STRING(SS_LCTL(SS_LSFT(SS_TAP(X_RIGHT))) SS_TAP(X_DEL)); 
        }
        /*  `   => LdrKey > Escape */
        SEQ_ONE_KEY(KC_GESC) {
            SEND_STRING("`");
        }
        /*  ``` => LdrKey > Escape > Escape > Escape */
        SEQ_THREE_KEYS(KC_GESC, KC_GESC, KC_GESC) {
            SEND_STRING("```");
        }
        /*  Printscreen             => LdrKey > Insert */
        SEQ_ONE_KEY(KC_INS) {
            onMac ? SEND_STRING(SS_LGUI(SS_LSFT(SS_TAP(X_4)))) : SEND_STRING(SS_TAP(X_PSCR));
        }
        /*  Home                    => LdrKey > Page Up */
        SEQ_ONE_KEY(KC_PGUP) {
            onMac ? SEND_STRING(SS_TAP(X_HOME)) : SEND_STRING(SS_LCTL(SS_TAP(X_HOME)));
        }
        /*  End                     => LdrKey > Page Down */
        SEQ_ONE_KEY(KC_PGDN) {
            onMac ? SEND_STRING(SS_TAP(X_END)) : SEND_STRING(SS_LCTL(SS_TAP(X_END)));
        }
        /*  Close Curernt File/Tab  => LdrKey > W */
        SEQ_ONE_KEY(KC_W) {
            onMac ? SEND_STRING(SS_LGUI(SS_TAP(X_W))) : SEND_STRING(SS_LCTL(SS_TAP(X_W)));
        }
        /*  Close Current App       => LdrKey > Q */
        SEQ_ONE_KEY(KC_Q) {
            onMac ? SEND_STRING(SS_LGUI(SS_TAP(X_Q))) : SEND_STRING(SS_LALT(SS_TAP(X_F4)));
        }
        /*  Show Desktop            => LdrKey > D */
        SEQ_ONE_KEY(KC_D) {
            onMac ? SEND_STRING(SS_LGUI(SS_TAP(X_SPC)) "Mission" SS_TAP(X_ENT)) : SEND_STRING(SS_LGUI(SS_TAP(X_D)));
        }
        /*  "           => LdrKey > ' */
        SEQ_ONE_KEY(KC_QUOT) {
            SEND_STRING("\"");
        }
        /*  "|"         => LdrKey > ' > ' */
        SEQ_TWO_KEYS(KC_QUOT, KC_QUOT) {
            SEND_STRING("\"\"" SS_TAP(X_LEFT));
        }
        /*  "X" wrap    => LdrKey > ' > ' > ' */
        SEQ_THREE_KEYS(KC_QUOT, KC_QUOT, KC_QUOT) {
            onMac ? SEND_STRING(SS_LALT(SS_TAP(X_LEFT)) "\"" SS_LALT(SS_TAP(X_RIGHT)) "\"") : SEND_STRING(SS_LCTL(SS_TAP(X_LEFT)) "\"" SS_LCTL(SS_TAP(X_RIGHT)) "\"");
        }
        /*  (           => LdrKey > Left Shift */
        SEQ_ONE_KEY(KC_LSFT) {
            SEND_STRING("(");
        }
        /*  )           => LdrKey > Right Shift */
        SEQ_ONE_KEY(KC_RSFT) {
            SEND_STRING(")");
        }
        /*  (|)         => LdrKey > Left Shift > Left Shift */
        SEQ_TWO_KEYS(KC_LSFT, KC_LSFT) {
            lk_send_paranthesis_wrap_ini();
        }
        /*  (|)         => LdrKey > Right Shift > Right Shift */
        SEQ_TWO_KEYS(KC_RSFT, KC_RSFT) {
            lk_send_paranthesis_wrap_ini();
        }
        /*  (X) wrap    => LdrKey > Left Shift > Left Shift > Left Shift */
        SEQ_THREE_KEYS(KC_LSFT, KC_LSFT, KC_LSFT) {
            lk_send_paranthesis_wrap_word();
        }
        /*  (X) wrap    => LdrKey > Right Shift > Right Shift > Right Shift */
        SEQ_THREE_KEYS(KC_RSFT, KC_RSFT, KC_RSFT) {
            lk_send_paranthesis_wrap_word();
        }
        /*  [           => LdrKey > Left CTL */
        SEQ_ONE_KEY(KC_LCTL) {
            SEND_STRING("[");
        }
        /*  ]           => LdrKey > Right CTL */
        SEQ_ONE_KEY(KC_RCTL) {
            SEND_STRING("]");
        }
        /*  [|]         => LdrKey > Left CTL > Left CTL */
        SEQ_TWO_KEYS(KC_LCTL, KC_LCTL) {
            lk_send_bracket_wrap_ini();
        }
        /*  [|]         => LdrKey > Right CTL > Right CTL */
        SEQ_TWO_KEYS(KC_RCTL, KC_RCTL) {
            lk_send_bracket_wrap_ini();
        }
        /*  [X] wrap    => LdrKey > Left CTL > Left CTL > Left CTL */
        SEQ_THREE_KEYS(KC_LCTL, KC_LCTL, KC_LCTL) {
            lk_send_bracket_wrap_word();
        }
        /*  [X] wrap    => LdrKey > Right CTL > Right CTL > Right CTL */
        SEQ_THREE_KEYS(KC_RCTL, KC_RCTL, KC_RCTL) {
            lk_send_bracket_wrap_word();
        }
        /*  {           => LdrKey > Left ALT */
        SEQ_ONE_KEY(KC_LALT) {
            SEND_STRING("{");
        }
        /*  }           => LdrKey > Right ALT */
        SEQ_ONE_KEY(KC_RALT) {
            SEND_STRING("}");
        }
        /*  {|}         => LdrKey > Left ALT > Left ALT */
        SEQ_TWO_KEYS(KC_LALT, KC_LALT) {
            lk_send_curlybrace_wrap_ini();
        }
        /*  {|}         => LdrKey > Right ALT > Right ALT */
        SEQ_TWO_KEYS(KC_RALT, KC_RALT) {
            lk_send_curlybrace_wrap_ini();
        }
        /*  {X} wrap    => LdrKey > Left ALT > Left ALT > Left ALT */
        SEQ_THREE_KEYS(KC_LALT, KC_LALT, KC_LALT) {
            lk_send_curlybrace_wrap_word();
        }
        /*  {X} wrap    => LdrKey > Right ALT > Right ALT > Right ALT */
        SEQ_THREE_KEYS(KC_RALT, KC_RALT, KC_RALT) {
            lk_send_curlybrace_wrap_word();
        }
        /*  Select everything on this line before cursor => LdrKey > Left */
        SEQ_ONE_KEY(KC_LEFT) { 
            onMac ? SEND_STRING(SS_LSFT(SS_LGUI(SS_TAP(X_LEFT)))) : SEND_STRING(SS_LSFT(SS_TAP(X_HOME)));
        }
        /*  Select everything on this line after cursor  => LdrKey > Right */
        SEQ_ONE_KEY(KC_RIGHT) { 
            onMac ? SEND_STRING(SS_LSFT(SS_LGUI(SS_TAP(X_RIGHT)))) : SEND_STRING(SS_LSFT(SS_TAP(X_END)));
        }
        /*  Select everything on this line before cursor and bring on previous line => LdrKey > Left > Left */
        SEQ_TWO_KEYS(KC_LEFT, KC_LEFT) { 
            onMac ? SEND_STRING(SS_LSFT(SS_TAP(X_UP) SS_LGUI(SS_TAP(X_RIGHT)))) : SEND_STRING(SS_LSFT(SS_TAP(X_UP) SS_TAP(X_END))); 
        }
        /*  Select everything on this line  => LdrKey > Right > Left */
        SEQ_TWO_KEYS(KC_RIGHT, KC_LEFT) { 
            onMac ? SEND_STRING(SS_LGUI(SS_TAP(X_RIGHT) SS_LSFT(SS_LGUI(SS_TAP(X_LEFT))))) : SEND_STRING(SS_TAP(X_END) SS_LSFT(SS_TAP(X_HOME))); 
        }
        /*  Select 1x Page Up on the page before the cursor  => LdrKey > Up */
        SEQ_ONE_KEY(KC_UP) { 
            SEND_STRING(SS_LSFT(SS_TAP(X_PGUP)));
        }
        /*  Select 1x Page Down on the page after the cursor => LdrKey > Down */
        SEQ_ONE_KEY(KC_DOWN) { 
            SEND_STRING(SS_LSFT(SS_TAP(X_PGDN)));   
        }
        /*  Select everything on the page before the cursor => LdrKey > Up > Up */
        SEQ_TWO_KEYS(KC_UP, KC_UP) { 
            onMac ? SEND_STRING(SS_LSFT(SS_LGUI(SS_TAP(X_UP)))) : SEND_STRING(SS_LSFT(SS_LCTL(SS_TAP(X_HOME))));
        }
        /*  Select everything on the page after the cursor => LdrKey > Down > Down */
        SEQ_TWO_KEYS(KC_DOWN, KC_DOWN) { 
            onMac ? SEND_STRING(SS_LSFT(SS_LGUI(SS_TAP(X_DOWN)))) : SEND_STRING(SS_LSFT(SS_LCTL(SS_TAP(X_END))));
        }
        /* HELPER => spit out the url of the layout description page on github */
        SEQ_FIVE_KEYS(KC_GESC, KC_GESC, KC_GESC, KC_GESC, KC_GESC) { 
            // SEND_STRING("https://github.com/qmk/qmk_firmware/tree/master/keyboards/dztech/dz65rgb/keymaps/drootz"); // Pending Pull Request #8199
            SEND_STRING("https://github.com/drootz/qmk_firmware/tree/dz65_drootz_tmppr/keyboards/dztech/dz65rgb/keymaps/drootz");
        }
        /*  google.ca   => LdrKey > G > G */
        SEQ_TWO_KEYS(KC_G, KC_G) { 
            SEND_STRING("https://google.ca" SS_TAP(X_ENT)); 
        }
        /*  @gmail  => LdrKey > M > L > T */
        SEQ_THREE_KEYS(KC_M, KC_L, KC_T) { 
            SEND_STRING("mailto." SS_TAP(X_D) SS_TAP(X_A) SS_TAP(X_N) SS_TAP(X_I) SS_TAP(X_E) SS_TAP(X_L) SS_TAP(X_R) SS_TAP(X_A) SS_TAP(X_C) SS_TAP(X_I) SS_TAP(X_N) SS_TAP(X_E) "@gmail.com"); 
        }
    }
}

void leader_start(void) {
    isLeader = true;
}

void leader_end(void) {
    isLeader = false;
}

#endif /* LEADER */

/**************** DYNAMIC MACRO *********************/

void dynamic_macro_record_start_user(void) {
    onMac = false; /* reset layer bool as dynamic macro clear the keyboard and reset layers. */
    if (!isBlinking && !isRecording) {
        reset_blink_cycle();
        isBlinking = true;
        isRecording = true;
    }
}

void dynamic_macro_record_end_user(int8_t direction) {
    if (isBlinking && isRecording) {
        reset_blink_status();
        reset_blink_cycle();
    }
}

void dynamic_macro_play_user(int8_t direction) {
    switch (direction) {
        case 1:
            if (!isBlinking && !isPlaying) {
                reset_blink_cycle();
                isBlinking = true;
                isPlaying = true;
                macro_one_play_timer = timer_read();
            }
            break;
        case -1:
            if (!isBlinking && !isPlaying) {
                reset_blink_cycle();
                isBlinking = true;
                isPlaying = true;
                macro_two_play_timer = timer_read();
            }
            break;
    }
}
