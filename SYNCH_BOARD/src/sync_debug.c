// gcc -O sync_debug.c sync_orm.c spi_common.c -l bcm2835 -o sync_debug.exe

#include "sync_orm.h"
#include <stdio.h>
#include <stdlib.h>


// RDOUT MASK OPTIONS:
#define PI1_MASK 0x0004
#define PI2_MASK 0x4000
#define PI3_MASK 0x0001
#define AHCAL_MASK 0x8000
// OR together the devices that you want (i.e. PI2_MASK | PI3_MASK)
#define RDOUT_MASK (PI1_MASK | PI2_MASK | PI3_MASK)

// ANABLE_VETO1 Options:
// 0 - ignores AHCAL
// 1 - listens to AHCAL
// If you include AHCAL in the RDOUT_MASK then you must listen to AHCL (ie set 1)
#define ENABLE_VETO1 0
#define ENABLE_VETO2 0// not gonna work anyways - not populated

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
	rdout_mask = RDOUT_MASK; // for debug
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
	SYNC_put_enable_veto((ENABLE_VETO2 << 1) | ENABLE_VETO1);
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

	max_loops = 360; // Wait about an hour...
	//for (i=0; i<max_loops; i++) {
	i = 0;
	while(1) {
		sleep(10); // Sleep 10 seconds.

		// initialize SPI
		init_spi();

		// Get the rdout_mask, via spi query of io expander.
		cables_mask = SYNC_cables_connected();
		fprintf(stderr,"cables_mask = 0x%04x\n",(int)cables_mask);

		// Set the rdout_mask.
		rdout_mask = cables_mask;
		rdout_mask = RDOUT_MASK; // for debug
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

		// end spi
		end_spi();

		// maxed_out, trig_reset, rdout_done, veto2, veto1, waiting_for_trig
		fprintf(stderr,"loop = %3d, waiting_for_trig = %d\n",(int)i, (int)(waiting>>0)&1);
		fprintf(stderr,"loop = %3d, veto1 = %d\n",(int)i, (int)(waiting>>1)&1);
		fprintf(stderr,"loop = %3d, veto2 = %d\n",(int)i, (int)(waiting>>2)&1);
		fprintf(stderr,"loop = %3d, rdout_done = %d\n",(int)i, (int)(waiting>>3)&1);
		fprintf(stderr,"loop = %3d, trig_reset = %d\n",(int)i, (int)(waiting>>4)&1);
		fprintf(stderr,"loop = %3d, maxed_out = %d\n",(int)i, (int)(waiting>>5)&1);

		fprintf(stderr,"loop = %3d, hold= 0x%04x\n",(int)i, (int)hold);
		fprintf(stderr,"loop = %3d, trigger = 0x%04x 0x%04x %10.0lf\n",
				(int)i, (int)trig1, (int)trig0, (double)trig_value);
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
