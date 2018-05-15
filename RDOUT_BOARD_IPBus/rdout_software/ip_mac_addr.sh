#!/bin/bash -v

gcc -O src/set_ip_mac_address.c src/ctl_orm.c src/spi_common.c -l bcm2835 -o bin/set_ip_mac_address.exe

sudo ./bin/set_ip_mac_address.exe

