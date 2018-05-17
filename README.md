# HGCal-RPi:new-prog-exe

The executable which flashes the FPGAs (`ProgramFPGA`) has been updated.
It now waits until it can properly read the chip ID back until exiting, and declares failure if this does not happen within 15 seconds.
This extra wait time fixes issues when trying to set the IPBus IP/MAC addresses immediately after programming the CTL ORM.
