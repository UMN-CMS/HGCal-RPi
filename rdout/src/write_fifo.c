// gcc -O new_rdout.c ejf_rdout.c ctl_orm.c data_orm.c hexbd.c hexbd_config.c spi_common.c -l bcm2835 -o new_rdout.exe

#include <bcm2835.h>
#include <stdio.h>

#include "ctl_orm.h"
#include "spi_common.h"


//========================================================================
// MAIN
//========================================================================

int main(int argc, char *argv[])
{
    // Startup the SPI interface on the Pi.
    init_spi();

    // reset to clear fifo
    CTL_reset_all();

    // send stuff into fifo
    int count = CTL_get_clk_count0();
    CTL_write_ipb_fifo(count);
    count = CTL_get_clk_count1();
    CTL_write_ipb_fifo(count);
    count = CTL_get_clk_count2();
    CTL_write_ipb_fifo(count);

    // done
    end_spi();
    return 0;    
}// end main

