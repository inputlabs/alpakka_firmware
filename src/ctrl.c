// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "ctrl.h"
#include "thumbstick.h"
#include "config.h"
#include "common.h"

Ctrl ctrl_establish_device() {
    Ctrl ctrl = {
        .protocol_version = CTRL_PROTOCOL_VERSION,
        .device_id = ALPAKKA,
        .message_type = ESTABLISH_DEVICE,
        .len=0
    };
    return ctrl;
}

Ctrl ctrl_log(uint8_t* offset_ptr, uint8_t len) {
    Ctrl ctrl = {
        .protocol_version = CTRL_PROTOCOL_VERSION,
        .device_id = ALPAKKA,
        .message_type = LOG,
        .len=len
    };
    for (uint8_t i=0; i<len; i++) {
        ctrl.payload[i] = offset_ptr[i];
    }
    return ctrl;
}

Ctrl ctrl_config_share(uint8_t index) {
    Ctrl ctrl = {
        .protocol_version = CTRL_PROTOCOL_VERSION,
        .device_id = ALPAKKA,
        .message_type = CONFIG_SHARE,
        .len = 7
    };
    ctrl.payload[0] = index;
    if (index == PROTOCOL) {
        ctrl.payload[1] = config_get_protocol();
        // All values are sent as zero / ignored.
    }
    else if (index == SENS_TOUCH) {
        ctrl.payload[1] = config_get_touch_sens_preset();
        ctrl.payload[2] = 0;  // Auto.
        ctrl.payload[3] = config_get_touch_sens_value(1);
        ctrl.payload[4] = config_get_touch_sens_value(2);
        ctrl.payload[5] = config_get_touch_sens_value(3);
        ctrl.payload[6] = config_get_touch_sens_value(4);
    }
    else if (index == SENS_MOUSE) {
        ctrl.payload[1] = config_get_mouse_sens_preset();
        ctrl.payload[2] = config_get_mouse_sens_value(0) * 10;
        ctrl.payload[3] = config_get_mouse_sens_value(1) * 10;
        ctrl.payload[4] = config_get_mouse_sens_value(2) * 10;
    }
    else if (index == DEADZONE) {
        ctrl.payload[1] = config_get_deadzone_preset();
        ctrl.payload[2] = config_get_deadzone_value(0) * 100;
        ctrl.payload[3] = config_get_deadzone_value(1) * 100;
        ctrl.payload[4] = config_get_deadzone_value(2) * 100;
    }
    return ctrl;
}

Ctrl ctrl_profile_share(uint8_t profile_index, uint8_t section_index) {
    Ctrl ctrl = {
        .protocol_version = CTRL_PROTOCOL_VERSION,
        .device_id = ALPAKKA,
        .message_type = PROFILE_SHARE,
        .len = 60
    };
    // Profile section struct cast into packed int array.
    // Note that section structs must be guaranteed to be packed.
    CtrlProfile *profile = config_profile_read(profile_index);
    uint8_t *section = (uint8_t*)&(profile->sections[section_index]);
    // Write payload.
    ctrl.payload[0] = profile_index;
    ctrl.payload[1] = section_index;
    for(uint8_t i=2; i<60; i++) {
        ctrl.payload[i] = section[i-2];
    }
    return ctrl;
}
