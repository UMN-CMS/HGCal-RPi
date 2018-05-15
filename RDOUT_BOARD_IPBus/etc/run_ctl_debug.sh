#!/bin/bash -v

gcc -O ctl_debug.c ctl_orm.c data_orm.c hexbd.c hexbd_config.c spi_common.c -l bcm2835 -o ctl_debug.exe

sudo ./ctl_debug.exe > debug1.dat
