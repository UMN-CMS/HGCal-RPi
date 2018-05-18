# HGCal-RPi:cleanup

This branch is the current cleanup effort of the Raspberry Pi code.
The (incomplete) list of changes includes:
  - Moved header files into their own respective `include/` directories
  - Added source files for the FPGA programming binary, as well as some others for reading the DIP switches
  - Moved program\_fpga into the bin/ directory and the firmware files into the fw/ directory
  - Merged RDOUT\_BOARD\_IPBus/rdout\_software with RDOUT\_BOARD\_IPBus
  - Renamed RDOUT\_BOARD\_IPBus/ to rdout/ and SYNCH\_BOARD/ to sync/
  - Removed unnecessary code in new\_rdout.c and sync\_debug.c
  - Removed unused files

## Instructions
More will come soon.
This has been tested with EUDAQ at UMN.
Before running, edit `start_pi_exes`, `stop_pi_exes`, and `setup_ipbus` to contain the correct rdout and sync pi aliases.
To run:
  1. Make sure there are no running executables on the pis with `./stop_pi_exes`
  2. Setup IPBus with `./setup_ipbus`
  3. Start executables with `./start_pi_exes`
  4. Start EUDAQ, take runs, etc...
  5. Stop runs again with `./stop_pi_exes`
