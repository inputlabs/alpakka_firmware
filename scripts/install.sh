# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

# Pico SDK.
SDK_URL=https://github.com/raspberrypi/pico-sdk.git
SDK_TAG=1.3.1

# ARM toolchain.
# WEBSITE: https://developer.arm.com/downloads/-/gnu-rm
ARM_URL_COMMON=https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10
ARM_FILENAME_DARWIN=gcc-arm-none-eabi-10.3-2021.10-mac.tar.bz2
ARM_FILENAME_LINUX=gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
ARM_TAR=arm-toolchain.tar.bz2
ARM_DIR=arm-toolchain

# Do not remove this.
## GITHUB IMPORTS ENVS UNTIL HERE

rm -rf deps
mkdir deps
cd deps

if [ `uname` = 'Darwin' ]; then
    ARM_URL=$ARM_URL_COMMON/$ARM_FILENAME_DARWIN
else
    ARM_URL=$ARM_URL_COMMON/$ARM_FILENAME_LINUX
fi

echo 'Downloading ARM toolchain...'
echo $ARM_URL
curl --progress-bar -L -o $ARM_TAR $ARM_URL

echo 'Extracting ARM toolchain...'
mkdir $ARM_DIR
tar -xf $ARM_TAR --directory $ARM_DIR --strip-components 1
rm $ARM_TAR

echo "Downloading Pico C SDK..."
git clone $SDK_URL
cd pico-sdk
git checkout --quiet $SDK_TAG

echo "Configuring Pico C SDK..."
git submodule update --init

echo "Dependencies installed"
