// gcc -O sync_debug.c sync_orm.c spi_common.c -l bcm2835 -o sync_debug.exe

#include "sync_orm.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RESET_IN_LOOP 0 // Resets trigger counter per loop iteration if 1
                        // This sets rdout_done_count and prbs_error_count
                        // to 0. Use this when doing prbs stuff, since the
                        // error count fills up fast.

#define VERBOSE1 1 // print rdout_done_count[]
#define VERBOSE2 1 // print prbs_error_count[]

int main(int argc, char *argv[])
{
	// Startup the SPI interface on the Pi.
	init_spi();

	// Reset all.
	SYNC_reset_all();

	// Get the firmware version.
	int version;
	version = SYNC_get_firmware_version();
	fprintf(stderr,"version = 0x%04x\n",(int)version);

	// Test the constant registers, for debug.
	int constant0, constant1;
	constant0 = SYNC_get_constant0();
	constant1 = SYNC_get_constant1();
	fprintf(stderr,"constant = 0x%04x 0x%04x\n",
			(int)constant1, (int)constant0);

	// Test the dummy registers, for debug.
	int dummy0, dummy1;
	SYNC_put_dummy0(0xABCD);
	SYNC_put_dummy1(0x1234);
	dummy0 = SYNC_get_dummy0();
	dummy1 = SYNC_get_dummy1();
	fprintf(stderr,"dummy = 0x%04x 0x%04x\n",
			(int)dummy1, (int)dummy0);

	// Set the done_delay.
	int delay;
	SYNC_put_delay_done(8);
	delay = SYNC_get_delay_done();
	fprintf(stderr,"delay = 0x%04x\n",(int)delay);

	// Get the rdout_mask, via spi query of io expander.
	int cables_mask;
	cables_mask = SYNC_cables_connected();
	fprintf(stderr,"cables_mask = 0x%04x\n",(int)cables_mask);

	// Set the rdout_mask.
	int rdout_mask;
	rdout_mask = cables_mask;
	SYNC_put_rdout_mask(rdout_mask);
	int mask;
	mask = SYNC_get_rdout_mask();
	fprintf(stderr,"rdout_mask = 0x%04x\n",(int)mask);

	// Disable the max_trig_count feature.
	int max_count0, max_count1, max_count2;
	SYNC_put_max_trig_count0(0);
	SYNC_put_max_trig_count1(0);
	max_count0 = SYNC_get_max_trig_count0();
	max_count1 = SYNC_get_max_trig_count1();
	fprintf(stderr,"max_count = 0x%04x 0x%04x\n",
			(int)max_count1, (int)max_count0);

	// Disable the fake triggers.
	SYNC_put_enable_fake25(0);
	SYNC_put_enable_fake40(0);
	SYNC_put_enable_fake65(0);

	// Create ~100 Hz fake trigger rate based on 25 MHz clock.
	SYNC_put_max0_fake25((int)(249999 % (1<<16)));
	SYNC_put_max1_fake25((int)(249999 / (1<<16)));
	SYNC_put_thresh_fake25(25); // 1 usec
	SYNC_put_enable_fake25(1);

	// Enable vetos
	SYNC_put_enable_veto(0);
	int enabled_vetos = SYNC_get_enable_veto();
	fprintf(stderr, "enable_veto = %i\n", enabled_vetos);

	// Start sending triggers to the RDOUT boards.
	int enable_trig;
	int PED = atoi(argv[1]);
	if(PED) SYNC_put_enable_trig(0);
	else SYNC_put_enable_trig(1);
	enable_trig = SYNC_get_enable_trig();
	fprintf(stderr,"enable_trig = %d\n",(int)enable_trig);

	// end spi. It is initialized again in the loop
	end_spi();

	// Loop.
	int i, max_loops;
	int waiting, hold;
	int trig0, trig1;
	double trig_value;
	int indx;
	int rdout_done_count[16];
	int prbs_error_count[16];

	max_loops = 360; // Wait about an hour...
	i = 0;
	while(1) {
		sleep(1); // Sleep 10 seconds.


		// initialize SPI
		init_spi();

        // reset the trigger counter
        // this resets rdout_done_count and prbs_error_count
        if(RESET_IN_LOOP) {
            SYNC_reset_trig_counter();
        }

		// Get the rdout_mask, via spi query of io expander.
		cables_mask = SYNC_cables_connected();
		fprintf(stderr,"cables_mask = 0x%04x\n",(int)cables_mask);

		// Set the rdout_mask.
		rdout_mask = cables_mask;
		SYNC_put_rdout_mask(rdout_mask);
		mask = SYNC_get_rdout_mask();
		fprintf(stderr,"rdout_mask = 0x%04x\n",(int)mask);

		// Get the current readout status.
		waiting = SYNC_get_waiting_status();
		hold = SYNC_get_rdout_done_hold();

		// Get the trigger counter.
		trig0 = SYNC_get_trig_count0();
		trig1 = SYNC_get_trig_count1();
		trig_value = trig0;
		trig_value += trig1 * 65536.0;

		// Get rdout_done_count[]
		for (indx=0; indx<16; indx++) rdout_done_count[indx] = 0;
		if (VERBOSE1) {
		  for (indx=0; indx<16; indx++) {
		    rdout_done_count[indx] = SYNC_get_rdout_done_count(indx);
		  }
		}

		// Get prbs_error_count[]
		for (indx=0; indx<16; indx++) prbs_error_count[indx] = 0;
		if (VERBOSE2) {
		  for (indx=0; indx<16; indx++) {
		    prbs_error_count[indx] = SYNC_get_prbs_error_count(indx);
		  }
		}

		// end spi
		end_spi();

		if (VERBOSE1) {
		  for (indx=0; indx<16; indx++) {
		    fprintf(stderr,"loop = %3d, rdout_done_count[%2d] = %6d\n",
			    (int)i, (int)indx, (int)rdout_done_count[indx]);
		  }
		}
		if (VERBOSE2) {
		  for (indx=0; indx<16; indx++) {
		    fprintf(stderr,"loop = %3d, prbs_error_count[%2d] = %6d\n",
			    (int)i, (int)indx, (int)prbs_error_count[indx]);
		  }
		}
		fprintf(stderr,"\n");

		i++;
	}

	init_spi();

	// Stop sending triggers to the RDOUT boards.
	SYNC_put_enable_trig(0);
	enable_trig = SYNC_get_enable_trig();
	fprintf(stderr,"enable_trig = %d\n",(int)enable_trig);

	// Close the SPI interface on the Pi.
	end_spi();

	return(0);
}
