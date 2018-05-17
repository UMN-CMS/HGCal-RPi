//
//	Synch Board Functionality Test
//

#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>


void die(const char *s);
void init_SPI();
void end_SPI();


int main()
{
	// Setting up SPI
	init_SPI();
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
	end_SPI();
	return 0;
}



void die(const char *s)
{
	printf(s);
	exit(1);
}



void init_SPI()
{
	if(!bcm2835_init())
	{
		die("bcm2825_init failed. You most likely are not running as root.\n");
	}

	if(!bcm2835_spi_begin())
	{
		die("bcm2825_spi_begin failed. You most likely are not running as root.\n");
	}

	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);	// 1.5625 MHz
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);			// Chip-Select 0
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);	// Value of CS when active
}



void end_SPI()
{
	bcm2835_spi_end();
	bcm2835_close();
}
