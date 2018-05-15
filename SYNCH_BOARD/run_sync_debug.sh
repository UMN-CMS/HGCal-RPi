#!/bin/bash -v

gcc -O src/sync_debug.c src/sync_orm.c src/spi_common.c -l bcm2835 -o bin/sync_debug.exe

 sudo ./bin/sync_debug.exe 0
# 1 for pedestal, 0 for regular triggers

