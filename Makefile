# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

default: version
	mkdir -p build
	cmake . -B build -DFW_DEVICE_ALPAKKA=1 && cd build && make -j10

dongle: version
	mkdir -p build
	cmake . -B build -DFW_DEVICE_DONGLE=1 && cd build && make -j10

rebuild: version
	cd build && make -j10

version:
	sh -e scripts/version.sh

install:
	sh -e scripts/install.sh

clean:
	rm -rf build
	rm -f src/headers/version.h

load:
	sh -e scripts/load.sh

reload: rebuild load

session:
	sh -e scripts/session.sh

session_dongle:
	sh -e scripts/session_dongle.sh

session_quit:
	screen -S alpakka -X quit

restart:
	screen -S alpakka -X stuff R

bootsel:
	screen -S alpakka -X stuff B

calibrate:
	screen -S alpakka -X stuff C

format:
	screen -S alpakka -X stuff F

test:
	screen -S alpakka -X stuff T
