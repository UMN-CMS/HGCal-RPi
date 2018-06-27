# HGCal-RPi:link-test

This branch contains code and firmware intended for testing connections between readout and sync boards.
It should be considered entirely separate from the other branches.


# Instructions
There are two main tests that can be run.

## RDOUT\_DONE Counting
This test is intended to test whether or not `RDOUT_DONE` signals sent through IPBus are correctly propagated through the readout board.
It can also show if the `RDOUT_DONE` signals sent from the readout board are not recieved by the sync board.

An executable is started up on the syncboard which shows the number of times it has recieved a `RDOUT_DONE` signal from each readout board.
IPBus then sends the `RDOUT_DONE` signal on a readout board (using the `test_done_count.py` script).
There is a special `RDOUT_DONE_COUNT` register on the readout board to ensure the signal was propagated through the board correctly.
This register is checked before sending `RDOUT_DONE` and after, and if it doesn't increment by one, the script exits.
This process is repeated many times.

If the python script exits before sending all 65535 signals, this means one of the `RDOUT_DONE` signals got lost inside the readout board after being sent by IPBus.
If instead the python script finishes but the `rdout_done_count` on the sync board does not match the number of signals sent, it is likely that a signal was lost between the readout board and the sync board.
If this happens, you can use prbs checking to test the rdout-sync link further.
If the python script finishes and the `rdout_done_count` on the sync board does match the number of signals sent, the link should be OK.

To run this test, first edit `copy-dirs` with the correct ssh aliases for the readout and sync boards you will be using.
The IPBus IP addresses of the readout boards will increment from 192.168.222.200.
Modify `etc/connections.xml` to have the same number of readout boards as you have, following the IPBus IP address scheme.
  1. Program the ORMs with `etc/prog_orms`.
  2. Setup the readout boards with `setup_ipbus`.
  3. Open a new terminal window, ssh into the sync board raspberry pi, and change into the copied directory: `cd ~/linktest-sync/`
  4. Start the sync exe with `./run_sync`.
  5. Exit the readout board ssh session. Setup IPBus using `source etc/env.sh`.
  6. Run `python test_done_count.py`. On the sync board screen, you should see the readout done count for the connected cable increase.
  7. Once the script finishes, stop the `sync_debug` executable.
  8. Change cable location on the sync board and repeat the test.

## PRBS Checking
This test is intended to test the link between readout and sync boards.
A pseudo-random bistream is sent from the readout board to the sync board where `RDOUT_DONE` signals are usually sent.
The sync board checks to see if the bitstream that is sent matches a given pseudo-random pattern.
If not, the `prbs_error_count` for that cable is incremented.

If the prbs generator is started on the readout board and the `prbs_error_count` for that cable is nonzero, something is likely wrong with that link.
It is expected that the `prbs_error_count` will be approximately maxed out (~65535) for unconnected cables or when the prbs generator is not running.

I will assume the RDOUT\_DONE counting test has been done already, so the `copy-dirs` script has been edited with the proper aliases, and two terminal windows are open: one on the computer running IPBus and the other in the `~/linktest-sync` directory on the sync board Raspberry Pi.
  1. Edit the file `sync/src/sync_debug.c` and set `RESET_IN_LOOP` to 1 on line 8. This resets the error count on each loop iteration, so you can start and stop the prbs generator while the script is running and see immediate results.
  2. The readout board should already be programmed and have its IP set. Run `./run_sync` on the sync board. You should see the same output as last time (except for possible count differences).
  3. Run `python prbs_start.py` to start the prbs generator on the readout board.
  4. Check that the `prbs_error_count` for the cable reduces to 0. The `rdout_done_count` should also increase.
  5. Run `python prbs_stop.py` to stop the prbs generator.
  6. Check that the `prbs_error_count` for the cable goes back to the maximum of 65535.
      - A little fluctuation is OK -- you will most likely see this fluctuation on the first few cables since the error count is still increasing from when it was reset at the start of the loop.
  7. Check the other cables. You should be able to hot-swap them without stopping the `sync_debug` executable.
