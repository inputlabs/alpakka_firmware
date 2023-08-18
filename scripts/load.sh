# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

UF2=build/alpakka.uf2
DRIVE_LINUX="/media/RPI-RP2"
DRIVE_MACOS="/Volumes/RPI-RP2"
DRIVE_WSL="/mnt/RPI-RP2"
WSL=0

RED=`tput setaf 1`
GREEN=`tput setaf 2`
YELLOW=`tput setaf 3`
CYAN=`tput setaf 6`
RESET=`tput sgr0`

bootsel() {
    if `screen -list | grep -q alpakka`; then
        echo "Requesting controller to go into Bootsel"
        screen -S alpakka -X stuff B
    fi
}

wsl_mount() {
    if [ $WSL -eq 1 ]; then
        sudo mkdir $DRIVE_WSL 2>/dev/null || true  # This mess is drvfs fault.
        WMIC="/mnt/c/Windows/System32/wbem/WMIC.exe"
        QUERY="logicalDisk get DeviceId,VolumeName"
        if `${WMIC} ${QUERY} | grep -q RPI-RP2`; then
            DRIVE_LETTER=`${WMIC} ${QUERY} | grep RPI-RP2 | cut -c 1-2`
            echo "Mounting Windows drive (${DRIVE_LETTER}) into ${DRIVE_WSL}"
            sudo mount -t drvfs $DRIVE_LETTER $DRIVE_WSL
        fi
    fi
}

if `uname -s | grep -q Darwin`; then DRIVE=$DRIVE_MACOS; fi
if `uname -s | grep -q Linux`; then
    if ! `uname -r | grep -q microsoft`; then
        DRIVE=$DRIVE_LINUX;
    else
        DRIVE=$DRIVE_WSL
        WSL=1
    fi
fi

echo "Expecting drive at: ${DRIVE}"
bootsel
printf $YELLOW"Waiting for Pico in Bootsel mode (RPI-RP2 drive)   "
i=0;
progress=("-" "\\" "|" "/")
while true; do
    wsl_mount
    if [ -d $DRIVE ]; then
        if [ -f $DRIVE/INFO_UF2.TXT ]; then
            cp $UF2 $DRIVE
            break
        fi
    fi
    printf "$YELLOW\b\b"
    printf ${progress[i % 4]}
    printf "$RESET "
    sleep 0.2
    ((i=i+1))
done
echo $GREEN"\nUF2 loaded into Pico" $RESET

