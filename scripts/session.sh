# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

clear

BAUD=115200

ADAPTERS=`ls -1 /dev/ | grep -e "usb" -e "USB"`
if [ ! $ADAPTER ]; then
    echo "\nUsage:"
    echo "$ ADAPTER=<adapter> make session"
    echo "$ ADAPTER=<adapter> SESSION=session make session"
    echo "\nFound adapters:"
    echo "$ADAPTERS"
    exit 0
fi

if [ ! $SESSION ]; then
    SESSION=alpakka
fi

CAPTION=" $SESSION @ /dev/$ADAPTER ($BAUD)   [Press 'Q' to quit]"
sleep 0.5 && screen -S $SESSION -X bindkey q quit &
sleep 1 && screen -S $SESSION -X caption always "$CAPTION" &
screen -S $SESSION /dev/$ADAPTER $BAUD
