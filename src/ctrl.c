// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctrl.h"
#include "thumbstick.h"
#include "config.h"
#include "common.h"
#include "version.h"
#include "logging.h"

Ctrl ctrl_empty() {
    // For some reason, the very first USB message goes to "waste" and ignored
    // by the other end, but it is needed to kickstart the communication.
    // So we will be sending this bogus message first.
    Ctrl ctrl = {
        .protocol_flags = CTRL_FLAG_NONE,
        .device_id = ALPAKKA,
        .message_type = 0,
        .len = 0
    };
    return ctrl;
}

Ctrl ctrl_log(uint8_t* offset_ptr, uint8_t len) {
    Ctrl ctrl = {
        .protocol_flags = CTRL_FLAG_NONE,
        .device_id = ALPAKKA,
        .message_type = LOG,
        .len=len
    };
    for (uint8_t i=0; i<len; i++) {
        ctrl.payload[i] = offset_ptr[i];
    }
    return ctrl;
}

Ctrl ctrl_status_share() {
    Ctrl ctrl = {
        .protocol_flags = CTRL_FLAG_NONE,
        .device_id = ALPAKKA,
        .message_type = STATUS_SHARE,
        .len = 3
    };
    char version[] = VERSION;
    char *version_temp = version;
    char *dash = "-";
    char *dot = ".";
    char *semantic;
    semantic = strsep(&version_temp, dash);  // Split semantic from git tag.
    ctrl.payload[0] = atoi(strsep(&semantic, dot));  // Version major.
    ctrl.payload[1] = atoi(strsep(&semantic, dot));  // Version mid.
    ctrl.payload[2] = atoi(strsep(&semantic, dot));  // Version minor.
    return ctrl;
}

void ctrl_config_set(Ctrl_cfg_type key, uint8_t preset, uint8_t values[5]) {
    if (key == PROTOCOL) config_set_protocol(preset);
    else if (key == SENS_TOUCH) {
        config_set_touch_sens_values(values);
        config_set_touch_sens_preset(preset, false);
    }
    else if (key == SENS_MOUSE) {
        // Scaled by 10 since the USB communication works with integers.
        double values_fmt[] = {
            values[0] / 10.0,
            values[1] / 10.0,
            values[2] / 10.0
            // Indexes 3 and 4 are ignored.
        };
        config_set_mouse_sens_values(values_fmt);
        config_set_mouse_sens_preset(preset, false);
    }
    else if (key == DEADZONE) {
        // Scaled by 100 so the USB communication works with integers.
        float values_fmt[] = {
            values[0] / 100.0,
            values[1] / 100.0,
            values[2] / 100.0
            // Indexes 3 and 4 are ignored.
        };
        config_set_deadzone_values(values_fmt);
        config_set_deadzone_preset(preset, false);
    }
    else if (key == LOG_MASK) {
        logging_set_mask(preset);
    }
    else if (key == LONG_CALIBRATION) {
        config_set_long_calibration(preset);
    }
    else if (key == SWAP_GYROS) {
        config_set_swap_gyros(preset);
    }
    else if (key == TOUCH_INVERT_POLARITY) {
        config_set_touch_invert_polarity(preset);
    }
    else if (key == GYRO_USER_OFFSET) {
        config_set_gyro_user_offset(values[0], values[1], values[2]);
    }
}

Ctrl ctrl_config_share(uint8_t index) {
    Config *config = config_read();
    Ctrl ctrl = {
        .protocol_flags = CTRL_FLAG_NONE,
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
        ctrl.payload[2] = config_get_touch_sens_value(0);
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
    else if (index == LOG_MASK) {
        ctrl.payload[1] = config->log_mask;
    }
    else if (index == LONG_CALIBRATION) {
        ctrl.payload[1] = config->long_calibration;
    }
    else if (index == SWAP_GYROS) {
        ctrl.payload[1] = config->swap_gyros;
    }
    else if (index == TOUCH_INVERT_POLARITY) {
        ctrl.payload[1] = config->touch_invert_polarity;
    }
    else if (index == GYRO_USER_OFFSET) {
        ctrl.payload[2] = config->offset_gyro_user_x;
        ctrl.payload[3] = config->offset_gyro_user_y;
        ctrl.payload[4] = config->offset_gyro_user_z;
    }
    return ctrl;
}

Ctrl ctrl_section_share(uint8_t profile_index, uint8_t section_index) {
    Ctrl ctrl = {
        .protocol_flags = CTRL_FLAG_NONE,
        .device_id = ALPAKKA,
        .message_type = SECTION_SHARE,
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
