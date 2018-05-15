#!/bin/bash -v

sudo ./ProgramFPGA 1 < data_orm1.hex

sudo ./ProgramFPGA 2 < data_orm1.hex

sudo ./ProgramFPGA 3 < data_orm1.hex

sudo ./ProgramFPGA 4 < ctl_orm1.hex

#sudo ./ProgramFPGA 4 < ctl_orm_ipbus1.hex
