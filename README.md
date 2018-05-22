# HGCal-RPi : cleanup

This branch is the current cleanup effort of the Raspberry Pi code.
The readout board Raspberry Pi code is in `rdout/`, and the sync board Pi code is in `sync/`.

## Instructions

### 1. Setup
Modify `RDOUT_ALIASES` and `SYNC_ALIASES` in `etc/config` to be the ssh aliases of all your readout and sync boards.
As an example, if you have 3 readout boards with ssh aliases `rdout0` through `rdout2` and one sync board with ssh alias`sync0`, `etc/config` should read:
```bash
#!/bin/bash

# function to check return values
check_retval () {
    if [ $1 -ne 0 ]
    then
        exit 1
    fi
}

# general variables
RDOUT_ALIASES=("rdout0 rdout1 rdout2")      # ssh aliases of the readout board pis
SYNC_ALIASES=("sync0")                      # ssh aliases of the sync board pis
PI_HOMEDIR="/home/pi"
PI_RDOUTDIR="$PI_HOMEDIR/cleanup-rdout/"    # directory to copy code into on rdout pis
PI_SYNCDIR="$PI_HOMEDIR/cleanup-sync/"      # directory to copy code into on sync pis
RDOUT_EXE="new_rdout.exe"                   # name of the rdout executable
SYNC_EXE="sync_debug.exe"                   # name of the sync executable
```

### 2. Stop Previously Running Executables
Make sure there are no running executables on the pis with `./stop_pi_exes`.
Each ssh alias will be printed in the order of execution.
If no executable is running on that Pi, you will see a line containing `no process found`.
If an executable is running, no output will be produced.
An example output is as follows (with `rdout` and `sync` as the ssh aliases)
```
sync:                            # syncboard Pi ssh alias
sync_debug.exe: no process found # not printed if there is a process running
rdout:                           # rdoutboad Pi ssh alias
new_rdout.exe: no process found  # not printed if there is a process running
```
### 3. IPBus Setup
Setup IPBus with `./setup_ipbus`.
Each ssh alias will be printed just like in the previous step.
The board number will be printed after readout board aliases.
The board number starts from 0 and determines the IP address of the CTL ORM.
The IP is determined in `rdout/src/set_ip_mac_address.c` - currently, the structure is `192.168.222.[BOARD NUMBER]`.
An example output for this script follows:
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

The script should print an error message and halt if any problems occur.

### 4. Start Executables
Run `./start_pi_exes` to start the executables on the Pis.
It first checks and stops any currently running executables, so you will likely see more `no process found` lines.
An example output is below:
```
sync:
sync_debug.exe: no process found
32365
rdout:
new_rdout.exe: no process found
1535
```

The process number is printed for each alias.
If no number is printed, the executable was not properly started.
There is currently no error checking for this, so check for each number.

### 5. Start Data Taking
Once these executables are started, you can begin taking data with EUDAQ.

### 6. Finishing Up
Once data taking is done, stop the executables again with `./stop_pi_exes`.


## Documentation
The (incomplete) list of changes includes:
  - Moved header files into their own respective `include/` directories
  - Added source files for the FPGA programming binary, as well as some others for reading the DIP switches
  - Moved program\_fpga into the bin/ directory and the firmware files into the fw/ directory
  - Merged RDOUT\_BOARD\_IPBus/rdout\_software with RDOUT\_BOARD\_IPBus
  - Renamed RDOUT\_BOARD\_IPBus/ to rdout/ and SYNCH\_BOARD/ to sync/
  - Removed unnecessary code in new\_rdout.c and sync\_debug.c
  - Removed unused files
