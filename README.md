# HGCal-RPi : umn-cosmic

This branch is intended for use to read out a small cosmic ray detector in the lab at UMN.
This branch assumes a setup with:
* One readout board with RPi ssh alias `piR10`
* One sync board with RPi ssh alias `piS2`

Setup the DAQ hardware by running `./reset`.
This will power cycle the FPGAs on the readout board, setup the IPBus IP address, and start the helper executables on the Raspberry Pis.
After running this command, make sure you see all of the connected hexaboards in the skiroc mask.
Right to left in the skiroc mask reads top to bottom along the readout board ports.
A `0` appears in the skiroc mask for ports without a hexaboard, and an `f` appears for ports with a hexaboard.
A setup with two hexaboards plugged into the top two readout board ports would have `skiroc_mask = 0x000000ff`. 

If things get stuck, you can run the `./reset` command again.

If a hexaboard is plugged in but not visible in the skiroc mask, check that the LEDs on the module are on.
If they are not, you can use a heat gun to warm it up and get it to turn on (Assuming you can actually access the module, and we don't have a water cooler setup connected! The water cooler setup should already regulate the temperature of the modules, so this shouldn't be an issue.)
Run the `./reset` command and use the heat gun on the affected modules while the reset cycle runs, and it should turn on with the rest of the hexaboards.
Be sure to not melt things when using the heat gun!
Don't hold it in one spot for too long.

See the README on the master branch for more detailed instructions.
