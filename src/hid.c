// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <tusb.h>
#include "config.h"
#include "hid.h"
#include "profile.h"
#include "xinput.h"
#include "helper.h"
#include "thanks.c"

bool hid_allow_communication = true;  // Extern.
bool synced_keyboard = false;
bool synced_mouse = false;
bool synced_gamepad = false;
uint16_t alarms = 0;
alarm_pool_t *alarm_pool;

uint8_t state_matrix[256] = {0,};
int16_t mouse_x = 0;
int16_t mouse_y = 0;
int16_t gamepad_lx = 0;
int16_t gamepad_ly = 0;
int16_t gamepad_lz = 0;
int16_t gamepad_rx = 0;
int16_t gamepad_ry = 0;
int16_t gamepad_rz = 0;

void hid_matrix_reset() {
    for(uint8_t i=0; i<255; i++) {
        state_matrix[i] = 0;
    }
    synced_keyboard = false;
    synced_mouse = false;
    synced_gamepad = false;
}

void hid_procedure_press(uint8_t procedure){
    if (procedure == PROC_HOME) profile_set_home(true);
    if (procedure == PROC_PROFILE_1) profile_set_active(1);
    if (procedure == PROC_PROFILE_2) profile_set_active(2);
    if (procedure == PROC_PROFILE_3) profile_set_active(3);
    if (procedure == PROC_PROFILE_4) profile_set_active(4);
    if (procedure == PROC_PROFILE_5) profile_set_active(5);
    if (procedure == PROC_PROFILE_6) profile_set_active(6);
    if (procedure == PROC_PROFILE_7) profile_set_active(7);
    if (procedure == PROC_PROFILE_8) profile_set_active(8);
    if (procedure == PROC_PROFILE_9) profile_set_active(9);
    if (procedure == PROC_PROFILE_10) profile_set_active(10);
    if (procedure == PROC_PROFILE_11) profile_set_active(11);
    if (procedure == PROC_PROFILE_12) profile_set_active(12);
    if (procedure == PROC_TUNE_UP) config_tune(1);
    if (procedure == PROC_TUNE_DOWN) config_tune(0);
    if (procedure == PROC_TUNE_OS) config_tune_set_mode(procedure);
    if (procedure == PROC_TUNE_SENSITIVITY) config_tune_set_mode(procedure);
    if (procedure == PROC_TUNE_DEADZONE) config_tune_set_mode(procedure);
    if (procedure == PROC_TUNE_TOUCH_THRESHOLD) config_tune_set_mode(procedure);
    if (procedure == PROC_TUNE_VIBRATION) config_tune_set_mode(procedure);
    if (procedure == PROC_CALIBRATE) config_calibrate();
    if (procedure == PROC_BOOTSEL) config_bootsel();
    if (procedure == PROC_THANKS) hid_thanks();
}

void hid_procedure_release(uint8_t procedure) {
    if (procedure == PROC_HOME) profile_set_home(false);
}

void hid_press(uint8_t key) {
    if (key == KEY_NONE) return;
    else if (key >= PROC_INDEX) hid_procedure_press(key);
    else {
        state_matrix[key] += 1;
        if (key >= GAMEPAD_INDEX) synced_gamepad = false;
        else if (key >= MOUSE_INDEX) synced_mouse = false;
        else synced_keyboard = false;
    }
}

void hid_release(uint8_t key) {
    if (key == KEY_NONE) return;
    else if (key == MOUSE_SCROLL_UP) return;
    else if (key == MOUSE_SCROLL_DOWN) return;
    else if (key >= PROC_INDEX) hid_procedure_release(key);
    else {
        state_matrix[key] -= 1;
        if (key >= GAMEPAD_INDEX) synced_gamepad = false;
        else if (key >= MOUSE_INDEX) synced_mouse = false;
        else synced_keyboard = false;
    }
}

