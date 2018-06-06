# HGCal-RPi

The computer where this repository is cloned acts as the central hub for the Pis.
The Raspberry Pi software and ORM firmware are copied out from the hub at the start of each run using `rsync`, ensuring each is running the latest versions.
Boards are designated as readout or sync in `etc/config`, and their respective code/fw is in `rdout/` and `sync/`.
A number of shell scripts are provided to facilitate this process.
The most notable scripts are `setup_ipbus`, `start_pi_exes`, and `stop_pi_exes`; others can be found in `etc/config`, `rdout/`, and `sync/` (though the last two folders contain scripts that are only run on the Pis after the corresponding directory has been copied over).


## Table of Contents
  * [Instructions](#instructions)
     * [1. Setup](#1-setup)
     * [2. Stop Previously Running Executables](#2-stop-previously-running-executables)
     * [3. IPBus Setup](#3-ipbus-setup)
     * [4. Start Executables](#4-start-executables)
     * [5. Start Data Taking](#5-start-data-taking)
     * [6. Finishing Up](#6-finishing-up)
  * [Documentation](#documentation)
     * [sync_debug](#sync_debug)
     * [new_rdout](#new_rdout)
  * [Current Software/Firmware](#current-softwarefirmware)


## Instructions

### 1. Setup
Modify `RDOUT_PI_ALIASES` and `SYNC_PI_ALIASES` in `etc/config` to be the ssh aliases of the Raspberry Pis on your readout and sync boards.
As an example, if you have 3 readout boards with rpi ssh aliases `rdout0` through `rdout2` and one sync board with rpi ssh alias`sync0`, `etc/config` should read:
```bash
RDOUT_PI_ALIASES=("rdout0 rdout1 rdout2")   # ssh aliases of the readout board pis
SYNC_PI_ALIASES=("sync0")                   # ssh aliases of the sync board pis
```

### 2. Stop Previously Running Executables
Make sure there are no running executables on the pis with `./stop_pi_exes`.
Each ssh alias will be printed in the order of execution.
If no executable is running on that Pi, you will see a line containing `no process found`.
If an executable is running, no output will be produced.
An example output is as follows (with `rdout` and `sync` as the ssh aliases)
```
sync:                            # syncboard Pi ssh alias
sync_debug: no process found     # not printed if there is a process running
rdout:                           # rdoutboad Pi ssh alias
new_rdout: no process found      # not printed if there is a process running
```
### 3. IPBus Setup
Setup IPBus with `./setup_ipbus`.
Each ssh alias will be printed just like in the previous step.
The board number will be printed after readout board aliases.
The board number starts from 0 and determines the IPBus IP address of the CTL ORM.
The IP is determined in `rdout/src/set_ipbus_ip.c` - currently, the structure is `192.168.222.[200 + BOARD NUMBER]`.
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
sync_debug: no process found
32365
rdout:
new_rdout: no process found
1535
```

The process number is printed for each alias.
If the process was not started (i.e. there is no pid for the process), the script will print an error message and exit.

### 5. Start Data Taking
Once these executables are started, you can begin taking data with EUDAQ.

### 6. Finishing Up
Once data taking is done, stop the executables again with `./stop_pi_exes`.


## Documentation

There are two main files: `new_rdout.c` located in `rdout/src/` and `sync_debug.c` in `sync/src/`.
These two files act as helpers to facilitate the data collection process with IPBus.
Both need to be started on each sync/readout board during each data-taking session.
They do not necessarily need to be restarted for each run.

### sync\_debug
This program prints out statistics to help with debugging while a run is being taken.
A typical printout is as follows (with comments after the hashes):
```
# these lines are printed at startup
version = 0x1105                                    # the fw version
constant = 0xbeef 0xdead                            # general constants
dummy = 0x1234 0xabcd
delay = 0x0008                                      # clock delay
cables_mask = 0x1000                                # mask showing connected cables
rdout_mask = 0x1000                                 # user-provided mask
max_count = 0x0000 0x0000
enable_veto = 0                                     # accept/ignore vetos
enable_trig = 1                                     # same for triggers

# these lines are printed once per loop
cables_mask = 0x1000
rdout_mask = 0x1000
loop =   0, waiting_for_trig = 0                    # are we waiting for the next trigger?
loop =   0, veto1 = 0                               # current veto status
loop =   0, veto2 = 0
loop =   0, rdout_done = 0                          # is the current readout done?
loop =   0, trig_reset = 0
loop =   0, maxed_out = 0
loop =   0, hold= 0xefff                            # mask telling which rdout boards are on hold
loop =   0, trigger = 0x0000 0x03e9       1001      # trigger count
```

### new\_rdout
This program facilitates the IPBus readout.
At startup, resets are performed and general information is printed.
During each iteration of the event loop, the following actions are performed:
  1. Each hexaboard is sent the reset command (`CMD_RESETPULSE`)
  2. Each hexaboard is sent the command to start acquisition (`CMD_SETSTARTACQ`)
  3. The CTL's FIFOs are reset
  4. We wait for a trigger
    - The `date_stamp` register is used to tell EUDAQ when we are OK for the next trigger.
      This register is set to 1 when we are OK to recieve one, and set to 0 when we don't currently want one (while we wait for readout to complete, etc...)
  5. Once we have recieved a trigger, each hexaboard is sent the start conversion command (`CMD_STARTCONPUL`) and then the start readout command (`CMD_STARTROPUL`)
  6. We wait until the FIFO is empty, as an empty FIFO indicates that the IPBus readout is complete and we can safely move on to the next event.
   This process is repeated for each event until the data taking is complete.

An example output is as follows (with comments after the hashes):
```
# General Info Printouts
date_stamp = 0x0000 0x0000                  # currently used for trigger OK - see above
ctl firmware version = 0x3002               # firmware versions
orm_0 firmware version = 0x2002
orm_1 firmware version = 0x2002
orm_2 firmware version = 0x2002
orm_3 firmware version = 0x2002
CTL constant = 0xbeef 0xdead                # general constants
dummy = 0x0000 0x0000
block_size = 30720                          # size of a block of data in 32-bit words
emptying local fifos (partially)...done.

# this section provides hexaboard information
hexbd: 0
hexbd: 0, queue error = 0                   # if any errors are 1, the hexaboard is not active
hexbd: 0, fifo error = 0
hexbd: 0, loop error = 1
hexbd: 1
hexbd: 1, queue error = 0                   # loop error is the most common - it means that
hexbd: 1, fifo error = 0                    # the cable is disconnected (i.e. no hexaboard)
hexbd: 1, loop error = 1
hexbd: 2
hexbd: 2, queue error = 0
hexbd: 2, fifo error = 0
hexbd: 2, loop error = 1
hexbd: 3
hexbd: 3, queue error = 0
hexbd: 3, fifo error = 0
hexbd: 3, loop error = 1
hexbd: 4
hexbd: 4, queue error = 0                   # this board is OK!
hexbd: 4, fifo error = 0                    # compare this to hexbd_mask and skiroc_mask below
hexbd: 4, loop error = 0
hexbd: 5
hexbd: 5, queue error = 0
hexbd: 5, fifo error = 0
hexbd: 5, loop error = 1
hexbd: 6
hexbd: 6, queue error = 0
hexbd: 6, fifo error = 0
hexbd: 6, loop error = 1
hexbd: 7
hexbd: 7, queue error = 0
hexbd: 7, fifo error = 0
hexbd: 7, loop error = 1

hexbd_mask = 0x10                           # bitmask to tell you which hexaboards are online
skiroc_mask = 0x000f 0x0000                 # bitmask to tell you which skirocs are online
Configuring hexbd 4...done.                 # hexaboard configuration (done for all active modules)

Emptying local fifos (partially)...done.
Sleeping...done.
date_stamp = 0x3434 0xaabb                  # dummy date stamp

Start events acquisition
block_size = 30000
skiroc_mask = 0x000f 0x0000
``` 


## Current Software/Firmware
These are the current versions:
### Firmware
  - CTL ORM: `ctl_orm_dly.hex`
  - DATA ORM: `data_orm_dly.hex`
  - SYNC ORM: `sync_orm_busy2.hex`

### Hexaboard Config String
```c
char prog_string[48] = 
{  0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2e, 0x10, 0x98, 0xb0,	\
   0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff,	\
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
   0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
};
```
