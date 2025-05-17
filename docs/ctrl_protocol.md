# Ctrl protocol

## Introduction

The Ctrl protocol is a minimalistic bidirectional communication protocol between the controller and the web app, using USB (WebUSB). For the purpose of exchanging data such as logging, configuration settings and profile mappings.

The exchange is performed with discrete packages of at most 64 bytes.

Both sides of the communication (USB host and USB device) are equally-levelled actors from the point of view of the protocol, both sides can initiate the communication (send a message) at any time.

There is no acknowledge when receiving messages at protocol level, though some messages could trigger a response message from the other side. In such cases the response message is asynchronous, and can happen at any time later or not happen at all. Is up to the "receiver" to determine adequate timeout values.

Verification and error correction is provided by the USB protocol.

Padding zeros to complete the 64 bytes may be used. Therefore the protocol avoids using zero as indexes for any of its enums.

## Package general structure

| Byte 0 | 1 | 2 | 3 | 4~63 |
| - | - | - | - | - |
| Protocol version | Device Id | Message type | Payload size | Payload

### Protocol flags

| Key | Index |
| - | - |
| NONE | 1 |
| WIRELESS | 2 |

### Device id

| Key | Index |
| - | - |
| ALPAKKA | 1
| KAPYBARA | 2

### Message type

| Key | Index |
| - | - |
LOG | 1
PROC | 2
CONFIG_GET | 3
CONFIG_SET | 4
CONFIG_SHARE | 5
SECTION_GET | 6
SECTION_SET | 7
SECTION_SHARE | 8
STATUS_GET | 9
STATUS_SET | 10
STATUS_SHARE | 11
PROFILE_OVERWRITE | 12

### Procedure index
Procedure index as defined in [hid.h](/src/headers/hid.h).

### Config index
| Key        | Index |
| -          | -     |
| PROTOCOL   | 1
| SENS_TOUCH | 2
| SENS_MOUSE | 3
| DEADZONE   | 4

### Section index
| Key              | Index |
| -                | -     |
| META             | 1
| A                | 2
| B                | 3
| X                | 4
| Y                | 5
| DPAD_LEFT        | 6
| DPAD_RIGHT       | 7
| DPAD_UP          | 8
| DPAD_DOWN        | 9
| SELECT_1         | 10
| START_1          | 11
| SELECT_2         | 12
| START_2          | 13
| L1               | 14
| R1               | 15
| L2               | 16
| R2               | 17
| L4               | 18
| R4               | 19
| ROTARY_UP        | 29
| ROTARY_DOWN      | 30
| LSTICK_SETTINGS  | 31
| LSTICK_LEFT      | 32
| LSTICK_RIGHT     | 33
| LSTICK_UP        | 34
| LSTICK_DOWN      | 35
| LSTICK_UL        | 55
| LSTICK_UR        | 56
| LSTICK_DL        | 57
| LSTICK_DR        | 58
| LSTICK_PUSH      | 36
| LSTICK_INNER     | 37
| LSTICK_OUTER     | 38
| RSTICK_SETTINGS  | 59
| RSTICK_LEFT      | 20
| RSTICK_RIGHT     | 21
| RSTICK_UP        | 22
| RSTICK_DOWN      | 23
| RSTICK_UL        | 24
| RSTICK_UR        | 25
| RSTICK_DL        | 26
| RSTICK_DR        | 27
| RSTICK_PUSH      | 28
| RSTICK_INNER     | 60
| RSTICK_OUTER     | 61
| GLYPHS_0         | 39
| GLYPHS_1         | 40
| GLYPHS_2         | 41
| GLYPHS_3         | 42
| DAISY_0          | 43
| DAISY_1          | 44
| DAISY_2          | 45
| DAISY_3          | 46
| GYRO_SETTINGS    | 47
| GYRO_X           | 48
| GYRO_Y           | 49
| GYRO_Z           | 50
| MACRO_1          | 51
| MACRO_2          | 52
| MACRO_3          | 53
| MACRO_4          | 54

### Section data
Section structs as defined in [ctrl.h](/src/headers/ctrl.h).

## Log message
Message output by the firmware, as strings of arbitrary size.

Direction: `Controller` -> `App`

| Byte 0 | 1 | 2 | 3 | 4~63 |
| - | - | - | - | - |
| Protocol version | Device Id | Message type | Payload size | Payload
|                  |           | LOG          | 1-60         | Log message

## Proc message
Trigger a procedure (eg: calibration) on the controller.

Direction: `Controller` <- `App`

| Byte 0 | 1 | 2 | 3 | 4 |
| - | - | - | - | - |
| Version | Device Id | Message type | Payload size | Payload
|         |           | PROC         | 1            | PROC INDEX

