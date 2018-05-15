#!/bin/bash -v


## REAL
gcc -O new_rdout.c ejf_rdout.c ctl_orm.c data_orm.c hexbd.c hexbd_config.c spi_common.c -l bcm2835 -o new_rdout.exe
sudo ./new_rdout.exe 0 1000 > rdout.dat
# sudo ./new_rdout.exe [RUN NUMBER] [EVENTS]


## PEDESTAL
# gcc -O new_rdout_PED.c ejf_rdout.c ctl_orm.c data_orm.c hexbd.c hexbd_config.c spi_common.c -l bcm2835 -o new_rdout_PED.exe
# sudo ./new_rdout_PED.exe 0 1000 > rdout.dat
# sudo ./new_rdout_PED.exe [RUN NUMBER] [EVENTS]

