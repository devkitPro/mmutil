#!/bin/sh
cd player/gba
make clean && make
cd ../..
python3 tools/bin2c.py source/player_data_gba.c source/player_data_gba.h player/gba/mmgbarom_mb.gba