void hid_press_multiple(uint8_t *keys) {
    if (keys[0] == PROC_MACRO) {
        if (alarms > 0) return;  // Disallows parallel macros. TODO fix.
        uint16_t time = 10;
        for(uint8_t i=1; i<MACROS_LEN; i++) {
            if (keys[i] == 0) break;
            hid_press_later(keys[i], time);
            time += 10;
            hid_release_later(keys[i], time);
            time += 10;
        }
    } else {
        for(uint8_t i=0; i<ACTIONS_LEN; i++) {
            if (keys[i] == 0) return;
            hid_press(keys[i]);
        }
    }
}

void hid_release_multiple(uint8_t *keys) {
    if (keys[0] == PROC_MACRO) return;
    for(uint8_t i=0; i<ACTIONS_LEN; i++) {
        if (keys[i] == 0) return;
        hid_release(keys[i]);
    }
}

void hid_press_later(uint8_t key, uint16_t delay) {
    alarm_pool_add_alarm_in_ms(
        alarm_pool,
        delay,
        (alarm_callback_t)hid_press_later_callback,
        (void*)(uint32_t)key,
        true
    );
    alarms++;
}

void hid_release_later(uint8_t key, uint16_t delay) {
    alarm_pool_add_alarm_in_ms(
        alarm_pool,
        delay,
        (alarm_callback_t)hid_release_later_callback,
        (void*)(uint32_t)key,
        true
    );
    alarms++;
}

void hid_press_multiple_later(uint8_t *keys, uint16_t delay) {
    alarm_pool_add_alarm_in_ms(
        alarm_pool,
        delay,
        (alarm_callback_t)hid_press_multiple_later_callback,
        keys,
        true
    );
    alarms++;
}

void hid_release_multiple_later(uint8_t *keys, uint16_t delay) {
    alarm_pool_add_alarm_in_ms(
        alarm_pool,
        delay,
        (alarm_callback_t)hid_release_multiple_later_callback,
        keys,
        true
    );
    alarms++;
}

void hid_press_later_callback(alarm_id_t alarm, uint8_t key) {
    alarm_pool_cancel_alarm(alarm_pool, alarm);
    hid_press(key);
    alarms--;
}

void hid_release_later_callback(alarm_id_t alarm, uint8_t key) {
    alarm_pool_cancel_alarm(alarm_pool, alarm);
    hid_release(key);
    alarms--;
}

void hid_press_multiple_later_callback(alarm_id_t alarm, uint8_t *keys) {
    alarm_pool_cancel_alarm(alarm_pool, alarm);
    hid_press_multiple(keys);
    alarms--;
}

void hid_release_multiple_later_callback(alarm_id_t alarm, uint8_t *keys) {
    alarm_pool_cancel_alarm(alarm_pool, alarm);
    hid_release_multiple(keys);
    alarms--;
}

bool hid_is_axis(uint8_t key) {
    return is_between(key, GAMEPAD_AXIS_INDEX, PROC_INDEX-1);
}

void hid_mouse_move(int16_t x, int16_t y) {
    mouse_x += x;
    mouse_y += y;
    synced_mouse = false;
}

void hid_gamepad_lx(int16_t value) {
    if (value == gamepad_lx) return;
    gamepad_lx = value;
    synced_gamepad = false;
}

void hid_gamepad_ly(int16_t value) {
    if (value == gamepad_ly) return;
    gamepad_ly = value;
    synced_gamepad = false;
}

void hid_gamepad_lz(int16_t value) {
    if (value == gamepad_lz) return;
    gamepad_lz = value;
    synced_gamepad = false;
}

void hid_gamepad_rx(int16_t value) {
    if (value == gamepad_rx) return;
    gamepad_rx = value;
    synced_gamepad = false;
}

void hid_gamepad_ry(int16_t value) {
    if (value == gamepad_ry) return;
    gamepad_ry = value;
    synced_gamepad = false;
}

