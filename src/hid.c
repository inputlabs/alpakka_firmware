// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <tusb.h>
#include <device/usbd_pvt.h>
#include <pico/util/queue.h>
#include "config.h"
#include "ctrl.h"
#include "hid.h"
#include "wireless.h"
#include "led.h"
#include "profile.h"
#include "xinput.h"
#include "common.h"
#include "webusb.h"
#include "logging.h"
#include "thanks.c"

bool hid_allow_communication = true;  // Extern.
bool synced_keyboard = false;
bool synced_mouse = false;
bool synced_mouse_eot = false;  // End Of Transmission.
bool synced_gamepad = false;
uint16_t alarms = 0;
alarm_pool_t *alarm_pool;
static queue_t report_queue;

uint8_t state_matrix[256] = {0,};
int16_t mouse_x = 0;
int16_t mouse_y = 0;
double gamepad_lx = 0;
double gamepad_ly = 0;
double gamepad_rx = 0;
double gamepad_ry = 0;
double gamepad_lz = 0;
double gamepad_rz = 0;

void hid_matrix_reset() {
    for(uint8_t i=0; i<255; i++) {
        state_matrix[i] = 0;
    }
    synced_keyboard = false;
    synced_mouse = false;
    synced_mouse_eot = false;
    synced_gamepad = false;
}

void hid_procedure_press(uint8_t procedure){
    if (procedure == PROC_HOME) profile_set_home(true);                  // Hold home.
    if (procedure == PROC_HOME_GAMEPAD) profile_set_home_gamepad(true);  // Double-click-hold home.
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
    if (procedure == PROC_TUNE_MOUSE_SENS) config_tune_set_mode(procedure);
    if (procedure == PROC_TUNE_TOUCH_SENS) config_tune_set_mode(procedure);
    if (procedure == PROC_TUNE_DEADZONE) config_tune_set_mode(procedure);
    if (procedure == PROC_CALIBRATE) config_calibrate();
    if (procedure == PROC_RESTART) config_reboot();
    if (procedure == PROC_BOOTSEL) config_bootsel();
    if (procedure == PROC_THANKS) hid_thanks();
    if (procedure == PROC_IGNORE_LED_WARNINGS) config_ignore_problems();
    // Scrollwheel alternative modes. (Used for example in Racing profile).
    if (procedure == PROC_ROTARY_MODE_0) rotary_set_mode(0);
    if (procedure == PROC_ROTARY_MODE_1) rotary_set_mode(1);
    if (procedure == PROC_ROTARY_MODE_2) rotary_set_mode(2);
    if (procedure == PROC_ROTARY_MODE_3) rotary_set_mode(3);
    if (procedure == PROC_ROTARY_MODE_4) rotary_set_mode(4);
    if (procedure == PROC_ROTARY_MODE_5) rotary_set_mode(5);
    // Macros.
    if (procedure == PROC_MACRO_1) hid_macro(1);
    if (procedure == PROC_MACRO_2) hid_macro(2);
    if (procedure == PROC_MACRO_3) hid_macro(3);
    if (procedure == PROC_MACRO_4) hid_macro(4);
    if (procedure == PROC_MACRO_5) hid_macro(5);
    if (procedure == PROC_MACRO_6) hid_macro(6);
    if (procedure == PROC_MACRO_7) hid_macro(7);
    if (procedure == PROC_MACRO_8) hid_macro(8);
    // Experimental.
    if (procedure == PROC_ADZ) gyro_wheel_antideadzone(1);
    if (procedure == PROC_ADZN) gyro_wheel_antideadzone(-1);
}

void hid_procedure_release(uint8_t procedure) {
    if (procedure == PROC_HOME) profile_set_home(false);
    if (procedure == PROC_HOME_GAMEPAD) profile_set_home_gamepad(false);
}

