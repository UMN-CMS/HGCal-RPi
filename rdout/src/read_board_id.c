//
//	Synch Board Functionality Test
//

#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi_common.h"


void die(const char *s);


int main()
{
	// Setting up SPI
	init_spi();

	// Let's read the DIP switches
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	char PAGE[] = {0xD};				// Chip has PAGE 13
	bcm2835_spi_writenb(PAGE,1);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);

	char PORTA_pullup[] = {0x40, 0xC, 0xFF};	// Pulling up on PORTA
	bcm2835_spi_writenb(PORTA_pullup, sizeof(PORTA_pullup));

	char PORTA_read_DIPS[] = {0x41, 0x12, 0};	// Reading DIP switches
	char PORTA_DIPS[sizeof(PORTA_read_DIPS)];
	bcm2835_spi_transfernb(PORTA_read_DIPS, PORTA_DIPS, sizeof(PORTA_read_DIPS));
	printf("%u\n", PORTA_DIPS[2]);

	// Close SPI
	end_spi();
	return 0;
}



void die(const char *s)
{
	printf(s);
	exit(1);
}
