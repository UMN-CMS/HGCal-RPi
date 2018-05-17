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
	printf("FUNCTIONALITY TEST\n\n");


	// Let's read the DIP switches
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	char PAGE[] = {0xD};				// Has PAGE 13
	bcm2835_spi_writenb(PAGE,1);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);

	char PORTB_pullup[] = {0x40, 0xD, 0xF};		// Pulling up on PORTB
	bcm2835_spi_writenb(PORTB_pullup, sizeof(PORTB_pullup));
	char PORTB_read_DIPS[] = {0x41, 0x13, 0};
	char PORTB_DIPS[sizeof(PORTB_read_DIPS)];
	bcm2835_spi_transfernb(PORTB_read_DIPS, PORTB_DIPS, sizeof(PORTB_read_DIPS));
	printf("PORTB DIPS: %02x (%u)\n", PORTB_DIPS[2], PORTB_DIPS[2]);
	printf("\n");


	// Close SPI
	end_spi();
	return 0;
}



void die(const char *s)
{
	printf(s);
	exit(1);
}
