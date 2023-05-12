# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

default:
	mkdir -p build
	cmake . -B build && cd build && make

rebuild:
	cd build && make

clean:
	rm -rf build

load:
	sh -e scripts/load.sh

reload: rebuild load

session:
	sh -e scripts/session.sh

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

install:
	sh -e scripts/install.sh