## Status GET message
Request status data from the controller.

Direction: `Controller` <- `App`

| Byte 0 | 1 | 2 | 3 | 4 |
| - | - | - | - | - |
| Version | Device Id | Message type | Payload size | Payload
|         |           | STATUS_GET   | 0            | -

## Status SET message
Send status data to the controller.

Direction: `Controller` <- `App`

| Byte 0 | 1 | 2 | 3 | 4~10
| - | - | - | - | - |
| Version | Device Id | Message type | Payload size | Payload
|         |           | STATUS_SET   | 8            | SYSTEM CLOCK

## Status SHARE message
Send status data to the app.

Direction: `Controller` -> `App`

| Byte 0 | 1 | 2 | 3 | 4~6
| - | - | - | - | - |
| Version | Device Id | Message type | Payload size | Payload
|         |           | STATUS_SHARE | 3            | FW SEMANTIC VERSION

## Config GET message
Request the current value of some specific configuration parameter.

Direction: `Controller` <- `App`

| Byte 0 | 1 | 2 | 3 | 4 |
| - | - | - | - | - |
| Version | Device Id | Message type | Payload size | Payload
|         |           | CONFIG_GET   | 1            | CONFIG INDEX

## Config SET message
Change the value of some specific configuration parameter.

Direction: `Controller` <- `App`

| Byte 0 | 1 | 2 | 3 | 4 | 5 | 6~10
| - | - | - | - | - | - | - |
| Version | Device Id | Message type | Payload size | Payload      | Payload       | Payload
|         |           | CONFIG_SET   | 6            | CONFIG INDEX | PRESET INDEX  | PRESETS VALUE

## Config SHARE message
Notify the current value of some specific configuration parameter.

Direction: `Controller` -> `App`

| Byte 0 | 1 | 2 | 3 | 4 | 5 | 6~10 |
| - | - | - | - | - | - | - |
| Version | Device Id | Message type | Payload size | Payload      | Payload       | Payload
|         |           | CONFIG_SHARE | 6            | CONFIG INDEX | PRESET INDEX  | PRESETS VALUE

## Section GET message
Request the current value of some specific profile section.

Direction: `Controller` <- `App`

| Byte 0  | 1         | 2            | 3            | 4             | 5 |
| -       | -         | -            | -            | -             | - |
| Version | Device Id | Message type | Payload size | Payload       | Payload
|         |           | SECTION_GET  | 2            | PROFILE INDEX | SECTION INDEX

## Section SET message
Change the value of some specific profile section.

Direction: `Controller` <- `App`

| Byte 0  | 1         | 2            | 3            | 4             | 5             | 6~64 |
| -       | -         | -            | -            | -             | -             | -    |
| Version | Device Id | Message type | Payload size | Payload       | Payload       | Payload
|         |           | SECTION_SET  | 58           | PROFILE INDEX | SECTION INDEX | SECTION DATA

## Section SHARE message
Notify the current value of some specific profile section.

Direction: `Controller` -> `App`

| Byte 0  | 1         | 2              | 3            | 4             | 5             | 6~64 |
| -       | -         | -              | -            | -             | -             | -    |
| Version | Device Id | Message type   | Payload size | Payload       | Payload       | Payload
|         |           | SECTION_SHARE  | 58           | PROFILE INDEX | SECTION INDEX | SECTION DATA

## Profile OVERWRITE message
Replaces a controller profile with a profile from another slot or default profile.

Direction: `App` -> `Controller`

| Byte 0  | 1         | 2                  | 3            | 4             | 5
| -       | -         | -                  | -            | -             | -
| Version | Device Id | Message type       | Payload size | Payload       | Payload
|         |           | PROFILE_OVERWRITE  | 2            | PROFILE TO    | PROFILE FROM

**Profile To:** The target profile slot index to be overwritten (1 to 12).

**Profile From:** The profile index to be used as data source. Positive values (1, 12) to use another profile from memory as is. Negative values (-1 to -9) to use profile built-in defaults.

## Example of config interchange
```mermaid
sequenceDiagram
    participant A as App
    participant C as Controller
    Note over A: App must display current mouse sens
    A->>C: App request the current mouse sensitivity value <br>[CONFIG_GET]
    C->>A: Controller share the current value <br>[CONFIG_SHARE]
    Note over A: App display the current mouse sensitivity
    Note over A: ...
    Note over A: User changes mouse sensitivity in-app
    A->>C: App request to change the current value <br>[CONFIG_SET]
    C->>A: Controller share the new value as confirmation <br>[CONFIG_SHARE]
    Note over A: App display the new mouse sensitivity
```
