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
