# HGCal-RPi : cleanup

This branch is the current cleanup effort of the Raspberry Pi code.
The readout board Raspberry Pi code is in `rdout/`, and the sync board Pi code is in `sync/`.

## Instructions
  1. Modify `RDOUT_ALIASES` and `SYNC_ALIASES` in `etc/config` to be the ssh aliases of all your readout and sync boards
     (i.e. `RDOUT_ALIASES=("rdout0" "rdout1" "rdout2")`)
  2. Make sure there are no running executables on the pis with `./stop_pi_exes`.
     Each ssh alias should be printed, and if no running process was found on that pi, you will see a line containing `no process found`
  3. Setup IPBus with `./setup_ipbus`.
     An example output follows:
    ```
    sync:
    Programming SYNC ORM
    rdout (0):
    Programming DATA ORM0
    Programming DATA ORM1
    Programming DATA ORM2
    Programming DATA ORM3
    Programming CTL ORM
    Setting IPBus IP/MAC
    ```
     The script will exit after printing an error message if an error occurs.
  4. Start executables with `./start_pi_exes`.
     An example output follows
```
sync:
sync_debug.exe: no process found
31489
rdout:
new_rdout.exe: no process found
1128
```
     If a number does not appear for one of the pis, the executable was not started.
  5. Start EUDAQ, take runs, etc...
  6. Stop runs again with `./stop_pi_exes`.

## Documentation
The (incomplete) list of changes includes:
  - Moved header files into their own respective `include/` directories
  - Added source files for the FPGA programming binary, as well as some others for reading the DIP switches
  - Moved program\_fpga into the bin/ directory and the firmware files into the fw/ directory
  - Merged RDOUT\_BOARD\_IPBus/rdout\_software with RDOUT\_BOARD\_IPBus
  - Renamed RDOUT\_BOARD\_IPBus/ to rdout/ and SYNCH\_BOARD/ to sync/
  - Removed unnecessary code in new\_rdout.c and sync\_debug.c
  - Removed unused files
