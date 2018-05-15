// gcc -g -c sync_orm.c

#include "sync_orm.h"

int SYNC_reset_all(void)
{ // { 12'h0, reset_waiting, reset_rdout, reset_trig_counter, reset_all}
  return(spi_put_16bits(4, SYNC_RESET_PULSE, 1<<0)); 
}

int SYNC_reset_trig_counter(void)
{ // { 12'h0, reset_waiting, reset_rdout, reset_trig_counter, reset_all}
  return(spi_put_16bits(4, SYNC_RESET_PULSE, 1<<1)); 
}

int SYNC_reset_rdout(void)
{ // { 12'h0, reset_waiting, reset_rdout, reset_trig_counter, reset_all}
  return(spi_put_16bits(4, SYNC_RESET_PULSE, 1<<2)); 
}

int SYNC_reset_waiting(void)
{ // { 12'h0, reset_waiting, reset_rdout, reset_trig_counter, reset_all}
  return(spi_put_16bits(4, SYNC_RESET_PULSE, 1<<3)); 
}

int SYNC_get_firmware_version(void)
{ return(spi_get_16bits(4, SYNC_FIRMWARE_VERSION)); }

int SYNC_get_constant0(void)
{ return(spi_get_16bits(4, SYNC_CONSTANT0)); }

int SYNC_get_constant1(void)
{ return(spi_get_16bits(4, SYNC_CONSTANT1)); }

int SYNC_get_dummy0(void)
{ return(spi_get_16bits(4, SYNC_DUMMY_REG0)); }

int SYNC_put_dummy0(int value16bits)
{ return(spi_put_16bits(4, SYNC_DUMMY_REG0, value16bits)); }

int SYNC_get_dummy1(void)
{ return(spi_get_16bits(4, SYNC_DUMMY_REG1)); }

int SYNC_put_dummy1(int value16bits)
{ return(spi_put_16bits(4, SYNC_DUMMY_REG1, value16bits)); }

int SYNC_get_enable_trig(void)
{ return(spi_get_16bits(4, SYNC_ENABLE_TRIG)); }

int SYNC_put_enable_trig(int value1bit)
{ return(spi_put_16bits(4, SYNC_ENABLE_TRIG, value1bit)); }

int SYNC_get_sync_mode(void)
{ return(spi_get_16bits(4, SYNC_MODE)); }

int SYNC_put_sync_mode(int value1bit)
{ return(spi_put_16bits(4, SYNC_MODE, value1bit)); }

int SYNC_get_rdout_mask(void)
{ return(spi_get_16bits(4, SYNC_RDOUT_MASK)); }

int SYNC_put_rdout_mask(int value16bits)
{ return(spi_put_16bits(4, SYNC_RDOUT_MASK, value16bits)); }

int SYNC_get_waiting_status(void)
{ return(spi_get_16bits(4, SYNC_WAITING_STATUS)); }

int SYNC_get_rdout_done_hold(void)
{ return(spi_get_16bits(4, SYNC_RDOUT_DONE_HOLD)); }

int SYNC_get_delay_done(void)
{ return(spi_get_16bits(4, SYNC_DELAY_DONE)); }

int SYNC_put_delay_done(int value16bits)
{ return(spi_put_16bits(4, SYNC_DELAY_DONE, value16bits)); }

int SYNC_get_enable_veto(void)
{ return(spi_get_16bits(4, SYNC_ENABLE_VETO)); }

int SYNC_put_enable_veto(int value2bits)
{ return(spi_put_16bits(4, SYNC_ENABLE_VETO, value2bits)); }

int SYNC_get_trig_count0(void)
{ return(spi_get_16bits(4, SYNC_TRIG_COUNT0)); }

int SYNC_get_trig_count1(void)
{ return(spi_get_16bits(4, SYNC_TRIG_COUNT1)); }

int SYNC_get_max_trig_count0(void)
{ return(spi_get_16bits(4, SYNC_MAX_TRIG_COUNT0)); }

int SYNC_put_max_trig_count0(int value16bits)
{ return(spi_put_16bits(4, SYNC_MAX_TRIG_COUNT0, value16bits)); }

int SYNC_get_max_trig_count1(void)
{ return(spi_get_16bits(4, SYNC_MAX_TRIG_COUNT1)); }

int SYNC_put_max_trig_count1(int value16bits)
{ return(spi_put_16bits(4, SYNC_MAX_TRIG_COUNT1, value16bits)); }

int SYNC_get_enable_fake25(void)
{ return(spi_get_16bits(4, SYNC_ENABLE_FAKE25)); }

int SYNC_put_enable_fake25(int value1bit)
{ return(spi_put_16bits(4, SYNC_ENABLE_FAKE25, value1bit)); }

int SYNC_get_thresh_fake25(void)
{ return(spi_get_16bits(4, SYNC_THRESH_FAKE25)); }

