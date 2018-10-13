# HGCal-RPi : CERNtb-oct18

This branch is used at CERN for the October 2018 beam tests.
[GNU Parallel](https://www.gnu.org/software/parallel/) is used in some scripts to control the Pis.
The ssh aliases in `etc/rdoutpis` and `etc/syncpis` are for the current setup.

The computer where this repository is cloned acts as the central hub for the Pis.
The Raspberry Pi software and ORM firmware are copied out from the hub at the start of each run using `rsync`, ensuring each is running the latest versions.
Boards are designated as readout or sync in `etc/rdoutpis` and `etc/syncpis`, and their respective code/fw is in `rdout/` and `sync/`.
A number of shell scripts are provided to facilitate this process.

There is a special timing hexaboard which requires a special configuration.
The folder `timing_hexbd/` contains a special `hexbd_config.c` to deal with this.


## Table of Contents
  * [Instructions](#instructions)
     * [1. Setup](#1-setup)
     * [2. Start](#2-start)
        * [1. Stop Previously Running Executables](#1-stop-previously-running-executables)
        * [2. IPBus Setup](#2-ipbus-setup)
        * [3. Start Executables](#3-start-executables)
        * [4. Start Data Taking](#4-start-data-taking)
        * [5. Finishing Up](#5-finishing-up)
  * [Documentation](#documentation)
     * [sync_debug](#sync_debug)
     * [new_rdout](#new_rdout)
     * [Power Cycling](#power-cycling)
     * [FPGA Programming](#fpga-programming)
     * [Setting IPBus IP](#setting-ipbus-ip)
  * [Current Software/Firmware](#current-softwarefirmware)


## Instructions

### 1. Setup
Modify `etc/rdoutpis` and `etc/syncpis` to contain the ssh aliases of the Raspberry Pis on your readout and sync boards.
With the current setup, we have two crates.
Crate 1 contains rdoutboard Pis 50-57, and crate 2 contains rdoutboard Pis 58-63,65 and syncboard Pi 20.
Crate 1 is connected to `em3` on the server, and crate 2 is connected to `em2`.
Since we are using two NICs on the same subnet, we must add routes:
```
route add -host 192.168.222.50 dev em3
```
This is done automatically on svhgcal01 using the script `etc/setup_routes`.
Use this script after a reboot, since the routing table we create is not saved.
Modify the variable `addroutes` in the script to add/delete routes as necessary.

### 2. Start
After setup, run `./reset` to get the Pis ready for data taking.
The `reset` script encompasses the following steps:

#### 1. Stop Previously Running Executables
Make sure there are no running executables on the Pis with `./stop_pi_exes`.
An error message will be printed if this was unsuccessful on a Pi.

#### 2. Power Cycle
Power cycle the ORMs on each rdout board with `etc/pwr_cycle`.
If a power cycle fails (usually because the syncboard cable is disconnected -> no clock), an error message will be printed.

#### 3. IPBus Setup
Setup IPBus with `./setup_ipbus`.
The ORMs on that board will be programmed - this can be changed with the `DOPROG` variable in the `setup_ipbus` script.
The board number starts from 0 and increments while it iterates through `etc/rdoutpis`.
The IP is determined in `rdout/src/set_ipbus_ip.c` - currently, the structure is `192.168.222.[200 + BOARD NUMBER]`.
An error message will be printed if this was unsuccessful on a Pi.

#### 4. Start Executables
Run `./start_pi_exes` to start the executables on the Pis.
If the process was not started (i.e. there is no pid for the process), the script will print an error message.

#### 5. Start Data Taking
Once these executables are started, you can begin taking data with EUDAQ.

#### 6. Finishing Up
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
  4. The Raspberry Pi sends the `RDOUT_DONE` signal to the syncboard, indicating we are ready for the next trigger
  5. We wait for a trigger
  6. Once we have recieved a trigger, each hexaboard is sent the start conversion command (`CMD_STARTCONPUL`) and then the start readout command (`CMD_STARTROPUL`)
  7. We wait until the FIFO is empty, as an empty FIFO indicates that the IPBus readout is complete and we can safely move on to the next event.
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

### Power Cycling
The script to power cycle the ORMs is `etc/pwr_cycle`.
This power cycles all 4 data ORMs and the CTL ORM on each readout board.
There is a small hardware modification on the RDOUTv2 boards that allow them to keep the hexaboards powered.
These boards must use the `rdout/bin/pwr_cycle_v2` power cycling executable, while the original v1 boards use `rdout/bin/pwr_cycle_v1`
The hexaboards do not recieve a clock while the ORMs are power cycling in the case of the v2 boards.
Be sure that the hexaboards turn back on before taking data - sometimes they may need to be warmed up before they will start up.

### FPGA Programming
The ORMs can be reprogrammed using the `etc/prog_orms` script.
This programs all of the readout and sync board ORMs.
The `rdout/prog_rdout` and `sync/prog_sync` scripts get run on the Pis over ssh - these contain the names of the firmware to be used.
You can also check the [Current Software/Firmware](#current-softwarefirmware) section for the hex files to use; this should be updated whenever there is new firmware.
If programming an ORM fails, you will be notified.
You can check the `prog.log` file on the Pi where the programming failed to see what went wrong.
Make sure any executables are stopped before running this script.

### Setting IPBus IP
The IPBus IP addresses on each ctl ORM are set during execution of `setup_ipbus`, which calls `rdout/set_ipbus_ip` on the Pis.
The IP form is currently `192.168.222.[200+BOARD NUMBER]` where the board number is determined by the position of the Pi's alias in `etc/config` (starting from 0).
This can be changed in `rdout/src/set_ipbus_ip.c`.
This script will print an error message and exit if the IP setting fails for some reason.
You can check `ip.log` on the board that failed to see what happened.

## Current Software/Firmware
These are the current versions:
### Firmware
  - CTL ORM: `ctl_orm_cs.hex`
  - DATA ORM: `data_orm_dly.hex`
  - SYNC ORM: `sync_orm_busy2.hex`

### Hexaboard Config Strings
```c
// the normal config string
char prog_string[48] = 
{   0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2c, 0xe0, 0x98, 0xb0, \
    0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff, \
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
    0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
};

// to mask channel 22
char maskch22_prog_string[48] =
{   0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2c, 0xe0, 0x98, 0xb0, \
    0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff, \
    0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
    0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, \
    0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
};

// for the timing hexaboard
char timing_prog_string[48] = 
{   0xDA,0xA0,0xFF,0x32,0xE0,0xC1,0x2E,0x10,0x98,0xB0,  \
    0x40,0x00,0x20,0x08,0x00,0x00,0x00,0x00,0x1F,0xFF,  \
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  \
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  \
    0xFF,0xFF,0xE9,0xD7,0xAE,0xBA,0x80,0x25
};
```
