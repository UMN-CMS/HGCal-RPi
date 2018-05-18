//---------------------------------------------------------------------
// COMPILATION:
// gcc program_fpga.c -o program_fpga -l bcm2835 -Wall -std=c11
//---------------------------------------------------------------------
// USAGE:
//  sudo ./program_fpga [ORM] < [HEX FILE]
//---------------------------------------------------------------------
//  Writes the hex file supplied through stdin to the flash memory of
//  the ORM specified by the first argument. The ORM is rebooted and
//  this new firmware is loaded in. The chip id is read back and
//  compared to the value in `CHIP_ID` to make sure the programming
//  operation was a success.
//---------------------------------------------------------------------

#include <bcm2835.h>
#include "spi_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define PAGES_PER_SECTOR    256
#define BYTES_PER_PAGE      256
#define MAX_BYTES           3011324     // maximum bytes we can write into memory
#define CHIP_ID             0x20BA18    // default ORM chip id
#define MAX_TRIALS          5           // number of times to try to write/erase memory if it doesn't work
#define MAX_WAIT            15          // how long to wait for the chip id to be read back


int main(int argc, char *argv[]) {

    // argument processing
    if (argc != 2) {
        fprintf(stderr, "usage: sudo ./program_fpga [ORM] < [HEX FILE]\n");
        return -1;
    }


    // setting up spi
    init_spi();


    // reading ORM
    int ORM = atoi(argv[1]);
    char PAGE[1];
    char GPIO_PIN;
    if(ORM == 0){      // ORM0 (DATA)
        GPIO_PIN = 0;
        PAGE[0] = 0x3;
    }
    else if(ORM == 1){ // ORM1 (DATA)
        GPIO_PIN = 1;
        PAGE[0] = 0x5;
    }
    else if(ORM == 2){ // ORM2 (DATA)
        GPIO_PIN = 2;
        PAGE[0] = 0x7;
    }
    else if(ORM == 3){ // ORM3 (DATA)
        GPIO_PIN = 3;
        PAGE[0] = 0x9;
    }
    else if(ORM == 4){ // ORM4 (CTL)
        GPIO_PIN = 4;
        PAGE[0] = 0xb;
    }
    else {
        fprintf(stderr, "ORM must be between 0 and 4\nDATA ORMs are 0-3, CTL/SYNC is 4\n");
        return -1;
    }


    printf("\n\tFLASH MEMORY WRITE\n\n");


    // sending PAGE to talk to the correct ORM
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);		// CS0 -- talk to decoder
    bcm2835_spi_writenb(PAGE,1);				    // Sending PAGE
    bcm2835_spi_chipSelect(BCM2835_SPI_CS1);		// CS1 -- talk to ORM


    // reading chip id so we know everything is working OK
    char RDID[] = {0x9E, 0x0, 0x0, 0x0}; // RDID command w/ 3 zero bytes
    char chip_id_data[sizeof(RDID)];
    bcm2835_spi_transfernb(RDID, chip_id_data, sizeof(RDID));
    unsigned int chipID = (chip_id_data[1]<<16) + (chip_id_data[2]<<8) + chip_id_data[3];
    printf("Current Chip ID: %06x\n", chipID);
    if(chipID != CHIP_ID) {
        fprintf(stderr, "The current chip ID is incorrect!\n");   
        return -1;
    }



    // reading hex file from stdin
    printf("Reading hex file..."); fflush(stdout);
    char *hex_data = malloc(sizeof(char) * MAX_BYTES);
    unsigned int val, num_bytes;
    for(num_bytes=0; num_bytes<MAX_BYTES; num_bytes++) {	// Read max number of bytes from file
        if(scanf("%2x", &val) < 1) break;		            // Each byte occupies two hex characters
        hex_data[num_bytes] = val;
    }
    if(num_bytes > MAX_BYTES) {
        fprintf(stderr, "num_bytes in hex file >= MAX_BYTES\n");
        return -1;
    }
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

            while(1) {                                  // Wait until erase complete
                bcm2835_spi_transfernb(STATUS, READ_STATUS, sizeof(STATUS));
                if(!(READ_STATUS[1] % 2)) break;        // Write in progress is last bit of status
            }

        }

        // Verifying sectors are erased
        int failed=0;								                        // Flip this if we don't erase right
        char *READ_ERASED = malloc(sizeof(char) * (4 + (num_sectors<<16)));	// Read all bytes from each sector
        READ_ERASED[0] = 0x03;	READ_ERASED[1] = 0x00;				        // 0x03 is read command
        READ_ERASED[2] = 0x00;	READ_ERASED[3] = 0x00;				        // We want to start reading at address 0x0, 0x0, 0x0
        char dataErased[sizeof(READ_ERASED)];
        bcm2835_spi_transfernb(READ_ERASED, dataErased, sizeof(READ_ERASED));
        int i;
        for(i=4;i<sizeof(READ_ERASED);i++) {					            // Make sure every byte is 0xFF (erased)
            if(dataErased[i] != 0xFF) {
                printf(" Failed\n");
                if(trial==MAX_TRIALS) {
                    fprintf(stderr, "The flash memory could not be erased\n");
                    return -1;
                }
                else {
                    failed=1;
                    printf("Erasing sectors..."); fflush(stdout);
                }
            }
        }
        free(READ_ERASED); // Free memory we got from malloc

        if(!failed) break;
        trial++;
    }
    printf(" Success\n");


    // write and verify
    unsigned int page, byte;
    trial=1;
    while(1) {

        // Writing the config data to the flash memory
        printf("Writing to flash memory..."); fflush(stdout);
        for(sector=0;sector<num_sectors;sector++) {
            for(page=0;page<PAGES_PER_SECTOR;page++) {
                bcm2835_spi_writenb(WEL, sizeof(WEL));					        // Set write enable
                char WRITE[BYTES_PER_PAGE+4] = {0x02, sector, page, 0x0};		// 0x02 is write command followed by start address
                char toWrite[BYTES_PER_PAGE];
                for(byte=0;byte<BYTES_PER_PAGE;byte++) {
                    if((sector<<16) + (page<<8) + byte >= MAX_BYTES) break;		// Don't want to read past the end of the array
                    toWrite[byte] = hex_data[(sector<<16) + (page<<8) + byte];	// Getting the hex data
                }
                memcpy(&WRITE[4], toWrite, BYTES_PER_PAGE);				        // Combining arrays to write one page at a time
                bcm2835_spi_writenb(WRITE, sizeof(WRITE));				        // Write to the page
                while(1) {								                        // Wait til write is done
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
            for(page=0;page<PAGES_PER_SECTOR;page++) {				        // Can only read one page at a time
                index = (sector<<16) + (page<<8);

                char READ[BYTES_PER_PAGE + 4] = {0x3, sector, page, 0x0};
                char data[sizeof(READ)];
                bcm2835_spi_transfernb(READ, data, sizeof(READ));		    // Read one page

                int byte;
                for(byte=0;byte<BYTES_PER_PAGE;byte++) {			        // Check if we read back what we tried to write
                    if(index + byte < MAX_BYTES) {				            // Don't care about what we didn't write to
                        if(data[byte+4] != hex_data[index+byte]) {
                            if(trial==MAX_TRIALS) {
                                fprintf(stderr, "The memory could not be programmed correctly\n");
                                return -1;
                            }
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
    printf("Power cycling ORM%d...",ORM); fflush(stdout);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);     // CS0

    char orm_page = PAGE[0];
    PAGE[0] = 0xF;
    bcm2835_spi_writenb(PAGE, sizeof(PAGE));
    bcm2835_spi_chipSelect(BCM2835_SPI_CS1);    // CS1

    char PIN_SELECT = 0xFF ^ (1 << GPIO_PIN);
    char PIN_as_output[] = {0x40, 0x01, PIN_SELECT};	// Want PIN to be an output
    bcm2835_spi_writenb(PIN_as_output, sizeof(PIN_as_output)+1);
    char PIN_low[] = {0x40, 0x13, PIN_SELECT};		// Set PIN to output low
    bcm2835_spi_writenb(PIN_low, sizeof(PIN_low)+1);
    sleep(5);						// Wait!
    char PIN_as_input[] = {0x40, 0x01, 0xFF};		// Set PIN as input again
    bcm2835_spi_writenb(PIN_as_input, sizeof(PIN_as_input)+1);
    printf(" Finished\n");


    // Waiting until we get a good chip ID
    printf("Checking chip ID..."); fflush(stdout);
    PAGE[0] = orm_page;
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);		// CS0
    bcm2835_spi_writenb(PAGE,1);				    // Sending PAGE
    bcm2835_spi_chipSelect(BCM2835_SPI_CS1);		// CS1

    int seconds=0;
    while(1) {
        bcm2835_spi_transfernb(RDID, chip_id_data, sizeof(RDID));	// Send RDID and read back into chip_id_data
        chipID = (chip_id_data[1]<<16) + (chip_id_data[2]<<8) + chip_id_data[3];// Chip ID in decimal form
        if(chipID ==  CHIP_ID) break;
        else {
            if(seconds >= MAX_WAIT) {
                fprintf(stderr, "The Chip ID was not able to be read back in 15 seconds.\nIt is likely that the configuration is bad.\n");
                return -1;
            }
            else {
                seconds++;
                sleep(1);
            }
        }
    }
    printf(" Done.\n");
    printf("The FPGA programming operation was a success!\n");


    // Closing time
    end_spi();
    return 0;
}
