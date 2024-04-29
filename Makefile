# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

default: version
	mkdir -p build
	cmake . -B build && cd build && make

rebuild: version
	cd build && make

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
	python3 scripts/session.py

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