int SYNC_put_thresh_fake25(int value16bits)
{ return(spi_put_16bits(4, SYNC_THRESH_FAKE25, value16bits)); }

int SYNC_get_max0_fake25(void)
{ return(spi_get_16bits(4, SYNC_MAX0_FAKE25)); }

int SYNC_put_max0_fake25(int value16bits)
{ return(spi_put_16bits(4, SYNC_MAX0_FAKE25, value16bits)); }

int SYNC_get_max1_fake25(void)
{ return(spi_get_16bits(4, SYNC_MAX1_FAKE25)); }

int SYNC_put_max1_fake25(int value16bits)
{ return(spi_put_16bits(4, SYNC_MAX1_FAKE25, value16bits)); }

int SYNC_get_enable_fake40(void)
{ return(spi_get_16bits(4, SYNC_ENABLE_FAKE40)); }

int SYNC_put_enable_fake40(int value1bit)
{ return(spi_put_16bits(4, SYNC_ENABLE_FAKE40, value1bit)); }

int SYNC_get_thresh_fake40(void)
{ return(spi_get_16bits(4, SYNC_THRESH_FAKE40)); }

int SYNC_put_thresh_fake40(int value16bits)
{ return(spi_put_16bits(4, SYNC_THRESH_FAKE40, value16bits)); }

int SYNC_get_max0_fake40(void)
{ return(spi_get_16bits(4, SYNC_MAX0_FAKE40)); }

int SYNC_put_max0_fake40(int value16bits)
{ return(spi_put_16bits(4, SYNC_MAX0_FAKE40, value16bits)); }

int SYNC_get_max1_fake40(void)
{ return(spi_get_16bits(4, SYNC_MAX1_FAKE40)); }

int SYNC_put_max1_fake40(int value16bits)
{ return(spi_put_16bits(4, SYNC_MAX1_FAKE40, value16bits)); }

int SYNC_get_enable_fake65(void)
{ return(spi_get_16bits(4, SYNC_ENABLE_FAKE65)); }

int SYNC_put_enable_fake65(int value1bit)
{ return(spi_put_16bits(4, SYNC_ENABLE_FAKE65, value1bit)); }

int SYNC_get_thresh_fake65(void)
{ return(spi_get_16bits(4, SYNC_THRESH_FAKE65)); }

int SYNC_put_thresh_fake65(int value16bits)
{ return(spi_put_16bits(4, SYNC_THRESH_FAKE65, value16bits)); }

int SYNC_get_max0_fake65(void)
{ return(spi_get_16bits(4, SYNC_MAX0_FAKE65)); }

int SYNC_put_max0_fake65(int value16bits)
{ return(spi_put_16bits(4, SYNC_MAX0_FAKE65, value16bits)); }

int SYNC_get_max1_fake65(void)
{ return(spi_get_16bits(4, SYNC_MAX1_FAKE65)); }

int SYNC_put_max1_fake65(int value16bits)
{ return(spi_put_16bits(4, SYNC_MAX1_FAKE65, value16bits)); }

int SYNC_cables_connected(void)
{
  char PAGE[1];
  
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  PAGE[0] = 0x9;				// Chip has PAGE 13
  bcm2835_spi_writenb(PAGE,1);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
  
  char PORTA_pullup[] = {0x40, 0xC, 0xFF};	// Pulling up on PORTA
  char PORTB_pullup[] = {0x40, 0xD, 0xFF};	// Pulling up on PORTB
  bcm2835_spi_writenb(PORTA_pullup, sizeof(PORTA_pullup));
  bcm2835_spi_writenb(PORTB_pullup, sizeof(PORTB_pullup));
  
  char PORTA_read_DIPS[] = {0x41, 0x12, 0};	// Reading DIP switches
  char PORTA_DIPS[sizeof(PORTA_read_DIPS)];
  bcm2835_spi_transfernb(PORTA_read_DIPS, PORTA_DIPS, sizeof(PORTA_read_DIPS));
  //printf("PORT A: %2x (%i)\n", PORTA_DIPS[2], PORTA_DIPS[2]);
  
  char PORTB_read_DIPS[] = {0x41, 0x13, 0};	// Reading DIP switches
  char PORTB_DIPS[sizeof(PORTB_read_DIPS)];
  bcm2835_spi_transfernb(PORTB_read_DIPS, PORTB_DIPS, sizeof(PORTB_read_DIPS));
  //printf("PORT B: %2x (%i)\n", PORTB_DIPS[2], PORTB_DIPS[2]);
  
  int CONN_LIST = (PORTB_DIPS[2] << 8) + PORTA_DIPS[2]; // 16 bits
  CONN_LIST ^= 0xFFFF; // List the cables that are connected.
  return(CONN_LIST);
}
