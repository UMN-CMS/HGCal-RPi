# HGCal-RPi:cleanup

This branch is the current cleanup effort of the Raspberry Pi code.
The list of improvements is:
  - Moved header files into their own respective `include/` directories
  - Added source files for the FPGA programming binary, as well as some others for reading the DIP switches
  - Moved ProgramFPGA into the bin/ directory and the firmware files into the fw/ directory
  - Merged RDOUT\_BOARD\_IPBus/rdout\_software with RDOUT\_BOARD\_IPBus
  - Renamed RDOUT\_BOARD\_IPBus/ to rdout/ and SYNCH\_BOARD/ to sync/
