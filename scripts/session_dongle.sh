# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

DEVICE=`ls /dev/ | grep -e "tty\.usbserial-140" -e "ttyUSB" | head -n 1`
# DEVICE=ttyUSB1  # Uncomment this line to manually define a device.

if [ ! $DEVICE ]; then
    echo 'No USB serial found'
    exit 1
fi

CAPTION=" Active session @ /dev/${DEVICE}    [Press 'Q' to quit]"
sleep 0.5 && screen -S dongle -X bindkey q quit &
sleep 1 && screen -S dongle -X caption always "$CAPTION" &
screen -S dongle /dev/$DEVICE 115200
