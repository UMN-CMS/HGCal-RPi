//
//	Synch Board Read FLASH Memory
//

#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>

#define BYTES 256


void die(const char *s);
void init_SPI();
void end_SPI();


int main()
{
	// Setting up SPI
	init_SPI();
	printf("FLASH MEMORY READ\n\n");


	// Let's read the DIP switches
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);	// CS0
	char PAGE[] = {0xB};
	bcm2835_spi_writenb(PAGE,1);			// Sending PAGE 11
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);	// CS1

	int sector;
	for(sector=0;sector<BYTES;sector++)
	{
		int page;
		for(page=0;page<BYTES;page++)
		{
			printf("Sector %u Page %u\n", sector, page);
			char READ[BYTES+4] = {0x3, sector, page, 0x0};
			char data[sizeof(READ)];
			bcm2835_spi_transfernb(READ, data, sizeof(READ));// Send READ and read back into data


			int addr;
			for(addr=0;addr<BYTES;addr++) if(data[addr+4]) printf("%02x\n",data[addr+4]);
			printf("\n");
		}
	}


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
