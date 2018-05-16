# HGCal-RPi

This repository contains the software and firmware to be placed on the Raspberry Pis.
A cleanup effort is being made on the `cleanup` branch.

There are two main files: `new_rdout` located in `RDOUT_BOARD_IPBus/rdout_software/` and `sync_debug` in `SYNCH_BOARD/`.
These two files act as helpers to facilitate the data collection process with IPBus.

## sync\_debug
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

## new\_rdout
This program facilitates the IPBus readout.
At startup, resets are performed and general information is printed.
During each iteration of the event loop, the following actions are performed:
1. Each hexaboard is sent the reset command (`CMD_RESETPULSE`)
2. Each hexaboard is sent the command to start acquisition (`CMD_SETSTARTACQ`)
3. The CTL's FIFOs are reset
4. We wait for a trigger
    * The `date_stamp` register is used to tell the syncboard when we are OK for the next trigger. This register is set to 1 when we are OK to recieve one, and set to 0 when we don't acurrently want one (while we wait for readout to complete, etc...).
5. Once we have recieved a trigger, each hexaboard is sent the start conversion command (`CMD_STARTCONPUL`) and then the start readout command (`CMD_STARTROPUL`)
6. We wait until the FIFO is empty, as an empty FIFO indicates that the IPBus readout is complete and we can safely move on to the next event.
This process is repeated for each event until the data taking is complete.
