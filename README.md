# HGCal-RPi

HGCal-RPi provides tools to facilitate the readout of UMN-produced DAQ hardware for beam tests of prototype HGCal modules.

The computer where this repository is cloned acts as the central hub for the Raspberry Pis.
The 'hub' computer remotely starts and stops the readout helper processes on the Pis.
The Raspberry Pi software and ORM firmware are copied out from the hub at the start of each run using `rsync`, ensuring each is running the latest versions.
Boards are designated as readout or sync in `etc/rdoutpis` and `etc/syncpis`, and their respective code and firmware are in the `rdout/` and `sync/` directories.


## Table of Contents
* [Requirements](#requirements)
* [Instructions](#instructions)
  * [0. System Configuration](#0-system-configuration)
  * [1. Setup](#1-setup)
  * [2. Start](#2-start)
    * [1. Stop Previously Running Executables](#1-stop-previously-running-executables)
    * [2. Power Cycle](#2-power-cycle)
    * [3. IPBus Setup](#3-ipbus-setup)
    * [4. Start Executables](#4-start-executables)
    * [5. Check Hexaboard Connections](#5-check-hexaboard-connections)
* [Documentation](#documentation)
  * [sync_debug](#sync_debug)
  * [new_rdout](#new_rdout)
  * [Power Cycling](#power-cycling)
  * [FPGA Programming](#fpga-programming)
  * [Setting IPBus IP](#setting-ipbus-ip)
* [Current Software/Firmware](#current-softwarefirmware)
  * [Firmware](#firmware)
  * [Hexaboard Config Strings](#hexaboard-config-strings)


## Requirements
  * Server Software
    * [Bash](https://www.gnu.org/software/bash/)
      * Used to run all of the scripts (each expects bash in `/bin/bash`).
      * Version must be >= 4 for `readarray` support. Bash 4.1.2 is what has been used for the beam tests.
    * [GNU Parallel](https://www.gnu.org/software/parallel/)
      * Used in some scripts to remotely execute software on the pis and automatically format the output.
      * GNU Parallel must also be installed on the Raspberry Pis.
      * All scripts can be modified to not use parallel at all, if necessary. This removes the requirement for GNU Parallel on the Raspberry Pis as well.
      * Version 20150522 was used during the beam tests, though most (if not all) versions should work.
        * Needs support for running over ssh and the `--sshloginfile` options.
    * [rsync](https://rsync.samba.org/)
      * Used to sync the server software and firmware with the Raspberry Pis.
      * Version 3.0.6 was used during the beam tests, though most (if not all) versions should work.
  * Raspberry Pi Software
    * The Raspbian OS should provide gcc, bash, and others.
    * [GNU Parallel](https://www.gnu.org/software/parallel/)
      * Required for the parallel scripts on the server to work correctly.
    * [bcm2835 C Library](https://www.airspayce.com/mikem/bcm2835/)
      * Required to communicate with the FPGAs and other devices on the readout/sync boards.


## Instructions

### 0. System Configuration
For these instructions, we have assumed a 'dummy' setup, consisting of:
  * Two readout boards with Raspberry Pi ssh aliases `piR1` and `piR2`. There are two hexaboards connected to the top two ports of both readout boards.
  * One sync board with Raspberry Pi ssh alias `piS`
We assume the network has been set up correctly, ssh keys have been copied, and all requirements have been installed.
You can see the test beam branches for examples of real configurations.

### 1. Setup
Modify `etc/rdoutpis` and `etc/syncpis` to contain the ssh aliases of the Raspberry Pis on your readout and sync boards.
You should have the following in `etc/rdoutpis`:
```
piR1
piR2
```
And in `etc/syncpis`:
```
piS
```

### 2. Start
After setup, run `./reset` to get the Pis ready for data taking.
Once the script finishes, you will be ready to take data through IPBus.

The `reset` script encompasses the following steps:

#### 1. Stop Previously Running Executables
All running executables on the Pis are stopped with `./stop_pi_exes`.

An error message will be printed if this was unsuccessful on a Pi.
If this is unsuccessful, you should reboot the misbehaving Pis.

#### 2. Power Cycle
The ORMs on each readout board are power cycled with `./etc/pwr_cycle`.
This creates a log file `pwr.log` on each readout board.

If a power cycle fails, an error message will be printed.
Usually, this fails because the ORM is not receiving a stable clock.
If there is a failure, first make sure the sync board is on.
If it is on, try unplugging/replugging the sync board HDMI cables on both ends.
If this fails, reboot the misbehaving Pis and try again.
If none of the above steps work, there has either been corruption in the ORM configuration stored in the EEPROM, or the ORM is broken.
You can try programming the ORMs with `etc/prog_orms`, but if that does not work, you will have to reflash through JTAG or replace the ORM.


#### 3. IPBus Setup
The IPBus firmware on the ORMs is configured with `./setup_ipbus`.
This creates a log file `ip.log` on each readout board.
You can program the ORMs during this step with the `DOPROG` variable.
The board number starts from 0 and increments while it iterates through `etc/rdoutpis`.
  * The board with `piR1` will have board number 0 (IP 192.168.222.200 by default).
  * The board with `piR2` will have board number 1 (IP 192.168.222.201 by default).
The IP is determined in `rdout/src/set_ipbus_ip.c` - currently, the structure is `192.168.222.[200 + BOARD NUMBER]`.
After setting the IP addresses, the `etc/wait_for_ping` script is run to wait until all boards appear on the network.

An error message will be printed if this was unsuccessful on a Pi.
If this fails, follow the same debugging steps as seen in the [power cycle section](#2-power-cycle).

#### 4. Start Executables
The script `start_pi_exes` is run to start the executables on the Pis.
This creates a log file `rdout.log` on readout boards and `sync.log` on sync boards.

If the process was not started (i.e. there is no pid for the process), the script will print an error message.
Check the `rdout.log` file on the misbehaving Pis to see if the hexaboards failed to configure.
This may indicate a bad module, bad cable, or bad HDMI port.

#### 5. Check Hexaboard Connections
The script `etc/hexbd_conn` is run to display a printout of the connected hexaboards.
An example printout is below for the dummy setup, with comments after the `#`:
```
piR1:   0x0000 0x00ff (2 hexbds)    # top two hexaboards show all four skirocs - one bit per skiroc
piR2:   0x0000 0x00ff (2 hexbds)    # top->bottom on the readout board is right->left here
total:  2 hexbds
```
If there are any boards with < 1 hexbd, the printout will be red.
If you see any with -1, this means the executable was not started.


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
char prog_string[48] =
{   0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2c, 0xe0, 0x98, 0xb0, \
    0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff, \
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
    0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
};
```
