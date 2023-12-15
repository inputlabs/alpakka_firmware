// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <bt_config.h>
#include "btstack.h"
#include "logging.h"


void bt_hid_device_setup(void){
    // allow to get found by inquiry
    gap_discoverable_control(1);
    // use Limited Discoverable Mode; Peripheral; Keyboard as CoD
    gap_set_class_of_device(0x2540);
    // set local name to be identified - zeroes will be replaced by actual BD ADDR
    gap_set_local_name("Input Labs Alpakka 00:00:00:00:00:00");
    // allow for role switch in general and sniff mode
    gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
    // allow for role switch on outgoing connections - this allow HID Host to become master when we re-connect to it
    gap_set_allow_role_switch(true);

    debug("BT: init l2cap\n");
    l2cap_init();

    debug("BT: init service discovery\n");
    sdp_init();
    debug("BT: ready\n");
}
