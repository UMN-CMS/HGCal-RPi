#include "ejf_rdout.h"
#include "ctl_orm.h"
#include "hexbd.h"

int power_cycle_just_fpgas()
{
  char PAGE[1];
  char GPIO_PIN = 6;
  
  // Time to power cycle the FPGAs
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);	// CS0
  
  PAGE[0] = 0xF | 0xF<<4;
  bcm2835_spi_writenb(PAGE, sizeof(PAGE));
  bcm2835_spi_chipSelect(BCM2835_SPI_CS1);	// CS1
  
  // Clearing Pin
  char PIN_SELECT = 0xFF ^ (1 << GPIO_PIN);
  char PIN_as_output[] = {0x40, 0x01, PIN_SELECT};	// Want PIN to be an output
  bcm2835_spi_writenb(PIN_as_output, sizeof(PIN_as_output)+1);

  char PIN_high[] = {0x40, 0x13, (1 << GPIO_PIN)};	// Set PIN to output high
  bcm2835_spi_writenb(PIN_high, sizeof(PIN_high)+1);
  sleep(5);						// Wait!

  char PIN_low[] = {0x40, 0x13, PIN_SELECT};		// Set PIN to output low
  bcm2835_spi_writenb(PIN_low, sizeof(PIN_high)+1);
  sleep(15);						// Wait!

  char PIN_as_input[] = {0x40, 0x01, 0xFF};		// Set PIN as input again
  bcm2835_spi_writenb(PIN_as_input, sizeof(PIN_as_input)+1);
  
  return 0;
}


int power_cycle(int orm)
{
  char PAGE[1];
  char GPIO_PIN;
  if (orm == 0) GPIO_PIN = 0;
  else if (orm == 1) GPIO_PIN = 1;
  else if (orm == 2) GPIO_PIN = 2;
  else if (orm == 3) GPIO_PIN = 3;
  else if (orm == 4) GPIO_PIN = 4;
  
  // Time to power cycle the FPGA
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);	// CS0
  
  PAGE[0] = 0xF | 0xF<<4;
  bcm2835_spi_writenb(PAGE, sizeof(PAGE));
  bcm2835_spi_chipSelect(BCM2835_SPI_CS1);	// CS1
  
  // Clearing Pin
  char PIN_SELECT = 0xFF ^ (1 << GPIO_PIN);
  char PIN_as_output[] = {0x40, 0x01, PIN_SELECT};	// Want PIN to be an output
  bcm2835_spi_writenb(PIN_as_output, sizeof(PIN_as_output)+1);
  char PIN_low[] = {0x40, 0x13, PIN_SELECT};	// Set PIN to output low
  bcm2835_spi_writenb(PIN_low, sizeof(PIN_low)+1);
  sleep(5);						// Wait!
  char PIN_as_input[] = {0x40, 0x01, 0xFF};		// Set PIN as input again
  bcm2835_spi_writenb(PIN_as_input, sizeof(PIN_as_input)+1);
  
  return 0;
}

int read_raw(int hexbd)
{
  int i, t;
  
  for (i = 0; i < RAWSIZE; i = i+1){
    t = HEXBD_read_local_fifo(hexbd);
    raw[i] = (unsigned char) (t & 255);
  }
  
  if (CRCSIZE > 0) {
    t = HEXBD_read_local_fifo(hexbd); // FF junk
    for (i=0; i<CRCSIZE; i=i+1) {
      t = HEXBD_read_local_fifo(hexbd);
      crc[i] = (unsigned char) (t & 255);
    }
  }
}

int read_raw_faster(int hexbd) 
{
  int i, j, num_done, status, t, num_leftovers;
  int result[1000];
  
  num_done=0;
  
  for (i=0; i<30; i=i+1) { // Get 30000 values
    status = HEXBD_read1000_local_fifo(hexbd, &(result[0]));
    for (j=0; j<1000; j++) raw[num_done++] = (unsigned char) (result[j] & 255);
  }
  
  for (i=0; i<3; i=i+1) { // Get 600 values.
    status = HEXBD_read200_local_fifo(hexbd, &(result[0]));
    for (j=0; j<200; j++) raw[num_done++] = (unsigned char) (result[j] & 255);
  }
  
  // Clean up the last stragglers.
  num_leftovers = RAWSIZE - num_done;
  for (i=0; i<num_leftovers; i=i+1) {
    t = HEXBD_read_local_fifo(hexbd);
    raw[num_done++] = (unsigned char) (t & 255);
  }
  
  if (CRCSIZE > 0) {
    t = HEXBD_read_local_fifo(hexbd); // FF junk
    for (i=0; i<CRCSIZE; i=i+1) {
      t = HEXBD_read_local_fifo(hexbd);
      crc[i] = (unsigned char) (t & 255);
    }
  }
  
  //for (i=0; i<CRCSIZE; i=i+1) fprintf(stderr,"CRC: %0x\n",(int)crc[i]);
  
  return(0);
}

int decode_raw()
{
  int i, j, k;
  unsigned char x;
  unsigned int t;
  unsigned int bith, bit11, bit10, bit9, bit8, bit7, bit6, bit5, bit4, bit3, bit2, bit1, bit0;
  
  for(i = 0; i < 1924; i = i+1){
    for (k = 0; k < 4; k = k + 1){
      ev[k][i] = 0;
    }
  }
  
  for(i = 0; i < 1924; i = i+1){
    for (j = 0; j < 16; j = j+1){
      x = raw[1 + i*16 + j];
      x = x & 15;
      for (k = 0; k < 4; k = k + 1){
	ev[k][i] = ev[k][i] | (unsigned int) (((x >> (3 - k) ) & 1) << (15 - j));
      }
    }
  }
  
  /*****************************************************/
  /*    Gray to binary conversion                      */
  /*****************************************************/
  for(k = 0; k < 4 ; k = k +1 ){
    for(i = 0; i < 128*15; i = i + 1){
      bith = ev[k][i] & 0x8000;
      t = ev[k][i] & 0x7fff;
      bit11 = (t >> 11) & 1;
      bit10 = bit11 ^ ((t >>10) &1);
      bit9 = bit10 ^ ((t >>9) &1);
      bit8 = bit9 ^ ((t >>8) &1);
      bit7 = bit8 ^ ((t >>7) &1);
      bit6 = bit7 ^ ((t >>6) &1);
      bit5 = bit6 ^ ((t >>5) &1);
      bit4 = bit5 ^ ((t >>4) &1);
      bit3 = bit4 ^ ((t >>3) &1);
      bit2 = bit3 ^ ((t >>2) &1);
      bit1 = bit2 ^ ((t >>1) &1);
      bit0 = bit1 ^ ((t >>0) &1);
      ev[k][i] =  bith | ((bit11 << 11) + (bit10 << 10) + (bit9 << 9) + (bit8 << 8) + (bit7 << 7) + (bit6 << 6) + (bit5 << 5) + (bit4 << 4) + (bit3  << 3) + (bit2 << 2) + (bit1  << 1) + bit0);
        	}
  }// IMPLEMENT GRAY_TO_BINARY convertion as a separate routine
  
}// The function decode_raw() ends here

int format_channels()
{  
  int chip, ch, hit; 
  for (chip = 0; chip < 4; chip = chip +1 ){
    for (ch = 0; ch < 128; ch = ch +1 ){
      for (hit = 0 ; hit <15 ; hit = hit +1){
	data[chip][ch][hit] = ev[chip][hit*128+ch] & 0x0FFF;
      }
    }
  }
  return(0);
}
