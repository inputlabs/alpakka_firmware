# Changelog

## 0.88.2
- Dynamic touch threshold improvements:
  - Do not increase the threshold on sudden inconsistent peaks.
  - Prevent the threshold from going higher and higher on long gaming sessions.

## 0.88.1
- Hotfix for dynamic touch threshold.
- Dedicated alarm pool for HID (macros).

## 0.88.0
- Added dynamic touch threshold logic. While the static threshold presets are kept as legacy options. See [Tune](https://inputlabs.io/alpakka/manual/tune) for details.
- Profiles can define the gyro engage trigger, can be either the touch surface (default) or any button.
- Profiles can define a thumbstick deadzone that overrides global tune deadzone.
- Profiles can define the thumbstick overlap for the 4 directions, as a value from `-1` to `1`:
  - `0` means no overlap (each direction takes 90°).
  - Values approaching `1` create bigger overlaps (each direction takes up to 180°).
  - Values approaching `-1` create thinner activation areas with gaps in-between (each direction takes down to 0°).
- Profiles can define any axis on independent sides of the thumbstick, and negative axis have their own identifier.
- Scrollwheel scroll (up or down) can be mapped to any button.
- Home button mapped to `gamepad-home` instead of `shift-tab`.
- In console profile, previously unused 2nd row select buttons are mapped to `M` and `N`. For consistency with FPS profiles.
- Fixed LED not blinking when requested profile was the same than the active profile.
- Basic implementation of macros (fixed timing, no modifier keys).
- Added predefined phrases on home thumbstick (using macros):
  - `yes`
  - `no`
  - `thanks`
  - `gg`

_This update requires recalibration._

## 0.87.0
- Added new OS mode: HID generic gamepad (aka DirectInput in Windows). See [Tune](https://inputlabs.io/alpakka/manual/tune) for details.
- Automatic controller restart when changing OS mode.

## 0.86.5
- Show Pico unique hardware id into session log.
- Fixed calibration procedure never completing.
- Added user adjustable presets for touch threshold (5 presets).

_This update requires recalibration._

## 0.86.1
- Added Patreon easter egg.

## 0.86.0
- Created `Console Legacy` profile.
- Any gamepad axis (thumbstick and analog triggers) can be assigned to any button.
- Gyro as a configurable input in the profiles.
  - Gyro to mouse mapping can be customized and inverted.
  - Gyro axis Z is now also available to profiles.
  - Foundations to assign any action to gyro axis (still not functional).
- Gyro modes:
  - `GYRO_MODE_ALWAYS_OFF`
  - `GYRO_MODE_ALWAYS_ON`
  - `GYRO_MODE_TOUCH_OFF`
  - `GYRO_MODE_TOUCH_ON`
- Calibration and non-volatile memory now include gyro axis Z offset.

_This update requires recalibration._

## 0.84.6
- First published version.
