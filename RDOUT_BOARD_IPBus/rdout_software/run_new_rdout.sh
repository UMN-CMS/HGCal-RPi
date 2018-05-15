#!/bin/bash -v

gcc -O src/new_rdout.c src/ejf_rdout.c src/ctl_orm.c src/data_orm.c src/hexbd.c src/hexbd_config.c src/spi_common.c -l bcm2835 -o bin/new_rdout.exe

sudo ./bin/new_rdout.exe 100 100 0 > rdout.dat