void hid_gamepad_rz(int16_t value) {
    if (value == gamepad_rz) return;
    gamepad_rz = value;
    synced_gamepad = false;
}

void hid_mouse_report() {
    int8_t buttons = 0;
    for(int i=0; i<5; i++) {
        buttons += state_matrix[MOUSE_INDEX + i] << i;
    }
    int8_t report_x = 0;
    int8_t report_y = 0;
    if (mouse_x >= 127) {
        report_x = 127;
        mouse_x -= 127;
    } else if (mouse_x <= -127) {
        report_x = -127;
        mouse_x += 127;
    } else {
        report_x = (int8_t)mouse_x;
        mouse_x = 0;
    }
    if (mouse_y >= 127) {
        report_y = 127;
        mouse_y -= 127;
    } else if (mouse_y <= -127) {
        report_y = -127;
        mouse_y += 127;
    } else {
        report_y = (int8_t)mouse_y;
        mouse_y = 0;
    }
    uint8_t mouse_z = state_matrix[MOUSE_SCROLL_UP] - state_matrix[MOUSE_SCROLL_DOWN];
    state_matrix[MOUSE_SCROLL_UP] = 0;
    state_matrix[MOUSE_SCROLL_DOWN] = 0;
    tud_hid_mouse_report(
        REPORT_MOUSE,
        buttons,
        report_x,
        report_y,
        mouse_z,
        0
    );
}

void hid_keyboard_report() {
    uint8_t report[6] = {0};
    uint8_t keys_available = 6;
    for(int i=0; i<=115; i++) {
        if (state_matrix[i] >= 1) {
            report[keys_available - 1] = (uint8_t)i;
            keys_available--;
            if (keys_available == 0) {
                break;
            }
        }
    }
    uint8_t modifier = 0;
    for(int i=0; i<8; i++) {
        modifier += !!state_matrix[MODIFIER_INDEX + i] << i;
    }
    tud_hid_keyboard_report(
        REPORT_KEYBOARD,
        modifier,
        report
    );
}

int16_t hid_axis(
    int16_t value,
    uint8_t matrix_index_pos,
    uint8_t matrix_index_neg
) {
    if (state_matrix[matrix_index_pos]) return 32767;
    if (matrix_index_neg != 0 && state_matrix[matrix_index_neg]) return -32767;
    return value;
}

void hid_gamepad_report() {
    int32_t buttons = 0;
    for(int i=0; i<16; i++) {
        buttons += state_matrix[GAMEPAD_INDEX + i] << i;
    }
    int16_t lx_report = hid_axis(gamepad_lx, GAMEPAD_AXIS_LX, GAMEPAD_AXIS_LX_NEG);
    int16_t ly_report = hid_axis(gamepad_ly, GAMEPAD_AXIS_LY, GAMEPAD_AXIS_LY_NEG);
    int16_t rx_report = hid_axis(gamepad_rx, GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RX_NEG);
    int16_t ry_report = hid_axis(gamepad_ry, GAMEPAD_AXIS_RY, GAMEPAD_AXIS_RY_NEG);
    int16_t lz_report = hid_axis(gamepad_lz, GAMEPAD_AXIS_LZ, 0);
    int16_t rz_report = hid_axis(gamepad_rz, GAMEPAD_AXIS_RZ, 0);
    hid_gamepad_report_t report = {
        lx_report / 256,
        ly_report / 256,
        rx_report / 256,
        ry_report / 256,
        lz_report / 256,
        rz_report / 256,
        0,
        buttons,
    };
    tud_hid_report(REPORT_GAMEPAD, &report, sizeof(report));
}

