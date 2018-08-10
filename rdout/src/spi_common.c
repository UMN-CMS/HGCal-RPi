#include <bcm2835.h>
#include "spi_common.h"

#include <stdlib.h>
#include <stdio.h>


#define CHIP_ID 0x20ba18


// set page to talk to an ORM's EEPROM
void spi_select_eeprom(int orm) {
    char page[1] = { 1 + (2 * (orm + 1)) };
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
    bcm2835_spi_writenb(page, 1);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
}


// read the chip ID from an ORM's EEPROM
int read_chip_id(int orm) {

    // send page to talk to the EEPROM
    spi_select_eeprom(orm);

    // send command to get the EEPROM's chip ID
    char read_id[] = {0x9e, 0x00, 0x00, 0x00};
    char response[4];
    bcm2835_spi_transfernb(read_id, response, 4);
    int chip_id = (response[1]<<16) | (response[2]<<8) | response[3];
    return chip_id;
}


// Get the board ID from the dip switches
int get_board_id() {

	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	char PAGE[] = {0xD};				// Chip has PAGE 13
	bcm2835_spi_writenb(PAGE,1);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);

	char PORTA_pullup[] = {0x40, 0xC, 0xFF};	// Pulling up on PORTA
	bcm2835_spi_writenb(PORTA_pullup, sizeof(PORTA_pullup));

	char PORTA_read_DIPS[] = {0x41, 0x12, 0};	// Reading DIP switches
	char PORTA_DIPS[sizeof(PORTA_read_DIPS)];
	bcm2835_spi_transfernb(PORTA_read_DIPS, PORTA_DIPS, sizeof(PORTA_read_DIPS));
    
    return PORTA_DIPS[2];
}


// Set the page register so we can talk to an ORM.
void spi_select_orm(int orm) {
    char page[1];
    page[0] = 2 * (orm + 1);

    // Select the appropriate oRM.
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
    bcm2835_spi_writenb(page,1);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
}


int spi_get_16bits(int orm, int addr) {
    char cmd[6], data[6];
    unsigned spi_read, spi_auto_inc, spi_addr, spi_command;
    int result1, result2, error;

    // Select the appropriate oRM.
    spi_select_orm(orm);

    // Create the 32-bit command word.
    spi_read = 1;
    spi_auto_inc = 0;
    spi_addr = addr & 0x3FF;
    spi_command = (spi_read<<31) | (spi_auto_inc<<30) | (spi_addr<<20);
    cmd[0] = spi_command >> 24;
    cmd[1] = spi_command >> 16;
    cmd[2] = spi_command >> 8;
    cmd[3] = spi_command >> 0;

    // Send the command.
    error = 1;
    while(error) {
        bcm2835_spi_transfernb(cmd, data, 6);
        result1 = (data[2]<<8) | data[3];
        result2 = (data[4]<<8) | data[5];

        if((spi_auto_inc != 0) || (result1 == result2))
            error = 0;
    }

    return result1;
}

// Send a 32-bit spi_read command, and keep the 16 bits that are returned.
int spi_get_16bits_fifo(int orm, int addr) {
    char cmd[4], data[4];
    unsigned spi_read, spi_auto_inc, spi_addr, spi_command;

    // Select the appropriate oRM.
    spi_select_orm(orm);

    // Create the 32-bit command word.
    spi_read = 1;
    spi_auto_inc = 0;
    spi_addr = addr & 0x3FF;
    spi_command = (spi_read<<31) | (spi_auto_inc<<30) | (spi_addr<<20);
    cmd[0] = spi_command >> 24;
    cmd[1] = spi_command >> 16;
    cmd[2] = spi_command >> 8;
    cmd[3] = spi_command >> 0;

    // Send the command.
    bcm2835_spi_transfernb(cmd, data, 4);
    return (data[2]<<8) | data[3];
}


// Send a 32-bit spi_write command, which writes 16 bits into the address.
int spi_put_16bits(int orm, int addr, int value) { 
    char cmd[4], data[4];
    unsigned spi_read, spi_auto_inc, spi_addr, spi_command;

    // Select the appropriate oRM.
    spi_select_orm(orm);

    // Create the 32-bit command word.
    spi_read = 0;
    spi_auto_inc = 0;
    spi_addr = addr & 0x3FF;
    spi_command = (spi_read<<31) | (spi_auto_inc<<30)
                | (spi_addr<<20) | (value & 0xFFFF);
    cmd[0] = spi_command >> 24;
    cmd[1] = spi_command >> 16;
    cmd[2] = spi_command >> 8;
    cmd[3] = spi_command >> 0;

    // Send the command.
    bcm2835_spi_transfernb(cmd, data, 4);
    return 0;
}

// Initialize the SPI interface.
void init_spi() {
    if(!bcm2835_init())
    {
        printf("bcm2825_init failed. You most likely are not running as root.\n");
        exit(1);
    }

    if(!bcm2835_spi_begin())
    {
        printf("bcm2825_spi_begin failed. You most likely are not running as root.\n");
        exit(1);
    }

    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);  // 6 MHz
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);			        // Chip-Select 0
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);	// Value of CS when active
}


// Close the SPI interface.
void end_spi() {
    bcm2835_spi_end();
    bcm2835_close();
}
