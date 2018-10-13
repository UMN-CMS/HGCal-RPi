// gcc -O new_rdout.c ejf_rdout.c ctl_orm.c data_orm.c hexbd.c hexbd_config.c spi_common.c -l bcm2835 -o new_rdout.exe

#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "hexbd_config.h"
#include "hexbd.h"
#include "ctl_orm.h"
#include "data_orm.h"
#include "ejf_rdout.h"
#include "spi_common.h"

#define BLOCKSIZE 30000


static int keeprunning = 1;
void handler(int signum) {
    keeprunning = 0;
}


//========================================================================
// MAIN
//========================================================================

int main(int argc, char *argv[])
{
    int res, hx;
    int junk[2000];



    //====================================================================
    // ARGUMENT PROCESSING
    //====================================================================

    int PED = 0;

    if( argc < 2 ){
        printf("usage: sudo ./bin/new_rdout [PED]");
        return(0);
    }

    PED = atoi(argv[1]);



    //====================================================================
    // PRE-RUN SETUP
    //====================================================================

    // Startup the SPI interface on the Pi.
    init_spi();

    // Reset everything.
    DATA_reset_all(0);
    DATA_reset_all(1);
    DATA_reset_all(2);
    DATA_reset_all(3);
    DATA_reset_all(4);
    DATA_reset_all(5);
    DATA_reset_all(6);
    DATA_reset_all(7);
    CTL_reset_all();

    // Get the firmware version.
    int version;
    version = CTL_get_firmware_version();
    fprintf(stderr,"ctl firmware version = 0x%04x\n",(int)version);
    version = DATA_get_firmware_version(0);
    fprintf(stderr,"orm_0 firmware version = 0x%04x\n",(int)version);
    version = DATA_get_firmware_version(1);
    fprintf(stderr,"orm_1 firmware version = 0x%04x\n",(int)version);
    version = DATA_get_firmware_version(2);
    fprintf(stderr,"orm_2 firmware version = 0x%04x\n",(int)version);
    version = DATA_get_firmware_version(3);
    fprintf(stderr,"orm_3 firmware version = 0x%04x\n",(int)version);

    // Test the constant registers, for debug.
    int constant0, constant1;
    constant0 = CTL_get_constant0();
    constant1 = CTL_get_constant1();
    fprintf(stderr,"CTL constant = 0x%04x 0x%04x\n",
            (int)constant1, (int)constant0);

    // Test the dummy registers, for debug.
    int dummy0, dummy1;
    CTL_put_dummy0(0xABCD);
    CTL_put_dummy1(0x1234);
    dummy0 = CTL_get_dummy0();
    dummy1 = CTL_get_dummy1();
    fprintf(stderr,"dummy = 0x%04x 0x%04x\n",
            (int)dummy1, (int)dummy0);

    // Get the default skiroc fifo block_size.
    int block_size;
    block_size = CTL_get_block_size();
    fprintf(stderr,"block_size = %d\n", (int)block_size);

    // Run a test on each of the 8 hexaboards looking for good communication.
    int hexbd_mask;
    hexbd_mask = HEXBD_verify_communication(1);
    fprintf(stderr,"hexbd_mask = 0x%02x\n",(int)hexbd_mask);

    // Set the skiroc mask from the hexbd mask
    int skiroc_mask0, skiroc_mask1;
    skiroc_mask0 = 0;
    for (hx=0; hx<4; hx++) {
        if ((hexbd_mask & (1<<hx)) != 0) skiroc_mask0 |= 0xF << (4*hx);
    }
    skiroc_mask1 = 0;
    for (hx=4; hx<8; hx++) {
        if ((hexbd_mask & (1<<hx)) != 0) skiroc_mask1 |= 0xF << ((4*hx)-16);
    }
    CTL_put_skiroc_mask1(skiroc_mask1);
    CTL_put_skiroc_mask0(skiroc_mask0);

    // Get the skiroc mask.
    skiroc_mask1 = CTL_get_skiroc_mask1();
    skiroc_mask0 = CTL_get_skiroc_mask0();
    fprintf(stderr,"skiroc_mask = 0x%04x 0x%04x\n",
            (int)skiroc_mask1, (int)skiroc_mask0);

    // Configure the active hexaboards here, before enabling the 
    // automatic xfer mechanism (which ignores hexaboard SPI commands).
    char prog_strings[4][48];
    setup_prog_strings(prog_strings);
    char prog_strings_timing[4][48];
    setup_prog_strings(prog_strings_timing[4][48]);
    int chip,byte;
    for(chip = 0; chip < 4; chip++) {
        for(byte = 0; byte < 48; byte++) {
            fprintf(stderr, "%2d %3d 0x%2x\n", prog_strings_timing[chip][byte]);
        }
        fprintf(stderr, "\n");
    }
    int config_status;
    for (hx=0; hx<MAXHEXBDS; hx++) {
        if ((hexbd_mask & (1<<hx)) != 0) {

            // Verify that the command and response queues are empty.
            config_status = HEXBD_queue_status(hx);
            if ((config_status & 0x0101) != 0x0101) {
                fprintf(stderr,"hexbd: %d, queue error\n",(int)hx);
                exit(-1);
            }

            // Configure the hexaboard.
            fprintf(stderr,"Configuring hexbd %d...",(int)hx);
            if(hx == TIMING_HEXBD) {
                config_status = configure_hexaboard_perskiroc(hx, prog_strings_timing, 0);
                config_status = configure_hexaboard_perskiroc(hx, prog_strings_timing, 1);
            } else {
                config_status = configure_hexaboard_perskiroc(hx, prog_strings, 0);
                config_status = configure_hexaboard_perskiroc(hx, prog_strings, 1);
            }
            fprintf(stderr,"done.\n");
            if (config_status < 0) {
                fprintf(stderr,"ERROR in configuration.\n");
                exit(-1);
            }
        }
    }

    // Delay the start of "data taking" post configuration to 
    // stabilize the state of the chip
    fprintf(stderr,"Sleeping...");
    sleep(1);
    fprintf(stderr,"done.\n");



    //===============================================================
    // EVENT LOOP
    //===============================================================

    fprintf(stderr,"\nStart events acquisition\n");

    // reset trigger count
    CTL_reset_trig_count();

    // Agree on the size of the block of data that will set BLOCK_READY flag.
    CTL_put_block_size(BLOCKSIZE);

    // Get the skiroc fifo block_size.
    block_size = CTL_get_block_size();
    fprintf(stderr,"block_size = %d\n", (int)block_size);

    // Get the skiroc mask, for debug...
    skiroc_mask1 = CTL_get_skiroc_mask1();
    skiroc_mask0 = CTL_get_skiroc_mask0();
    fprintf(stderr,"skiroc_mask = 0x%04x 0x%04x\n\n",
            (int)skiroc_mask1, (int)skiroc_mask0);

    uint32_t curr_trig, old_trig;
    uint64_t clock_count, prev_clock_count = 0;

    // reset the fifos for new data
    CTL_reset_fifos();

    // start event loop
    signal(SIGTERM, handler); // handle `kill` commands
    signal(SIGINT, handler); // handle Ctrl-c
    uint32_t count = 0;
    old_trig = CTL_get_trig_count0() | (CTL_get_trig_count1() << 16);
    curr_trig = old_trig;
    while(keeprunning) {

        fprintf(stderr, "%lu hexbd setup\n", count);
        // Get hexaboards ready.
        for(hx = 0; hx < MAXHEXBDS; hx++) {
            if((hexbd_mask & (1 << hx)) != 0) { 
                res = HEXBD_send_command(hx, CMD_RESETPULSE);
            }
        }
        usleep(HX_DELAY1);// Can be reduced to 1 MuS

        // Start acquisition.
        for(hx = 0; hx < MAXHEXBDS; hx++) {
            if((hexbd_mask & (1 << hx)) != 0) { 
                res = HEXBD_send_command(hx, CMD_SETSTARTACQ | 1);
                // usleep(HX_DELAY2);// Can be reduced to 1 MuS
                // res = HEXBD_send_command(hx, CMD_SETSTARTACQ); // this acts as a software trigger - don't use!
            }
        }

        // get the next trigger
        if(PED) {
            // send put trigger to each ORM
            DATA_put_trigger_pulse(0);
            DATA_put_trigger_pulse(1);
            DATA_put_trigger_pulse(2);
            DATA_put_trigger_pulse(3);
        }
        else {
            fprintf(stderr, "%lu send done\n", count);
            // send RDOUT_DONE to get a new trigger
            CTL_put_done();

            // wait for trigger count to increment
            fprintf(stderr, "%lu wait for trig\n", count);
            fprintf(stderr, "%lu    before loop - curr: %7lu old: %7lu\n", count, curr_trig, old_trig);
            while(keeprunning && (curr_trig == old_trig)){
                curr_trig = CTL_get_trig_count0() | (CTL_get_trig_count1() << 16);
                usleep(10); // this ruins timing measurements
            }
            if(curr_trig != old_trig+1) {
                fprintf(stderr, "%lu    bad increment! trig=%lu old=%lu\n", count, curr_trig, old_trig);
                curr_trig = CTL_get_trig_count0() | (CTL_get_trig_count1() << 16);
                fprintf(stderr, "%lu    again: trig=%lu old=%lu\n", count, curr_trig, old_trig);
                if(curr_trig != old_trig+1) {
                    fprintf(stderr, "%lu    exit on bad trig increment: trig=%lu old=%lu\n", count, curr_trig, old_trig);
                    keeprunning = 0;
                }
            }

            fprintf(stderr, "%lu    after loop - curr: %7lu old: %7lu\n", count, curr_trig, old_trig);
            old_trig = curr_trig;

            clock_count = CTL_get_clk_count0() | (CTL_get_clk_count1() << 16) | ((uint64_t)CTL_get_clk_count2() << 32);
            fprintf(stderr, "%lu    clock count: %llu (diff=%llu)\n", count, clock_count, clock_count-prev_clock_count);
            prev_clock_count = clock_count;

        }

        fprintf(stderr, "%lu start rdout\n", count);
        // tell skirocs to start conversion
        for(hx = 0; hx < MAXHEXBDS; hx++) {
            if((hexbd_mask & (1 << hx)) != 0) {
                res = HEXBD_send_command(hx, CMD_STARTCONPUL);
            }
        }
        usleep(HX_DELAY3);

        // tell skirocs to send their data out
        for(hx = 0; hx < MAXHEXBDS; hx++) {
            if((hexbd_mask & (1 << hx)) != 0) {
                res = HEXBD_send_command(hx, CMD_STARTROPUL);
            }
        }
        usleep(HX_DELAY4);

        fprintf(stderr, "%lu wait for fifo to empty\n", count);
        // wait for the FIFO to be empty, indicating IPBus has read it out
        int isFifoEmpty = 0;
        while(keeprunning && !isFifoEmpty){
            isFifoEmpty = CTL_get_empty();
            usleep(10); // this ruins timing measurements
        }

        count++;
    }// event loop



    //===============================================================
    // CLOSING ACTIONS
    //===============================================================

    end_spi();
    return 0;    

}// end main

