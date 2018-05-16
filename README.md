# HGCal-RPi

This repository contains the software and firmware to be placed on the Raspberry Pis.
A cleanup effort is being made on the `cleanup` branch.

## General Execution
The procedure to start up the proper scripts on the Raspberry Pis is as follows:
  1. Stop any running programs on the Pis.
     This is often done using `pgrep` to find the process numbers and then killing them.
  2. Program the ORMs with the correct firmware.
     (Note: this step is usually only done at the start of a data-taking session or when things start to break)
  3. Set the IP and MAC addresses for IPBus on each readout board.
  4. Start up the main executables `new_rdout.exe` and `sync_debug.exe`.
     These must be left running during the whole data-taking session.
Information on the last three steps can be found below.

## Main Executables
There are two main files: `new_rdout.exe` located in `RDOUT_BOARD_IPBus/rdout_software/bin/` and `sync_debug.exe` in `SYNCH_BOARD/bin/`.
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
Compilation of this file is done using the `SYNCH_BOARD/compile` shell script.

### new\_rdout
This program facilitates the IPBus readout.
At startup, resets are performed and general information is printed.
During each iteration of the event loop, the following actions are performed:
  1. Each hexaboard is sent the reset command (`CMD_RESETPULSE`)
  2. Each hexaboard is sent the command to start acquisition (`CMD_SETSTARTACQ`)
  3. The CTL's FIFOs are reset
  4. We wait for a trigger
    - The `date_stamp` register is used to tell the syncboard when we are OK for the next trigger.
      This register is set to 1 when we are OK to recieve one, and set to 0 when we don't currently want one (while we wait for readout to complete, etc...)
  5. Once we have recieved a trigger, each hexaboard is sent the start conversion command (`CMD_STARTCONPUL`) and then the start readout command (`CMD_STARTROPUL`)
  6. We wait until the FIFO is empty, as an empty FIFO indicates that the IPBus readout is complete and we can safely move on to the next event.
   This process is repeated for each event until the data taking is complete.

An example output is as follows (with comments after the hashes):
```
# The following lines are unused with IPBus
The run number is: RUN_0200, Number of events: 2000000. Save Raw: 1 
Filename will be /home/pi/RDOUT_BOARD_IPBus/rdout_software/data/RUN_0200_111117_0140_RDOUT3.txt
Raw filename will be /home/pi/RDOUT_BOARD_IPBus/rdout_software/data/RUN_0200_111117_0140_RDOUT3.raw
Trigger timing file will be /home/pi/RDOUT_BOARD_IPBus/rdout_software/data/RUN_0200_111117_0140_TIMING_RDOUT3.txt

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

Event 1
# After this is just event counting
``` 
Compilation of this file is done using the `RDOUT_BOARD_IPBus/rdout_software/compile` shell script.

## ORM Programming 
At the start of a data-taking session or when things stop working, it is useful to reprogram all the ORMs on each readout and sync board.
This is accomplished using the `ProgramFPGA` executable, located in the `prgm_fpga/` folders of both `RDOUT_BOARD_IPBus/` (note that this is *not* the inner `rdout_software/` directory) and `SYNCH_BOARD/`.
The relevant firmware files ending in '.hex' are also located in these directories.
The program reads from stdin; the usage is:
```
sudo ./ProgramFPGA [ORM] < [HEX FILE]
```
ORMs 0-3 are the DATA ORMs - they are only present on the readout boards.
These should be programmed with the `data_*.hex` files.
ORM 4 is the CTL orm - present on both readout and sync boards.
For the readout board, the CTL should be programmed with the `ctl_*.hex` files.
For the sync board, the CTL should be programmed with the `sync_*.hex` files.
The newest firmwares are:
  - DATA: `data_orm1.hex`
  - RDOUT CTL: `ctl_orm_rst.hex`
  - SYNC CTL: `sync_orm1.hex`

Note that the `prog_all_orms.sh` shell script inside `RDOUT_BOARD_IPBus/prgm_fpga/` does all 4 DATA ORMs plus the CTL ORM.
There is no source code for this executable inside the current directories. It will be added to the `cleanup` branch as the cleanup progresses.

## IPBus IP/MAC Addresses
The IPBus IP/MAC addresses can be set using the `RDOUT_BOARD_IPBus/rdout_software/ip_mac_addr.sh` shell script.
This invokes `set_ip_mac_address.exe`, which sets the IP and MAC addresses according to flags set in the source file `src/set_ip_mac_address.c`.
There were 3 readout boards at the CERN test beam, which is why there are 3 flags.
Each board must have a unique IP and MAC address to be able to connect to it over the network.
