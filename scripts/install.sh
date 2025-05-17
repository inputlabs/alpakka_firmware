# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

# Pico SDK.
SDK_URL=https://github.com/raspberrypi/pico-sdk.git
SDK_TAG=2.1.0

# Pico Extras.
EXTRAS_URL=https://github.com/raspberrypi/pico-extras.git
EXTRAS_TAG=sdk-2.1.0

# Pico tool.
PICOTOOL_URL=https://github.com/raspberrypi/picotool.git
PICOTOOL_TAG=v2.1.0

# ESP serial flasher
ESPSF_URL=https://github.com/espressif/esp-serial-flasher
ESPSF_TAG=v1.6.2

# ARM toolchain.
# WEBSITE: https://developer.arm.com/downloads/-/gnu-rm
ARM_URL_COMMON=https://developer.arm.com/-/media/Files/downloads/gnu/12.3.rel1/binrel
ARM_FILENAME_DARWIN=arm-gnu-toolchain-12.3.rel1-darwin-arm64-arm-none-eabi.tar.xz
ARM_FILENAME_LINUX_X86_64=arm-gnu-toolchain-12.3.rel1-x86_64-arm-none-eabi.tar.xz
ARM_FILENAME_LINUX_ARM64=arm-gnu-toolchain-12.3.rel1-aarch64-arm-none-eabi.tar.xz
ARM_TAR=arm-toolchain.tar.bz2
ARM_DIR=arm-toolchain

# Do not remove this.
## GITHUB IMPORTS ENVS UNTIL HERE

rm -rf deps
mkdir deps
cd deps

PLATFORM="$( uname -sm )"

if [ "$PLATFORM" = "Darwin arm64" ]; then
    ARM_URL=$ARM_URL_COMMON/$ARM_FILENAME_DARWIN
elif [ "$PLATFORM" = "Linux x86_64" ]; then
    ARM_URL=$ARM_URL_COMMON/$ARM_FILENAME_LINUX_X86_64
elif [ "$PLATFORM" = "Linux aarch64" ]; then
    ARM_URL=$ARM_URL_COMMON/$ARM_FILENAME_LINUX_ARM64
else
    echo "Unsupported platform: ${PLATFORM}"
    exit 1
fi

# ARM toolchain.
echo 'Downloading ARM toolchain...'
echo $ARM_URL
curl --progress-bar -L -o $ARM_TAR $ARM_URL
echo 'Extracting ARM toolchain...'
mkdir $ARM_DIR
tar -xf $ARM_TAR --directory $ARM_DIR --strip-components 1
rm $ARM_TAR

# Pico SDK.
echo "Downloading Pico C SDK..."
git clone $SDK_URL
cd pico-sdk
git checkout --quiet $SDK_TAG
echo "Configuring Pico C SDK..."
git submodule update --init
cd ..

# Pico Extras.
echo "Downloading Pico Extras..."
git clone $EXTRAS_URL
cd pico-extras
git checkout --quiet $EXTRAS_TAG
cd ..

# ESP serial flasher.
echo "Downloading ESP Serial Flasher..."
git clone $ESPSF_URL
cd esp-serial-flasher
git checkout --quiet $ESPSF_TAG
echo "Configuring ESP Serial Flasher..."
git submodule update --init
python3 ../../scripts/esp_flasher_patch.py
cd ..

# Picotool (pico-sdk depends on it now).
echo "Downloading Picotool..."
git clone $PICOTOOL_URL
cd picotool
git checkout --quiet $PICOTOOL_TAG_TAG
echo "Building Picotool..."
export PICO_SDK_PATH=`pwd`/../pico-sdk
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=./bin -DPICOTOOL_FLAT_INSTALL=1 ..
make install
cd ../..

# Done.
echo "Dependencies installed"
