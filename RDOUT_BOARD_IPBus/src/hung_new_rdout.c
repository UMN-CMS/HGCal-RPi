// gcc -O new_rdout.c ejf_rdout.c ctl_orm.c data_orm.c hexbd.c hexbd_config.c spi_common.c -l bcm2835 -o new_rdout.exe

#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "hexbd_config.h"
#include "ctl_orm.h"
#include "data_orm.h"
#include "ejf_rdout.h"
#include "spi_common.h"

#define AUTOPILOT 0
//const int Hexbd_Indices[MAXHEXBDS] = {0, 1, 2, 3, 4, 5, 6, 7};

int main(int argc, char *argv[])
{
  int res,status;
  int ch, sample, chip;
  int i, k, hexbd;
  time_t rawtime;
  bool saveraw;
  struct tm *info;
  char buffer[80];
  char fname [160];
  char dirname[] = "/home/pi/RDOUT_BOARD/data/";
  
  int maxevents = 1000;
  char instr [1024];
  FILE *fraw;
  FILE *fout;

  int hx;
  int junk[2000];
  
  // Setting up number of run events, file name, etc
  int runid = 0;
  
  if( argc < 3 ){
    fprintf(stderr,"You need to provide more arguments: <RunNumber> <Number of Events> \n");
    return(0);
  }
  
  runid = atoi(argv[1]);
  char runNum[8];
  sprintf (runNum, "RUN_%04d", runid);
  
  maxevents = atoi(argv[2]);
  
  saveraw = true;
  if (argc == 4 && atoi(argv[3]) == 0)
    saveraw = false;
  
  fprintf(stderr,"The run number is: %s, Number of events: %d. Save Raw: %d \n \n", runNum, maxevents, (int)saveraw);
  
  // Make up a file name for data
  time(&rawtime);
  info = localtime(&rawtime);
  strftime(buffer,80,"_%d%m%y_%H%M", info);
  
  strcpy(fname, dirname);
  strcat(fname, runNum);
  strcat(fname, buffer);
  strcat(fname,".txt");
  fprintf(stderr,"Filename will be %s\n",fname);

  fout = fopen(fname, "w");
  fprintf(fout,"\nTotal number of events: %d",maxevents);
  fprintf(fout,"\n%s\n##########################################\n",buffer);
  
  // optional save raw data
  strcpy(fname, dirname);
  strcat(fname, runNum);
  strcat(fname, buffer);
  strcat(fname,".raw");
  fprintf(stderr,"Raw filename will be %s\n",fname);

  fraw = fopen(fname, "w");

  // Startup the SPI interface on the Pi.
  init_spi();

  // Set the date stamp to zero, indicating we are not done initializing.
  int date_stamp0, date_stamp1;
  CTL_put_date_stamp0(0);
  CTL_put_date_stamp1(0);
  date_stamp0 =  CTL_get_date_stamp0();
  date_stamp1 =  CTL_get_date_stamp1();
  fprintf(stderr,"date_stamp = 0x%04x 0x%04x\n",
	  (int)date_stamp1, (int)date_stamp0);
  
  // Power cycle the ORMs.
  if (1) {
    fprintf(stderr,"power cycle orm: data_0...");
    power_cycle(0); // DATA_0
    fprintf(stderr,"done.\n");
    sleep(1);
    fprintf(stderr,"power cycle orm: data_1...");
    power_cycle(1); // DATA_1
    fprintf(stderr,"done.\n");
    sleep(1);
    fprintf(stderr,"power cycle orm: data_2...");
    power_cycle(2); // DATA_2
    fprintf(stderr,"done.\n");
    sleep(1);
    fprintf(stderr,"power cycle orm: data_3...");
    power_cycle(3); // DATA_3
    fprintf(stderr,"done.\n");
    sleep(1);
    fprintf(stderr,"power cycle orm: ctl...");
    power_cycle(4); // CTL
    fprintf(stderr,"done.\n");
    sleep(5);
  }

  // Disable the xfer, so the SPI commands to hexaboards are allowed.
  int xfer_disable;
  int orm;
  for (orm=3; orm>=0; orm--) DATA_put_xfer_disable(orm,3);
  fprintf(stderr,"xfer_disable = ");
  for (orm=0; orm<4; orm++) {
    xfer_disable = DATA_get_xfer_disable(orm);
    fprintf(stderr,"0x%x ",(int)xfer_disable);
  }
  fprintf(stderr,"\n");

  CTL_reset_all();
  DATA_reset_all(0);
  DATA_reset_all(1);
  DATA_reset_all(2);
  DATA_reset_all(3);
  DATA_reset_all(4);
  DATA_reset_all(5);
  DATA_reset_all(6);
  DATA_reset_all(7);

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
  fprintf(stderr,"CTL constant = 0x%04x 0x%04x\n",
	  (int)constant1, (int)constant0);
  
  // Test the dummy registers, for debug.
  int dummy0, dummy1;
  CTL_put_dummy0(0xABCD);
  CTL_put_dummy1(0x1234);
  dummy0 = CTL_get_dummy0();
  dummy1 = CTL_get_dummy1();
  fprintf(stderr,"dummy = 0x%04x 0x%04x\n",
	  (int)dummy1, (int)dummy0);

  // Get the skiroc fifo block_size.
  int block_size;
  block_size = CTL_get_block_size();
  fprintf(stderr,"block_size = 0x%04x\n", (int)block_size);
  
  // Reset the hexaboard local fifos.
  for (hx=0; hx<8; hx++) DATA_reset_local_fifo(hx);

  // empty local fifo by forcing extra reads, ignore results
  fprintf(stderr,"emptying local fifos (partially)...");
  for (hx=0; hx<8; hx++) HEXBD_read1000_local_fifo(hx,junk);
  fprintf(stderr,"done.\n");

  // Run a test on each of the8 hexaboards looking for good communication.
  int hexbd_mask;
  hexbd_mask = HEXBD_verify_communication(1);
  //hexbd_mask = 1; // debug
  fprintf(stderr,"hexbd_mask = 0x%02x\n",(int)hexbd_mask);

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

  // Enable the debug mode on the data orms, to fill the skiroc FIFOs.
  DATA_put_debug_mode(0,0);
  DATA_put_debug_mode(1,0);
  DATA_put_debug_mode(2,0);
  DATA_put_debug_mode(3,0);
  int debug_mode;
  debug_mode =  DATA_get_debug_mode(0);
  fprintf(stderr,"debug_mode0 = %d\n",(int)debug_mode);
  debug_mode =  DATA_get_debug_mode(1);
  fprintf(stderr,"debug_mode1 = %d\n",(int)debug_mode);
  debug_mode =  DATA_get_debug_mode(2);
  fprintf(stderr,"debug_mode2 = %d\n",(int)debug_mode);
  debug_mode =  DATA_get_debug_mode(3);
  fprintf(stderr,"debug_mode3 = %d\n",(int)debug_mode);

  // Configure the active hexaboards here, before enabling the 
  // automatic xfer mechanism (which ignores hexaboard SPI commands).
  int config_status;
  for (hx=0; hx<8; hx++) {
    if ((hexbd_mask & (1<<hx)) != 0) {
      
      // Verify that the command and response queues are empty.
      config_status = HEXBD_queue_status(hx);
      if ((config_status & 0x0101) != 0x0101) {
	fprintf(stderr,"hexbd: %d, queue error\n",(int)hx);
	exit(-1);
      }
      
      // Configure the hexaboard.
      fprintf(stderr,"Configuring hexbd %d...",(int)hx);
      config_status = configure_hexaboard(hx,0);
      config_status = configure_hexaboard(hx,1);
      fprintf(stderr,"done.\n");
      if (config_status < 0) {
	fprintf(stderr,"ERROR in configuration.\n");
	exit(-1);
      }
    }
  }

  // Reset the hexaboard local fifos.
  for (hx=0; hx<8; hx++) DATA_reset_local_fifo(hx);
  
  // empty local fifo by forcing extra reads, ignore results
  fprintf(stderr,"emptying local fifos (partially)...");
  for (hx=0; hx<8; hx++) HEXBD_read1000_local_fifo(hx,junk);
  fprintf(stderr,"done.\n");
  
  // Delay the start of "data taking" post configuration to 
  // stabilize the state of the chip
  //usleep(10000);
  sleep(10);

  // Startup the AUTOPILOT by enabling xfer, based on the hexbd_mask.
  // (SPI commands to hexaboards are allowed.)
  int two_bits;
  if (AUTOPILOT != 0) {
    for (orm=3; orm>=0; orm--) {
      two_bits = (hexbd_mask >> (2*orm)) & 3; 
      two_bits ^= 3; // disable , not enable
      DATA_put_xfer_disable(orm,two_bits);
    }
    fprintf(stderr,"xfer_disable = ");
    for (orm=0; orm<4; orm++) {
      xfer_disable = DATA_get_xfer_disable(orm);
      fprintf(stderr,"0x%x ",(int)xfer_disable);
    }
    fprintf(stderr,"\n");
  }

  // Set the date stamp to non-zero, indicating we are done initializing.
  CTL_put_date_stamp0(0xAABB);
  CTL_put_date_stamp1(0x3434);
  date_stamp0 =  CTL_get_date_stamp0();
  date_stamp1 =  CTL_get_date_stamp1();
  fprintf(stderr,"date_stamp = 0x%04x 0x%04x\n",
	  (int)date_stamp1, (int)date_stamp0);
  
  //===============================================================
  fprintf(stderr,"\nStart events acquisition\n");
  //===============================================================
  
  int usec0, usec1, usec2, trig0, trig1, old_trig0;
  double trig_value, usec_value;
  int fifo_ready, block_ready, block_ready0, block_ready1, skiroc, j;
  int value0, value1;

  if(AUTOPILOT == 0) {
    for(i = 0; i < maxevents; i = i + 1) {

      for(hexbd = 0; hexbd < MAXHEXBDS; hexbd++) {
        if((hexbd_mask & (1 << hexbd)) != 0) { 

	  // get hexaboards ready
	  res = HEXBD_send_command(hexbd, CMD_RESETPULSE);
	  usleep(HX_DELAY1);// Can be reduced to 1 MuS
	  res = HEXBD_send_command(hexbd, CMD_SETSTARTACQ | 1);
	  usleep(HX_DELAY2);// Can be reduced to 1 MuS
	  res = HEXBD_send_command(hexbd, CMD_SETSTARTACQ);

        }// if hexbd_mask
      }// hexbd loop

      // Send a pulse back to the SYNC board. Give us a trigger.
      old_trig0 = CTL_get_trig_count0();
      CTL_put_done();

      // Wait for trigger.
      trig0 = old_trig0;
      while (trig0 == old_trig0) {
        trig0 = CTL_get_trig_count0();
	//fprintf(stderr,"trig0 = 0x%04x, old_trig0 = 0x%04x\n",
	//(int) trig0, (int)old_trig0);
	//sleep(1);
      }

      if(!(i%10))
	      fprintf(stderr, "Number of Events: %i\n", i);

      for(hexbd = 0; hexbd < MAXHEXBDS; hexbd++) {
        if((hexbd_mask & (1 << hexbd)) != 0) {

	  // tell skirocs to send data back
          res = HEXBD_send_command(hexbd, CMD_STARTCONPUL);
	  usleep(HX_DELAY3);
	  res = HEXBD_send_command(hexbd, CMD_STARTROPUL);
	  usleep(HX_DELAY4);

        }// if hexbd_mask
      }// hexbd loop

      for(hexbd = 0; hexbd < MAXHEXBDS; hexbd++) {
        if((hexbd_mask & (1 << hexbd)) != 0) {

	  // READOUT one hexaboard.
	  res = read_raw_faster(hexbd);

	  // save raw to be converted to 32bit format
	  memcpy(tmp_raw[hexbd], raw, sizeof(raw));
	  
	
	  /*****************************************************/
	  /*         convert raw to readable data             */
	  /*****************************************************/
	  res = decode_raw();
	  
	  /*****************************************************/
	  /* do some verification that data look OK on one chip*/
	  /*****************************************************/
	  chip= 1;
	  for(k = 0; k < 1664; k = k + 1){
	    if((ev[chip][k] & 0x8000 ) == 0){
	      fprintf(stderr,"Wrong MSB at %d %x \n",k,ev[chip][k]);
	      exit(-1);
	    }
	    //if((ev[chip][k] & 0x7E00 ) != 0x0000){
	    //  fprintf(stderr,"Wrong word at %d %d %x\n", i, k,ev[chip][k] );
	    //  exit(-1);
	    //}
	  }
	  if(ev[chip][1923] != 0xc099){
	    fprintf(stderr,"Wrong Trailer is %x \n",ev[chip][1923]);
	    exit(-1);
	  }
	  
	  /*****************************************************/
	  /*           final convert to readable stuff         */
	  /*****************************************************/
	  res = format_channels();
	  
	  /*****************************************************/
	  /*             write event to data file              */
	  /*****************************************************/
	  for(chip = 0; chip < 4; chip = chip + 1){
	    fprintf(fout, "Event %d Chip %d RollMask %x \n",
		    i, chip + 4*hexbd, ev[chip][1920]);
	    for(ch = 0; ch < 128; ch = ch +1){
	      for (sample = 0; sample < 15; sample = sample +1){
		fprintf(fout, "%d  ", data[chip][ch][sample]);
	      }
	      fprintf(fout, "\n");
            }// for ch
          }// for chip
        }// if hexbd_mask
      }// hexbd loop

      int raw_it, layer;
      for(raw_it = 0; raw_it < RAWSIZE; raw_it++) {
		raw_32bit[raw_it] = 0;
	for(layer = 0; layer < MAXHEXBDS; layer++) {
		raw_32bit[raw_it] = ( (tmp_raw[layer][raw_it] & 0xf) << (4*(layer-1)) );
	if(layer == 0 && raw_it < 64 && raw_it %4 == 1) {	
		//if(raw_it == 1) printf("%i",sizeof(raw_32bit[raw_it]));
		if (raw_32bit[raw_it] & 0x8000)
			printf("1");
		else
			printf("0");
		}
	}// layer
      }// raw_it

      raw_32bit[RAWSIZE] = 0x0a0b0c0d;

      if (saveraw) fwrite(raw_32bit, 1, sizeof(raw_32bit), fraw);

    }// event loop
  }// if AUTOPILOT
  else {
printf("Ha! good try\n");
exit(1);
/*
  for(i=0; i<maxevents; i=i+1) {
    
    // FIX ME!!! - this needs work, for more than 1 hexaboard.
    for (hexbd=0; hexbd<1; hexbd++) {
      if ((hexbd_mask & (1<<hexbd)) != 0) {

	if (AUTOPILOT == 0) {
	  res = HEXBD_send_command(hexbd, CMD_RESETPULSE);
	  usleep(HX_DELAY1);// Can be reduced to 1 MuS 
	  res = HEXBD_send_command(hexbd, CMD_SETSTARTACQ | 1);
	  usleep(HX_DELAY2);// Can be reduced to 1 MuS 
	  res = HEXBD_send_command(hexbd, CMD_SETSTARTACQ);

	  // Send a pulse back to the SYNC board. Give us a trigger.
	  old_trig0 = CTL_get_trig_count0();
	  CTL_put_done();
	  
	  // Wait for trigger.
	  trig0 = old_trig0;
	  while (trig0 == old_trig0) {
	    trig0 = CTL_get_trig_count0();
	    //fprintf(stderr,"trig0 = 0x%04x, old_trig0 = 0x%04x\n",
	    //(int) trig0, (int)old_trig0);
	    //sleep(1);
	  }
	  
	  res = HEXBD_send_command(hexbd, CMD_STARTCONPUL);
	  usleep(HX_DELAY3);// Can be reduced to 3 milliseconds 
	  res = HEXBD_send_command(hexbd, CMD_STARTROPUL);
	  usleep(HX_DELAY4);
	  
	  // READOUT one hexaboard.
	  res = read_raw_faster(hexbd);
	  if (saveraw) fwrite(raw, 1, sizeof(raw), fraw);
	
	  //=====================================================
	  //         convert raw to readable data             
	  //====================================================/
	  res = decode_raw();
	  
	  //================================================
	  // do some verification that data look OK on one chip
	  //=======================================================
	  chip= 1;
	  for(k = 0; k < 1664; k = k + 1){
	    if((ev[chip][k] & 0x8000 ) == 0){
	      fprintf(stderr,"Wrong MSB at %d %x \n",k,ev[chip][k]);
	      exit(-1);
	    }
	    //if((ev[chip][k] & 0x7E00 ) != 0x0000){
	    //  fprintf(stderr,"Wrong word at %d %d %x\n", i, k,ev[chip][k] );
	    //  exit(-1);
	    //}
	  }
	  if(ev[chip][1923] != 0xc099){
	    fprintf(stderr,"Wrong Trailer is %x \n",ev[chip][1923]);
	    exit(-1);
	  }
	  
	  //===================================================
	  //           final convert to readable stuff
	  //====================================================
	  res = format_channels();
	  fprintf(stderr,"*");
	  
	  //===================================================
	  //             write event to data file              
	  //=================================================
	  for(chip = 0; chip < 4; chip = chip + 1){
	    fprintf(fout, "Event %d Chip %d RollMask %x \n",
		    i, chip, ev[chip][1920]);
	    for(ch = 0; ch < 128; ch = ch +1){
	      for (sample = 0; sample < 13; sample = sample +1){
		fprintf(fout, "%d  ", data[chip][ch][sample]);
	      }
	      fprintf(fout, "\n");
	    }
	  }
	}
	else { // (AUTOPILOT != 0)

	  //==============================================================
	  // Send a pulse back to the SYNC board. Give us a new trigger.
	  //==============================================================
	  CTL_put_done();
	  
	  //==============================================================
	  // Wait until this skiroc FIFOs have a block of data ready.
	  // Then read it out...
	  //==============================================================
	  for (skiroc=(4*hexbd); skiroc<(4*hexbd)+4; skiroc++) {
	    
	    // Wait.
	    fifo_ready = 0;
	    while (fifo_ready == 0) {
	      block_ready0 = CTL_get_block_ready0();
	      block_ready1 = CTL_get_block_ready1();
	      block_ready = (block_ready1<<16) | block_ready0;
	      fifo_ready = block_ready & (1<<skiroc);
	    }

	    // Get the microsecond counter.
	    usec0 = CTL_get_usec_count0();
	    usec1 = CTL_get_usec_count1();
	    usec2 = CTL_get_usec_count2();
	    fprintf(stdout,"usecond = 0x%04x 0x%04x 0x%04x\n",
		    (int)usec2, (int)usec1, (int)usec0);
	    
	    // Get the trigger counter.
	    trig0 = CTL_get_trig_count0();
	    trig1 = CTL_get_trig_count1();
	    fprintf(stdout,"trigger = 0x%04x 0x%04x\n",
		    (int)trig1, (int)trig0);
	    
	    // Get a block of values from this skiroc fifo.
	    fprintf(stdout,"\n");
	    for (j=0; j<block_size; j++) {
	      value0 = CTL_get_fifo_LS16(skiroc);
	      value1 = CTL_get_fifo_MS16(skiroc);
	      fprintf(stdout,"value = 0x%04x 0x%04x\n",
		      (int)value1, (int)value0);
	    }
	    fprintf(stdout,"\n");
	  } // next skiroc
	} // else AUTOPILOT
      } // end if ((hexbd_mask & (1<<hexbd)) != 0)
    } //next hexaboard
  } // next trigger
*/
  }
  
  fclose(fout);
  fclose(fraw);
  end_spi();
  return(0);    
  
}// Main ends here

