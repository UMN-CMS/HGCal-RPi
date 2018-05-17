// gcc -O ctl_debug.c ctl_orm.c data_orm.c hexbd.c hexbd_config.c spi_common.c -l bcm2835 -o ctl_debug.exe

#include <stdio.h>
#include <stdlib.h>

#include "hexbd_config.h"
#include "ctl_orm.h"
#include "data_orm.h"

int main()
{
  int hx, orm;

  setbuf(stdout, NULL);

  // Startup the SPI interface on the Pi.
  init_spi();
  
  // Reset all.
  CTL_reset_all();
  
  // Get the firmware version.
  int version;
  version = CTL_get_firmware_version();
  fprintf(stderr,"ctl firmware version = 0x%04x\n",(int)version);
  version = DATA_get_firmware_version(0);
  fprintf(stderr,"orm_0 firmware version = 0x%04x\n",(int)version);
  version = DATA_get_firmware_version(1);
  fprintf(stderr,"orm_1 firmware version = 0x%04x\n",(int)version);
  version = DATA_get_firmware_version(2);
  fprintf(stderr,"orm_2 firmware version = 0x%04x\n",(int)version);
  version = DATA_get_firmware_version(3);
  fprintf(stderr,"orm_3 firmware version = 0x%04x\n",(int)version);

  // Test the constant registers, for debug.
  int constant0, constant1;
  constant0 = CTL_get_constant0();
  constant1 = CTL_get_constant1();
  fprintf(stderr,"constant = 0x%04x 0x%04x\n",
	  (int)constant1, (int)constant0);
  
  // Test the dummy registers, for debug.
  int dummy0, dummy1;
  CTL_put_dummy0(0xABCD);
  CTL_put_dummy1(0x1234);
  dummy0 = CTL_get_dummy0();
  dummy1 = CTL_get_dummy1();
  fprintf(stderr,"dummy = 0x%04x 0x%04x\n",
	  (int)dummy1, (int)dummy0);
  
  // Get the data stamp.
  int date_stamp0, date_stamp1;
  CTL_put_date_stamp0(0xAABB);
  CTL_put_date_stamp1(0x3434);
  date_stamp0 =  CTL_get_date_stamp0();
  date_stamp1 =  CTL_get_date_stamp1();
  fprintf(stderr,"date_stamp = 0x%04x 0x%04x\n",
	  (int)date_stamp1, (int)date_stamp0);

  // Get the mac_address.
  int mac_address0, mac_address1, mac_address2;
  mac_address0 = CTL_get_mac_address0();
  mac_address1 = CTL_get_mac_address1();
  mac_address2 = CTL_get_mac_address2();
  fprintf(stderr,"mac_address = 0x%04x 0x%04x 0x%04x\n",
	  (int)mac_address2, (int)mac_address1, (int)mac_address0);
  
  // Get the ip_address.
  int ip_address0, ip_address1;
  ip_address0 = CTL_get_ip_address0();
  ip_address1 = CTL_get_ip_address1();
  fprintf(stderr,"ip_address = 0x%04x 0x%04x\n",
	  (int)ip_address1, (int)ip_address0);
  
  // Disable the debug_mode dump into the skiroc fifos.
  //CTL_put_disable_debug(1);
  //int disable_debug;
  //disable_debug = CTL_get_disable_debug();
  //fprintf(stderr,"disable_debug = %d\n",(int)disable_debug);

  DATA_put_debug_mode(0,1);
  DATA_put_debug_mode(1,1);
  DATA_put_debug_mode(2,1);
  DATA_put_debug_mode(3,1);

  int debug_mode;
  debug_mode =  DATA_get_debug_mode(0);
  fprintf(stderr,"debug_mode0 = %d\n",(int)debug_mode);
  debug_mode =  DATA_get_debug_mode(1);
  fprintf(stderr,"debug_mode1 = %d\n",(int)debug_mode);
  debug_mode =  DATA_get_debug_mode(2);
  fprintf(stderr,"debug_mode2 = %d\n",(int)debug_mode);
  debug_mode =  DATA_get_debug_mode(3);
  fprintf(stderr,"debug_mode3 = %d\n",(int)debug_mode);

  // Disable the xfer, so the SPI commands to hexaboards are allowed.
  int xfer_disable;
  for (orm=3; orm>=0; orm--) DATA_put_xfer_disable(orm,3);
  fprintf(stderr,"xfer_disable = ");
  for (orm=0; orm<4; orm++) {
    xfer_disable = DATA_get_xfer_disable(orm);
    fprintf(stderr,"0x%x ",(int)xfer_disable);
  }
  fprintf(stderr,"\n");

  // Reset the hexaboard local fifos.
  for (hx=0; hx<8; hx++) DATA_reset_local_fifo(hx);

  // Run a test on each of the8 hexaboards looking for good communication.
  int hexbd_mask;
  hexbd_mask = HEXBD_verify_communication(1);
  fprintf(stderr,"hexbd_mask = 0x%02x\n",(int)hexbd_mask);

  //hexbd_mask = 1; // debug

  // Set the skiroc mask.
  int skiroc_mask0, skiroc_mask1;
  skiroc_mask0 = 0;
  for (hx=0; hx<4; hx++) {
    if ((hexbd_mask & (1<<hx)) != 0) skiroc_mask0 |= 0xF << (4*hx);
  }
  skiroc_mask1 = 0;
  for (hx=4; hx<8; hx++) {
    if ((hexbd_mask & (1<<hx)) != 0) skiroc_mask1 |= 0xF << ((4*hx)-16);
  }
  CTL_put_skiroc_mask1(skiroc_mask1);
  CTL_put_skiroc_mask0(skiroc_mask0);
  
  // Get the skiroc mask.
  skiroc_mask1 = CTL_get_skiroc_mask1();
  skiroc_mask0 = CTL_get_skiroc_mask0();
  fprintf(stderr,"skiroc_mask = 0x%04x 0x%04x\n",
	  (int)skiroc_mask1, (int)skiroc_mask0);

  // After this reset, skiroc FIFOs should remain empty if masked out.
  CTL_reset_fifos();

  // Configure the active hexaboards here, before enabling the 
  // automatic xfer mechanism (which ignores hexaboard SPI commands).
  for (hx=0; hx<8; hx++) {
    if ((hexbd_mask & (1<<hx)) != 0) {
      fprintf(stderr,"Configuring hexbd %d...",(int)hx);
      configure_hexaboard(hx,0);
      configure_hexaboard(hx,1);
      fprintf(stderr,"done.\n");
    }
  }

  // Enable the xfer, so the SPI commands to hexaboards are NOT allowed.
  int two_bits;
  for (orm=0; orm<4; orm++) {
    two_bits = (hexbd_mask >> (2*orm)) & 3; 
    two_bits ^= 3; // disable , not enable
    DATA_put_xfer_disable(orm,two_bits);
  }
  fprintf(stderr,"xfer_disable = ");
  for (orm=3; orm>=0; orm--) {
    xfer_disable = DATA_get_xfer_disable(orm);
    fprintf(stderr,"0x%x ",(int)xfer_disable);
  }
  fprintf(stderr,"\n");

  // Get the skiroc fifo block_size.
  int block_size;
  block_size = CTL_get_block_size();
  fprintf(stderr,"block_size = 0x%04x\n", (int)block_size);

  // debug
  skiroc_mask1 = CTL_get_skiroc_mask1();
  skiroc_mask0 = CTL_get_skiroc_mask0();
  fprintf(stderr,"skiroc_mask = 0x%04x 0x%04x\n",
	  (int)skiroc_mask1, (int)skiroc_mask0);

  // Disable the debug_mode dump into the skiroc fifos.
  CTL_put_disable_debug(1);
  int disable_debug;
  disable_debug = CTL_get_disable_debug();
  fprintf(stderr,"disable_debug = %d\n",(int)disable_debug);

  // Reset the hexaboard local fifos.
  for (hx=0; hx<8; hx++) DATA_reset_local_fifo(hx);

  // debug
  skiroc_mask1 = CTL_get_skiroc_mask1();
  skiroc_mask0 = CTL_get_skiroc_mask0();
  fprintf(stderr,"skiroc_mask = 0x%04x 0x%04x\n",
	  (int)skiroc_mask1, (int)skiroc_mask0);

  // Reset skiroc fifos.
  CTL_reset_fifos();

  // debug
  skiroc_mask1 = CTL_get_skiroc_mask1();
  skiroc_mask0 = CTL_get_skiroc_mask0();
  fprintf(stderr,"skiroc_mask = 0x%04x 0x%04x\n",
	  (int)skiroc_mask1, (int)skiroc_mask0);

  // Get empty flags.
  int empty0, empty1;
  empty0 = CTL_get_empty0();
  empty1 = CTL_get_empty1();
  fprintf(stderr,"empty = 0x%04x 0x%04x\n",
	  (int)empty1, (int)empty0);
  
  // Get half_empty flags.
  int half_empty0, half_empty1;
  half_empty0 = CTL_get_half_empty0();
  half_empty1 = CTL_get_half_empty1();
  fprintf(stderr,"half_empty = 0x%04x 0x%04x\n",
	  (int)half_empty1, (int)half_empty0);

  //====================================================
  // Loop, using external triggers from SYNC board.
  //====================================================
  fprintf(stderr,"\n");

  int i, j, max_triggers;
  int value0, value1;
  int block_ready0, block_ready1;
  int usec0, usec1, usec2;
  int trig0, trig1;

  int block_ready, skiroc_mask, skiroc;
  skiroc_mask = (skiroc_mask1 << 16) | skiroc_mask0;

  // Send a pulse back to the SYNC board. Give us a trigger.
  fprintf(stderr,"Send a trigger!\n");
  CTL_put_done();

  // debug
  skiroc_mask1 = CTL_get_skiroc_mask1();
  skiroc_mask0 = CTL_get_skiroc_mask0();
  fprintf(stderr,"skiroc_mask = 0x%04x 0x%04x\n",
	  (int)skiroc_mask1, (int)skiroc_mask0);

  max_triggers = 4;
  for (i=0; i<max_triggers; i++) {

    // Wait for block_ready.
    fprintf(stderr,"\n");
    fprintf(stderr,"Waiting for data to arrive from the active skirocs...\n");

    block_ready = 0;
    while((block_ready & skiroc_mask) != skiroc_mask) {
      block_ready0 = CTL_get_block_ready0();
      block_ready1 = CTL_get_block_ready1();
      block_ready = (block_ready1<<16) | block_ready0;
    }
    fprintf(stderr,"block_ready = 0x%04x 0x%04x\n",
	    (int)block_ready1, (int)block_ready0);

    // Loop over the skirocs that are ready.
    for (skiroc=0; skiroc<32; skiroc++) {
      if (((block_ready & skiroc_mask) & (1<<skiroc)) != 0) {

	fprintf(stdout,"trigger = %3d, skiroc = %d\n", (int)i, (int)skiroc);
	fprintf(stderr,"trigger = %3d, skiroc = %d\n", (int)i, (int)skiroc);
	
	// Get the microsecond counter.
	usec0 = CTL_get_usec_count0();
	usec1 = CTL_get_usec_count1();
	usec2 = CTL_get_usec_count2();
	fprintf(stdout,"trigger = %3d, usecond = 0x%04x 0x%04x 0x%04x\n",
		(int)i, (int)usec2, (int)usec1, (int)usec0);
	fprintf(stderr,"trigger = %3d, usecond = 0x%04x 0x%04x 0x%04x\n",
		(int)i, (int)usec2, (int)usec1, (int)usec0);
	
	// Get the trigger counter.
	trig0 = CTL_get_trig_count0();
	trig1 = CTL_get_trig_count1();
	fprintf(stdout,"trigger = %3d, trigger = 0x%04x 0x%04x\n",
		(int)i, (int)trig1, (int)trig0);
	fprintf(stderr,"trigger = %3d, trigger = 0x%04x 0x%04x\n",
		(int)i, (int)trig1, (int)trig0);
	
	// Get a block of values from this skiroc fifo.
	fprintf(stdout,"\n");
	for (j=0; j<block_size; j++) {
	  value0 = CTL_get_fifo_LS16(skiroc);
	  value1 = CTL_get_fifo_MS16(skiroc);
	  fprintf(stdout,"trigger = %d, %6d, value = 0x%04x 0x%04x\n",
		  (int)i, (int)j, (int)value1, (int)value0);
	  fprintf(stderr,"trigger = %d, %6d, value = 0x%04x 0x%04x\n",
		  (int)i, (int)j, (int)value1, (int)value0);
	}
	fprintf(stdout,"\n");
	fprintf(stderr,"\n");
      }
    } // next skiroc

    // Send a pulse back to the SYNC board. Give us another trigger.
    fprintf(stderr,"Send a trigger!\n");
    CTL_put_done();

  } // next trigger
  
  // Close the SPI interface on the Pi.
  end_spi();
  
  return(0);
}
