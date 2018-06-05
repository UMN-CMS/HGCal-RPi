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
    if(signum == SIGTERM) {
        keeprunning = 0;
    }
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

    // Set the date stamp to zero.
    int date_stamp0, date_stamp1;
    CTL_put_date_stamp0(0); // To be used as Trigger_Send_OK
    CTL_put_date_stamp1(0);
    date_stamp0 =  CTL_get_date_stamp0();
    date_stamp1 =  CTL_get_date_stamp1();
    fprintf(stderr,"date_stamp = 0x%04x 0x%04x\n",
            (int)date_stamp1, (int)date_stamp0);

    // Reset everything. twice...
    DATA_reset_all(0);
    DATA_reset_all(1);
    DATA_reset_all(2);
    DATA_reset_all(3);
    DATA_reset_all(4);
    DATA_reset_all(5);
    DATA_reset_all(6);
    DATA_reset_all(7);
    CTL_reset_all();
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

    // empty local fifo by forcing extra reads, ignore results
    fprintf(stderr,"emptying local fifos (partially)...");
    for (hx=0; hx<MAXHEXBDS; hx++) HEXBD_read1000_local_fifo(hx,junk);
    for (hx=0; hx<MAXHEXBDS; hx++) HEXBD_read1000_local_fifo(hx,junk);
    for (hx=0; hx<MAXHEXBDS; hx++) HEXBD_read1000_local_fifo(hx,junk);
    fprintf(stderr,"done.\n");

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
            config_status = configure_hexaboard(hx,0);
            config_status = configure_hexaboard(hx,1);
            fprintf(stderr,"done.\n");
            if (config_status < 0) {
                fprintf(stderr,"ERROR in configuration.\n");
                exit(-1);
            }
        }
    }

    // empty local fifo by forcing extra reads, ignore results
    fprintf(stderr,"Emptying local fifos (partially)...");
    for (hx=0; hx<8; hx++) HEXBD_read1000_local_fifo(hx,junk);
    for (hx=0; hx<8; hx++) HEXBD_read1000_local_fifo(hx,junk);
    for (hx=0; hx<8; hx++) HEXBD_read1000_local_fifo(hx,junk);
    fprintf(stderr,"done.\n");

    // Delay the start of "data taking" post configuration to 
    // stabilize the state of the chip
    fprintf(stderr,"Sleeping...");
    usleep(10000);
    sleep(1);
    fprintf(stderr,"done.\n");

    // Set the date stamp to non-zero, indicating we are done initializing.
    CTL_put_date_stamp0(0xAABB);
    CTL_put_date_stamp1(0x3434);
    date_stamp0 =  CTL_get_date_stamp0();
    date_stamp1 =  CTL_get_date_stamp1();
    fprintf(stderr,"date_stamp = 0x%04x 0x%04x\n",
            (int)date_stamp1, (int)date_stamp0);


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

    uint64_t trig0, old_trig0;

    // Send a pulse back to the SYNC board. Give us a trigger.
    CTL_put_done();
    
    // start event loop
    signal(SIGTERM, handler);
    while(keeprunning) {

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

        CTL_reset_fifos();

        // get the next trigger
        if(PED) {
            // send put trigger to each ORM
            DATA_put_trigger_pulse(0);
            DATA_put_trigger_pulse(1);
            DATA_put_trigger_pulse(2);
            DATA_put_trigger_pulse(3);
        }
        else {

            // Send a pulse back to the SYNC board. Give us a trigger.
            old_trig0 = CTL_get_trig_count0();

            // OK to send trigger
            CTL_put_date_stamp0(1);

            // Wait for trigger.
            trig0 = old_trig0;
            while (trig0 == old_trig0) {
                if(!keeprunning) {
                    end_spi();
                    return 0;
                }
                trig0 = CTL_get_trig_count0();
            }

            // We have received a trigger, so its not OK to receive another
            // one until readout is complete and SKIs are reset.
            CTL_put_date_stamp0(0);
        }

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

        // wait for the FIFO to be empty, indicating IPBus has read it out
        int isFifoEmpty = 0;
        while(!isFifoEmpty){
            if(!keeprunning) {
                end_spi();
                return 0;
            }
            isFifoEmpty = CTL_get_empty();
        }

    }// event loop


    //===============================================================
    // CLOSING ACTIONS
    //===============================================================

    // done
    end_spi();
    return 0;    

}// end main

