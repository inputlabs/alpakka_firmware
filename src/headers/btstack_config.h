#pragma once

#define ENABLE_PRINTF_HEXDUMP
#define ENABLE_SOFTWARE_AES128
// #define ENABLE_L2CAP_ENHANCED_RETRANSMISSION_MODE
// #define ENABLE_LOG_INFO
// #define ENABLE_LOG_ERROR
// #define ENABLE_SCO_OVER_HCI

#define HCI_ACL_PAYLOAD_SIZE 52
#define HCI_OUTGOING_PRE_BUFFER_SIZE 4
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT 4

#define MAX_NR_BTSTACK_LINK_KEY_DB_MEMORY_ENTRIES  2
#define MAX_NR_GATT_CLIENTS 1
#define MAX_NR_HCI_CONNECTIONS 2
#define MAX_NR_HID_HOST_CONNECTIONS 1
#define MAX_NR_HIDS_CLIENTS 1

#define MAX_NR_L2CAP_CHANNELS 8
#define MAX_NR_L2CAP_SERVICES 3

#define MAX_NR_RFCOMM_CHANNELS 1
#define MAX_NR_RFCOMM_MULTIPLEXERS 1
#define MAX_NR_RFCOMM_SERVICES 1

#define MAX_NR_SERVICE_RECORD_ITEMS 4
#define MAX_NR_SM_LOOKUP_ENTRIES 3
#define MAX_NR_WHITELIST_ENTRIES 16
#define MAX_NR_LE_DEVICE_DB_ENTRIES 16

// Limit number of ACL/SCO Buffer to use by stack to avoid cyw43 shared bus overrun.
#define MAX_NR_CONTROLLER_ACL_BUFFERS 3
#define MAX_NR_CONTROLLER_SCO_PACKETS 3

// Enable and configure HCI Controller to Host Flow Control to avoid cyw43 shared bus overrun.
#define ENABLE_HCI_CONTROLLER_TO_HOST_FLOW_CONTROL

#define HCI_HOST_ACL_PACKET_LEN 52
#define HCI_HOST_ACL_PACKET_NUM 4
#define HCI_HOST_SCO_PACKET_LEN 52
#define HCI_HOST_SCO_PACKET_NUM 4

// Link Key DB and LE Device DB using TLV on top of Flash Sector interface
#define NVM_NUM_DEVICE_DB_ENTRIES 0
#define NVM_NUM_LINK_KEYS 0

// We don't give btstack a malloc, so use a fixed-size ATT DB.
#define MAX_ATT_DB_SIZE 512

// BTstack HAL configuration.
#define HAVE_EMBEDDED_TIME_MS

// Map btstack_assert onto Pico SDK assert().
#define HAVE_ASSERT

// Some USB dongles take longer to respond to HCI reset (e.g. BCM20702A).
#define HCI_RESET_RESEND_TIMEOUT_MS 1000