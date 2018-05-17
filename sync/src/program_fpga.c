//
//	Program FPGA
//

#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define PAGES_PER_SECTOR 256
#define BYTES_PER_PAGE 256
#define MAX_BYTES 3011324
#define MAX_TRIALS 5

#define CHIP_ID 2144792 // hex 20 BA 18

#define FPGA_POWER_PAGE 0xF
#define FLASH_PAGE 0x3
#define GPIO_PIN (0x1 - 1)	// first is the physical pin #. The GPIOB number of that pin is always one less.


void die(const char *s);
void init_SPI();
void end_SPI();


int main(int argc, char *argv[]) {

	printf("\n\tFLASH MEMORY WRITE\n\n");


	// Setting up SPI
	init_SPI();



	// Reading chip ID so we know everything is working OK
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);		// CS0
	char PAGE[] = {FLASH_PAGE};
	bcm2835_spi_writenb(PAGE,1);				// Sending PAGE 11
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);		// CS1

	char RDID[] = {0x9E, 0x0, 0x0, 0x0};			// RDID command w/ 3 zero bytes
	char chip_id_data[sizeof(RDID)];
	bcm2835_spi_transfernb(RDID, chip_id_data, sizeof(RDID));// Send RDID and read back into chip_id_data
	unsigned int chipID = (chip_id_data[1]<<16) + (chip_id_data[2]<<8) + chip_id_data[3];// Chip ID in decimal form
	printf("Current Chip ID: %06x\n", chipID);
	if(chipID != CHIP_ID) die("The current chip ID is incorrect!");



	// Time to write to the FPGA
	// Reading file from stdin
	printf("Reading hex file..."); fflush(stdout);
	char *hex_data = malloc(sizeof(char) * MAX_BYTES);	// Using malloc because gcc doesn't like big arrays
	unsigned int val, num_bytes;
	for(num_bytes=0; num_bytes<MAX_BYTES; num_bytes++) {	// Read max number of bytes from file
		if(scanf("%2x", &val) < 1) break;		// Each byte occupies two hex characters
		hex_data[num_bytes] = val;
	}
	if(num_bytes > MAX_BYTES) die("num_bytes >= MAX_BYTES");
	unsigned int num_sectors = (num_bytes >> 16) + 1; 	// Add 1 because the last sector is often only partially filled
	printf(" Finished\n");


	// Erasing sectors
	printf("Erasing sectors..."); fflush(stdout);
	char WEL[] = {0x06};
	char STATUS[] = {0x05, 0};
	char READ_STATUS[sizeof(STATUS)];
	unsigned int sector;
	int trial=1;
	while(1) {
		// Erasing the individual sectors
		for(sector=0;sector<num_sectors;sector++) {		// Erase only the sectors that will be written to to save time
			bcm2835_spi_writenb(WEL, sizeof(WEL));		// Set Write Enable latch

			char ERASE[] = {0xD8, sector, 0x0, 0x0};	// Erase command followed by any address within that sector
			bcm2835_spi_writenb(ERASE, sizeof(ERASE));	// Erase the sector

			while(1) {					// Wait until erase complete
				bcm2835_spi_transfernb(STATUS, READ_STATUS, sizeof(STATUS));
				if(!(READ_STATUS[1] % 2)) break;	// Write in progress is last bit of status
			}
		}

		// Verifying sectors are erased
		int failed=0;								// Flip this if we don't erase right
		char *READ_ERASED = malloc(sizeof(char) * (4 + (num_sectors<<16)));	// Read all bytes from each sector
		READ_ERASED[0] = 0x03;	READ_ERASED[1] = 0x00;				// 0x03 is read command
		READ_ERASED[2] = 0x00;	READ_ERASED[3] = 0x00;				// We want to start reading at address 0x0, 0x0, 0x0
		char dataErased[sizeof(READ_ERASED)];
		bcm2835_spi_transfernb(READ_ERASED, dataErased, sizeof(READ_ERASED));
		int i;
		for(i=4;i<sizeof(READ_ERASED);i++) {					// Make sure every byte is 0xFF
			if(dataErased[i] != 0xFF) {
				printf(" Failed\n");
				if(trial==MAX_TRIALS) die("The flash memory could not be erased");
				else {
					failed=1;
					printf("Erasing sectors..."); fflush(stdout);
				}
			}
		}
		free(READ_ERASED);	// Free memory we got from malloc

		if(!failed) break;
		trial++;
	}
	printf(" Success\n");

	// Writing the config data to the flash memory
	unsigned int page, byte;
	trial=1;
	while(1) {
		printf("Writing to flash memory..."); fflush(stdout);
		for(sector=0;sector<num_sectors;sector++) {
			for(page=0;page<PAGES_PER_SECTOR;page++) {
				bcm2835_spi_writenb(WEL, sizeof(WEL));					// Set write enable
				char WRITE[BYTES_PER_PAGE+4] = {0x02, sector, page, 0x0};		// 0x02 is write command followed by start address
				char toWrite[BYTES_PER_PAGE];
				for(byte=0;byte<BYTES_PER_PAGE;byte++) {
					if((sector<<16) + (page<<8) + byte >= MAX_BYTES) break;		// Don't want to read past the end of the array
					toWrite[byte] = hex_data[(sector<<16) + (page<<8) + byte];	// Getting the hex data
				}
				memcpy(&WRITE[4], toWrite, BYTES_PER_PAGE);				// Combining arrays to write one page at a time
				bcm2835_spi_writenb(WRITE, sizeof(WRITE));				// Write to the page
				while(1) {								// Wait til write is done
					bcm2835_spi_transfernb(STATUS, READ_STATUS, sizeof(STATUS));
					if(!(READ_STATUS[1] % 2)) break;
				}
			}
		}
		printf(" Finished\n");


		// Reading back the flash memory and making sure it's ok
		printf("Verifying write operation..."); fflush(stdout);
		int failed=0;
		unsigned int index;
		for(sector=0;sector<num_sectors;sector++) {
			for(page=0;page<PAGES_PER_SECTOR;page++) {				// Can only read one page at a time
				index = (sector<<16) + (page<<8);
				
				char READ[BYTES_PER_PAGE + 4] = {0x3, sector, page, 0x0};
				char data[sizeof(READ)];
				bcm2835_spi_transfernb(READ, data, sizeof(READ));		// Read one page

				int byte;
				for(byte=0;byte<BYTES_PER_PAGE;byte++) {			// Check if we read back what we tried to write
					if(index + byte < MAX_BYTES) {				// Don't care about what we didn't write to
						if(data[byte+4] != hex_data[index+byte]) {
							if(trial==MAX_TRIALS) die("The memory could not be programmed correctly");
							else {
								failed=1;
								break;
							}
						}
					}
				}
			}
		}
		if(!failed) break;
		else printf(" Failed\n");

		trial++;
	}
	printf(" Success\n");

	// Done with the write
	printf("The flash memory was successfully written.\n");
	free(hex_data);	// Again free what we got from malloc



	// Time to power cycle the FPGA
	printf("Power cycling the FPGA..."); fflush(stdout);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);	// CS0
	PAGE[0] = FPGA_POWER_PAGE;			// PAGE 15
	bcm2835_spi_writenb(PAGE, sizeof(PAGE));
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);	// CS1

	
	// Clearing Pin
	char PIN_SELECT = 0xFF ^ (1 << GPIO_PIN);
	char PIN_as_output[] = {0x40, 0x01, PIN_SELECT};		// Want PIN to be an output
	bcm2835_spi_writenb(PIN_as_output, sizeof(PIN_as_output));
	char PIN_low[] = {0x40, 0x13, PIN_SELECT};			// Set PIN to output low
	bcm2835_spi_writenb(PIN_low, sizeof(PIN_low));
	sleep(5);							// Wait!
	char PIN_as_input[] = {0x40, 0x01, 0xFF};			// Set PIN as input again
	bcm2835_spi_writenb(PIN_as_input, sizeof(PIN_as_input));
	printf(" Finished\n");



	// Waiting until we get a good chip ID
	printf("Checking Chip ID\n");
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);		// CS0
	PAGE[0] = FLASH_PAGE;
	bcm2835_spi_writenb(PAGE,1);				// Sending PAGE 11
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);		// CS1

	int seconds=0;
	const int MAXWAIT = 15;	// Only want to wait 15 seconds
	while(1) {
		bcm2835_spi_transfernb(RDID, chip_id_data, sizeof(RDID));	// Send RDID and read back into chip_id_data
		chipID = (chip_id_data[1]<<16) + (chip_id_data[2]<<8) + chip_id_data[3];// Chip ID in decimal form
		if(chipID ==  CHIP_ID) break;
		else {
			if(seconds == MAXWAIT) die("The Chip ID was not able to be read back in 15 seconds. It is likely that the configuration is bad.");
			else {
				seconds++;
				sleep(1);
			}
		}
	}
	printf("The Chip ID was successfully read back in %i seconds\n", seconds);
	printf("Current Chip ID: %06x\n", chipID);
	printf("The FPGA programming operation was a success!\n");



	// Closing time
	end_SPI();
	return 0;
}



void die(const char *s)
{
	printf("\n");
	printf(s);
	printf("\n");
	end_SPI();
	exit(1);
}



void init_SPI()
{
	if(!bcm2835_init())
	{
		die("bcm2825_init failed. You most likely are not running as root.");
	}

	if(!bcm2835_spi_begin())
	{
		die("bcm2825_spi_begin failed. You most likely are not running as root.");
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