void hid_xinput_report() {
    int8_t buttons_0 = 0;
    int8_t buttons_1 = 0;
    for(int i=0; i<8; i++) {
        buttons_0 += state_matrix[GAMEPAD_INDEX + i] << i;
    }
    for(int i=0; i<8; i++) {
        buttons_1 += state_matrix[GAMEPAD_INDEX + i + 8] << i;
    }
    int16_t lx_report = hid_axis(gamepad_lx, GAMEPAD_AXIS_LX, GAMEPAD_AXIS_LX_NEG);
    int16_t ly_report = hid_axis(gamepad_ly, GAMEPAD_AXIS_LY, GAMEPAD_AXIS_LY_NEG);
    int16_t rx_report = hid_axis(gamepad_rx, GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RX_NEG);
    int16_t ry_report = hid_axis(gamepad_ry, GAMEPAD_AXIS_RY, GAMEPAD_AXIS_RY_NEG);
    int16_t lz_report = hid_axis(gamepad_lz, GAMEPAD_AXIS_LZ, 0);
    int16_t rz_report = hid_axis(gamepad_rz, GAMEPAD_AXIS_RZ, 0);
    xinput_report report = {
        .report_id   = 0,
        .report_size = XINPUT_REPORT_SIZE,
        .buttons_0   = buttons_0,
        .buttons_1   = buttons_1,
        .lz          = lz_report,
        .rz          = rz_report,
        .lx          = lx_report,
        .ly          = -ly_report,
        .rx          = rx_report,
        .ry          = -ry_report,
        .reserved    = {0, 0, 0, 0, 0, 0}
    };
    xinput_send_report(&report);
}

void hid_report() {
    static bool is_tud_ready = false;
    static bool is_tud_ready_logged = false;
    static uint8_t priority_mouse = 0;
    static uint8_t priority_gamepad = 0;

    if (!synced_mouse) priority_mouse += 1 * CFG_HID_REPORT_PRIORITY_RATIO;
    if (!synced_gamepad) priority_gamepad += 1;

    if (!hid_allow_communication) return;
    tud_task();
    if (tud_ready()) {
        is_tud_ready = true;
        if (!is_tud_ready_logged) {
            is_tud_ready_logged = true;
            // hid_matrix_reset();
            printf("USB: tud_ready TRUE\n");
        }

        // xinput_receive_report();

        if (tud_hid_ready()) {
            if (!synced_keyboard) {
                hid_keyboard_report();
                synced_keyboard = true;
                return;
            }
            if (!synced_mouse && (priority_mouse > priority_gamepad)) {
                hid_mouse_report();
                synced_mouse = true;
                priority_mouse = 0;
                return;
            }
            if (!synced_gamepad && config_get_os_mode() == OS_MODE_GENERIC) {
                hid_gamepad_report();
                synced_gamepad = true;
                priority_gamepad = 0;
                return;
            }
        }

        if (!synced_gamepad && config_get_os_mode() != OS_MODE_GENERIC) {
            if (tud_suspended()) {
                tud_remote_wakeup();
            }
            hid_xinput_report();
            synced_gamepad = true;
            priority_gamepad = 0;
            return;
        }
    } else {
        is_tud_ready = false;
        if (is_tud_ready_logged) {
            is_tud_ready_logged = false;
            printf("USB: tud_ready FALSE\n");
        }
    }
}

// A not-so-secret easter egg.
void hid_thanks_(alarm_id_t alarm) {
    cancel_alarm(alarm);
    static uint8_t x = 0;
    static bool p = 0;
    static uint8_t r;
    if (x == 0 && p == false) {
        r = random8() % thanks_len;
    }
    if (thanks_list[r][x] == 0) {
        x = 0;
        p = 0;
        return;
    }
    if (!p) {
        hid_press(thanks_list[r][x]);
        p = true;
    } else if (p) {
        hid_release(thanks_list[r][x]);
        p = false;
        x += 1;
    }
    add_alarm_in_ms(5, (alarm_callback_t)hid_thanks_, NULL, true);
}

void hid_thanks() {
    add_alarm_in_ms(5, (alarm_callback_t)hid_thanks_, NULL, true);
}

void hid_init() {
    printf("INIT: HID\n");
    alarm_pool = alarm_pool_create(2, 255);
}
