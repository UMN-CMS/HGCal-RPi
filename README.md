# HGCal-RPi:cleanup

This branch is the current cleanup effort of the Raspberry Pi code.
The list of changes includes:
  - Moved header files into their own respective `include/` directories
  - Added source files for the FPGA programming binary, as well as some others for reading the DIP switches
  - Moved program\_fpga into the bin/ directory and the firmware files into the fw/ directory
  - Merged RDOUT\_BOARD\_IPBus/rdout\_software with RDOUT\_BOARD\_IPBus
  - Renamed RDOUT\_BOARD\_IPBus/ to rdout/ and SYNCH\_BOARD/ to sync/
  - Removed unnecessary code in new\_rdout.c and sync\_debug.c
  - Removed unused files