void hid_press(uint8_t key) {
    if (key == KEY_NONE) return;
    else if (key >= PROC_INDEX) hid_procedure_press(key);
    else {
        state_matrix[key] += 1;
        if (key >= GAMEPAD_INDEX) synced_gamepad = false;
        else if (key >= MOUSE_INDEX) synced_mouse = synced_mouse_eot = false;
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
        else if (key >= MOUSE_INDEX) synced_mouse = synced_mouse_eot = false;
        else synced_keyboard = false;
    }
}

void hid_press_multiple(uint8_t *keys) {
    for(uint8_t i=0; i<ACTIONS_LEN; i++) {
        if (keys[i] == 0) return;
        hid_press(keys[i]);
    }
}

void hid_release_multiple(uint8_t *keys) {
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

void hid_macro(uint8_t index) {
    uint8_t section = SECTION_MACRO_1 + ((index - 1) / 2);
    uint8_t subindex = (index - 1) % 2;
    CtrlProfile *profile = config_profile_read(profile_get_active_index(false));
    uint8_t *macro = profile->sections[section].macro.macro[subindex];
    if (alarms > 0) return;  // Disallows parallel macros. TODO fix.
    uint16_t time = 10;
    for(uint8_t i=0; i<28; i++) {
        if (macro[i] == 0) break;
        hid_press_later(macro[i], time);
        time += 10;
        hid_release_later(macro[i], time);
        time += 10;
    }
}

bool hid_is_axis(uint8_t key) {
    return is_between(key, GAMEPAD_AXIS_INDEX, PROC_INDEX-1);
}

void hid_mouse_move(int16_t x, int16_t y) {
    mouse_x += x;
    mouse_y += y;
    synced_mouse = false;
    synced_mouse_eot = false;
}

void hid_gamepad_lx(double value) {
    if (value == gamepad_lx) return;
    gamepad_lx += value;  // Multiple inputs can be combined.
    synced_gamepad = false;
}

void hid_gamepad_ly(double value) {
    if (value == gamepad_ly) return;
    gamepad_ly += value;  // Multiple inputs can be combined.
    synced_gamepad = false;
}

void hid_gamepad_lz(double value) {
    if (value == gamepad_lz) return;
    gamepad_lz += value;  // Multiple inputs can be combined.
    synced_gamepad = false;
}

void hid_gamepad_rx(double value) {
    if (value == gamepad_rx) return;
    gamepad_rx += value;  // Multiple inputs can be combined.
    synced_gamepad = false;
}

void hid_gamepad_ry(double value) {
    if (value == gamepad_ry) return;
    gamepad_ry += value;  // Multiple inputs can be combined.
    synced_gamepad = false;
}

void hid_gamepad_rz(double value) {
    if (value == gamepad_rz) return;
    gamepad_rz += value;  // Multiple inputs can be combined.
    synced_gamepad = false;
}

void hid_report_to_queue(uint8_t report_type, void *report, uint8_t len) {
    if (!wireless_device_is_connected()) return;
    uint8_t entry[32] = {report_type};
    memcpy(&entry[1], report, len);
    bool added = queue_try_add(hid_get_queue(), entry);
    // if (!added) printf("WL: Cannot add into queue\n");
    if (!added) printf("Q");
}

void hid_mouse_report(bool wired) {
    // Create button bitmask.
    int8_t buttons = 0;
    for(int i=0; i<5; i++) {
        buttons += state_matrix[MOUSE_INDEX + i] << i;
    }
    uint8_t scroll = state_matrix[MOUSE_SCROLL_UP] - state_matrix[MOUSE_SCROLL_DOWN];
    // Create report.
    MouseReport report = {buttons, mouse_x, mouse_y, scroll, 0};
    // Reset values.
    mouse_x = 0;
    mouse_y = 0;
    state_matrix[MOUSE_SCROLL_UP] = 0;
    state_matrix[MOUSE_SCROLL_DOWN] = 0;
    // Send report.
    if (wired) tud_hid_report(REPORT_MOUSE, &report, sizeof(report));
    else hid_report_to_queue(REPORT_MOUSE, &report, sizeof(report));
}

void hid_keyboard_report(bool wired) {
    uint8_t keys[6] = {0};
    uint8_t keys_available = 6;
    for(int i=0; i<=115; i++) {
        if (state_matrix[i] >= 1) {
            keys[keys_available - 1] = (uint8_t)i;
            keys_available--;
            if (keys_available == 0) {
                break;
            }
        }
    }
    uint8_t modifiers = 0;
    for(int i=0; i<8; i++) {
        modifiers += !!state_matrix[MODIFIER_INDEX + i] << i;
    }
    KeyboardReport report = {modifiers};
    memcpy(report.keycode, keys, 6);
    if (wired) tud_hid_report(REPORT_KEYBOARD, &report, sizeof(report));
    else hid_report_to_queue(REPORT_KEYBOARD, &report, sizeof(report));
}

double hid_axis(
    double value,
    uint8_t matrix_index_pos,
    uint8_t matrix_index_neg
) {
    if (matrix_index_neg) {
        if (state_matrix[matrix_index_neg]) return -1;
        else if (state_matrix[matrix_index_pos]) return 1;
        else return constrain(value, -1, 1);
    } else {
        if (state_matrix[matrix_index_pos]) return 1;
        else return constrain(fabs(value), 0, 1);
    }
}

void hid_gamepad_report() {
    // Sorted so the most common assigned buttons are lower and easier to
    // identify in-game.
    int32_t buttons = (
        (state_matrix[GAMEPAD_A]      <<  0) +
        (state_matrix[GAMEPAD_B]      <<  1) +
        (state_matrix[GAMEPAD_X]      <<  2) +
        (state_matrix[GAMEPAD_Y]      <<  3) +
        (state_matrix[GAMEPAD_L1]     <<  4) +
        (state_matrix[GAMEPAD_R1]     <<  5) +
        (state_matrix[GAMEPAD_L3]     <<  6) +
        (state_matrix[GAMEPAD_R3]     <<  7) +
        (state_matrix[GAMEPAD_LEFT]   <<  8) +
        (state_matrix[GAMEPAD_RIGHT]  <<  9) +
        (state_matrix[GAMEPAD_UP]     << 10) +
        (state_matrix[GAMEPAD_DOWN]   << 11) +
        (state_matrix[GAMEPAD_SELECT] << 12) +
        (state_matrix[GAMEPAD_START]  << 13) +
        (state_matrix[GAMEPAD_HOME]   << 14)
    );
    // Adjust range from [-1,1] to [-32767,32767].
    int16_t lx_report = hid_axis(gamepad_lx, GAMEPAD_AXIS_LX, GAMEPAD_AXIS_LX_NEG) * BIT_15;
    int16_t ly_report = hid_axis(gamepad_ly, GAMEPAD_AXIS_LY, GAMEPAD_AXIS_LY_NEG) * BIT_15;
    int16_t rx_report = hid_axis(gamepad_rx, GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RX_NEG) * BIT_15;
    int16_t ry_report = hid_axis(gamepad_ry, GAMEPAD_AXIS_RY, GAMEPAD_AXIS_RY_NEG) * BIT_15;
    // HID triggers must be also defined as unsigned in the USB descriptor, and has to be manually
    // value-shifted from signed to unsigned here, otherwise Windows is having erratic behavior and
    // inconsistencies between games (not sure if a bug in Windows' DirectInput or TinyUSB).
    int16_t lz_report = ((hid_axis(gamepad_lz, GAMEPAD_AXIS_LZ, 0) * 2) - 1) * BIT_15;
    int16_t rz_report = ((hid_axis(gamepad_rz, GAMEPAD_AXIS_RZ, 0) * 2) - 1) * BIT_15;
    GamepadReport report = {
        lx_report,
        ly_report,
        rx_report,
        ry_report,
        lz_report,
        rz_report,
        buttons,
    };
    tud_hid_report(REPORT_GAMEPAD, &report, sizeof(report));
}

void hid_xinput_report(bool wired) {
    int8_t buttons_0 = 0;
    int8_t buttons_1 = 0;
    for(int i=0; i<8; i++) {
        buttons_0 += state_matrix[GAMEPAD_INDEX + i] << i;
    }
    for(int i=0; i<8; i++) {
        buttons_1 += state_matrix[GAMEPAD_INDEX + i + 8] << i;
    }
    // Adjust range from [-1,1] to [-32767,32767].
    int16_t lx_report = hid_axis(gamepad_lx, GAMEPAD_AXIS_LX, GAMEPAD_AXIS_LX_NEG) * BIT_15;
    int16_t ly_report = hid_axis(gamepad_ly, GAMEPAD_AXIS_LY, GAMEPAD_AXIS_LY_NEG) * BIT_15;
    int16_t rx_report = hid_axis(gamepad_rx, GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RX_NEG) * BIT_15;
    int16_t ry_report = hid_axis(gamepad_ry, GAMEPAD_AXIS_RY, GAMEPAD_AXIS_RY_NEG) * BIT_15;
    // Adjust range from [0,1] to [0,255].
    uint16_t lz_report = hid_axis(gamepad_lz, GAMEPAD_AXIS_LZ, 0) * BIT_8;
    uint16_t rz_report = hid_axis(gamepad_rz, GAMEPAD_AXIS_RZ, 0) * BIT_8;
    XInputReport report = {
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
    if (wired) xinput_send_report(&report);
    else hid_report_to_queue(REPORT_XINPUT, &report, sizeof(report));
}

void hid_gamepad_reset() {
    gamepad_lx = 0;
    gamepad_ly = 0;
    gamepad_rx = 0;
    gamepad_ry = 0;
    gamepad_lz = 0;
    gamepad_rz = 0;
}

void hid_report_wireless() {
    if (synced_mouse && !synced_mouse_eot) {
        hid_report_to_queue(REPORT_MOUSE_EOT, NULL, 0);
        synced_mouse_eot = true;
    }
    if (!synced_keyboard) {
        hid_keyboard_report(false);
        synced_keyboard = true;
    }
    if (!synced_mouse) {
        hid_mouse_report(false);
        synced_mouse = true;
    }
    if (!synced_gamepad) {
        hid_xinput_report(false);  // TODO: Generic gamepad support.
        hid_gamepad_reset();
        synced_gamepad = true;
    }
}

bool hid_report() {
    static uint8_t priority_mouse = 0;
    static uint8_t priority_gamepad = 0;

    // Not all events are sent everytime, they are delivered based on their
    // priority ratio and how long they have been queueing.
    // For example thumbstick movement may be queued for some cycles if there
    // is a lot of mouse data being sent.
    if (!synced_mouse) priority_mouse += 1 * CFG_HID_REPORT_PRIORITY_RATIO;
    if (!synced_gamepad) priority_gamepad += 1;
    if (!hid_allow_communication) return true;
    tud_task();
    if (tud_ready()) {
        if (tud_hid_ready()) {
            webusb_read();
            webusb_flush();
            if (!synced_keyboard) {
                hid_keyboard_report(true);
                synced_keyboard = true;
            }
            else if (!synced_mouse && (priority_mouse > priority_gamepad)) {
                hid_mouse_report(true);
                synced_mouse = true;
                priority_mouse = 0;
            }
            else if (!synced_gamepad && config_get_protocol() == PROTOCOL_GENERIC) {
                hid_gamepad_report();
                synced_gamepad = true;
                priority_gamepad = 0;
            }
        }
        if (!synced_gamepad && config_get_protocol() != PROTOCOL_GENERIC) {
            if (tud_suspended()) {
                tud_remote_wakeup();
            }
            hid_xinput_report(true);
            priority_gamepad = 0;
        }
        // Gamepad values being reset so potentially unsent values are not
        // aggregated with the next cycle.
        hid_gamepad_reset();
        return true;
    } else {
        return false;
    }
}

void hid_report_from_queue() {
    uint8_t kb_reports = 0;  // Keyboard.
    uint8_t m_reports = 0;  // Mouse.
    uint8_t x_reports = 0;  // XInput.
    uint8_t mouse_combined = 0;
    static bool mouse_engaged = false;
    static uint8_t predicted = 0;
    static MouseReport mouse_last_report;
    static int16_t mouse_predicted_x = 0;
    static int16_t mouse_predicted_y = 0;
    KeyboardReport kb_report;
    MouseReport m_report;
    XInputReport x_report;
    while(!queue_is_empty(hid_get_queue())) {
        uint8_t entry[32];
        queue_remove_blocking(hid_get_queue(), entry);
        uint8_t report_type = entry[0];
        if (report_type == REPORT_KEYBOARD) {
            kb_reports += 1;
            KeyboardReport report = *(KeyboardReport*)&entry[1];
            memcpy(&kb_report, &report, sizeof(KeyboardReport));
        }
        if (report_type == REPORT_MOUSE) {
            m_reports += 1;
            mouse_engaged = true;
            mouse_combined += entry[1];
            MouseReport report = *(MouseReport*)&entry[2];
            if (m_reports == 1) {
                memcpy(&mouse_last_report, &report, sizeof(MouseReport));
            }
            else {
                report.x += m_report.x;
                report.y += m_report.y;
            }
            memcpy(&m_report, &report, sizeof(MouseReport));
        }
        if (report_type == REPORT_XINPUT) {
            x_reports += 1;
            XInputReport report = *(XInputReport*)&entry[1];
            memcpy(&x_report, &report, sizeof(XInputReport));
        }
        if (report_type == REPORT_MOUSE_EOT) {
            // printf("E");
            mouse_engaged = false;
        }
    }
    tud_task();
    if (!tud_ready()) {
        printf("T\n");
        return;
    }
    webusb_read();
    webusb_flush();
    bool kb_sent;
    bool m_sent;
    bool x_sent;
    if (kb_reports > 0) {
        kb_sent = tud_hid_report(REPORT_KEYBOARD, &kb_report, sizeof(kb_report));
        if (!kb_sent) printf("K");
    }
    if (m_reports > 0) {
        // printf("%i ", mouse_combined);
        if (kb_sent) {
            sleep_us(1000);
            tud_task();
        }
        // Reconcile predictions.
        if (predicted > 0 && mouse_combined > 1) {
            // printf("R");
            m_report.x -= mouse_predicted_x;
            m_report.y -= mouse_predicted_y;
            mouse_predicted_x = 0;
            mouse_predicted_y = 0;
            predicted -= (mouse_combined - 1);
            if (predicted <= 0) {
                predicted = 0;
            }
        }
        m_sent = tud_hid_report(REPORT_MOUSE, &m_report, sizeof(m_report));
        if (!m_sent) printf("M");
    }
    if (x_reports > 0) {
        if (kb_sent || m_sent) {
            sleep_us(1000);
            tud_task();
        }
        x_sent = xinput_send_report(&x_report);
        if (!x_sent) printf("X");
    }
    // Do prediction.
    if (m_reports == 0 && mouse_engaged) {
        if (kb_sent || x_sent) {
            sleep_us(1000);
            tud_task();
        }
        m_sent = tud_hid_report(REPORT_MOUSE, &mouse_last_report, sizeof(mouse_last_report));
        if (!m_sent) printf("M");
        predicted += 1;
        mouse_predicted_x += mouse_last_report.x;
        mouse_predicted_y += mouse_last_report.y;
        // printf("P ");
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

queue_t* hid_get_queue() {
    return &report_queue;
}

void hid_init() {
    info("INIT: HID\n");
    alarm_pool = alarm_pool_create(2, 255);
    queue_init(&report_queue, REPORT_QUEUE_ITEM_SIZE, REPORT_QUEUE_LEN);
}
