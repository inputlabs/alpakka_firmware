// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <tusb.h>
#include "config.h"
#include "hid.h"
#include "profile.h"
#include "xinput.h"
#include "helper.h"

bool synced_keyboard = false;
bool synced_mouse = false;
bool synced_gamepad = false;

uint8_t state_matrix[256] = {0,};
int16_t mouse_x = 0;
int16_t mouse_y = 0;
int8_t mouse_z = 0;
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
    if (procedure == PROC_TUNE_VIBRATION) config_tune_set_mode(procedure);
    if (procedure == PROC_CALIBRATE) config_calibrate();
    if (procedure == PROC_BOOTSEL) config_bootsel();
    if (procedure == PROC_THANKS) hid_thanks();
}

void hid_procedure_release(uint8_t procedure) {
    if (procedure == PROC_HOME) profile_set_home(false);
}

void hid_press(uint8_t key) {
    if (key == 0 || key == KEY_NONE) return;
    else if (key >= PROC_INDEX) hid_procedure_press(key);
    else {
        state_matrix[key] += 1;
        if (key >= GAMEPAD_INDEX) synced_gamepad = false;
        else if (key >= MOUSE_INDEX) synced_mouse = false;
        else synced_keyboard = false;
    }
}

void hid_release(uint8_t key) {
    if (key == 0) return;
    else if (key >= PROC_INDEX) hid_procedure_release(key);
    else {
        state_matrix[key] -= 1;
        if (key >= GAMEPAD_INDEX) synced_gamepad = false;
        else if (key >= MOUSE_INDEX) synced_mouse = false;
        else synced_keyboard = false;
    }
}

void hid_press_multiple(uint8_t *keys) {
    for(uint8_t i=0; i<4; i++) {
        if (keys[i] == 0) return;
        hid_press(keys[i]);
    }
}

void hid_release_multiple(uint8_t *keys) {
    for(uint8_t i=0; i<4; i++) {
        if (keys[i] == 0) return;
        hid_release(keys[i]);
    }
}

void hid_press_multiple_delayed(alarm_id_t alarm, uint8_t *keys) {
    cancel_alarm(alarm);
    for(uint8_t i=0; i<4; i++) {
        if (keys[i] == 0) return;
        hid_press(keys[i]);
    }
}

void hid_release_multiple_delayed(alarm_id_t alarm, uint8_t *keys) {
    cancel_alarm(alarm);
    for(uint8_t i=0; i<4; i++) {
        if (keys[i] == 0) return;
        hid_release(keys[i]);
    }
}

void hid_mouse_move(int16_t x, int16_t y) {
    mouse_x += x;
    mouse_y += y;
    synced_mouse = false;
}

void hid_mouse_wheel(int8_t z) {
    mouse_z += z;
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
    tud_hid_mouse_report(
        REPORT_MOUSE,
        buttons,
        report_x,
        report_y,
        mouse_z,
        0
    );
    mouse_z = 0;
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

void hid_gamepad_report() {
    int8_t buttons_0 = 0;
    int8_t buttons_1 = 0;
    for(int i=0; i<8; i++) {
        buttons_0 += state_matrix[GAMEPAD_INDEX + i] << i;
    }
    for(int i=0; i<8; i++) {
        buttons_1 += state_matrix[GAMEPAD_INDEX + i + 8] << i;
    }
    int16_t gamepad_lx_report = 0;
    int16_t gamepad_ly_report = 0;
    int16_t gamepad_lz_report = 0;
    int16_t gamepad_rx_report = 0;
    int16_t gamepad_ry_report = 0;
    int16_t gamepad_rz_report = 0;
    // LX
    if (state_matrix[GAMEPAD_AXIS_LX]) gamepad_lx_report = 32767;
    else if (state_matrix[GAMEPAD_AXIS_LX_NEG]) gamepad_lx_report = -32767;
    else gamepad_lx_report = gamepad_lx;
    // LY
    if (state_matrix[GAMEPAD_AXIS_LY]) gamepad_ly_report = 32767;
    else if (state_matrix[GAMEPAD_AXIS_LY_NEG]) gamepad_ly_report = -32767;
    else gamepad_ly_report = gamepad_ly;
    // RX
    if (state_matrix[GAMEPAD_AXIS_RX]) gamepad_rx_report = 32767;
    else if (state_matrix[GAMEPAD_AXIS_RX_NEG]) gamepad_rx_report = -32767;
    else gamepad_rx_report = gamepad_rx;
    // RY
    if (state_matrix[GAMEPAD_AXIS_RY]) gamepad_ry_report = 32767;
    else if (state_matrix[GAMEPAD_AXIS_RY_NEG]) gamepad_ry_report = -32767;
    else gamepad_ry_report = gamepad_ry;
    // LZ
    if (state_matrix[GAMEPAD_AXIS_LZ]) gamepad_lz_report = 255;
    else gamepad_lz_report = gamepad_lz;
    // RZ
    if (state_matrix[GAMEPAD_AXIS_RZ]) gamepad_rz_report = 255;
    else gamepad_rz_report = gamepad_rz;
    // Report.
    xinput_report report = {
        .report_id   = 0,
        .report_size = XINPUT_REPORT_SIZE,
        .buttons_0   = buttons_0,
        .buttons_1   = buttons_1,
        .lz          = gamepad_lz_report,
        .rz          = gamepad_rz_report,
        .lx          = gamepad_lx_report,
        .ly          = -gamepad_ly_report,
        .rx          = gamepad_rx_report,
        .ry          = -gamepad_ry_report,
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
        }

        if (!synced_gamepad) {
            if (tud_suspended()) {
                tud_remote_wakeup();
            }
            hid_gamepad_report();
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

void hid_thanks_(alarm_id_t alarm) {
    cancel_alarm(alarm);
    static uint8_t x = 0;
    static bool p = 0;
    static uint8_t keys[16][16] = {
        // You make this possible, thanks.
        {19, 28, 14, 8, 8, 0},
        {18, 6, 11, 12, 8, 9, 0}
    };
    static uint8_t r;
    if (x == 0 && p == false) {
        r = random8() % 2;
    }
    if (keys[r][x] == 0) {
        x = 0;
        p = 0;
        return;
    }
    if (!p) {
        hid_press(keys[r][x]);
        p = true;
    } else if (p) {
        hid_release(keys[r][x]);
        p = false;
        x += 1;
    }
    add_alarm_in_ms(5, (alarm_callback_t)hid_thanks_, NULL, true);
}

void hid_thanks() {
    add_alarm_in_ms(5, (alarm_callback_t)hid_thanks_, NULL, true);
}
