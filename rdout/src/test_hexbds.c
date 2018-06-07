#include <bcm2835.h>
#include <stdio.h>

#include "spi_common.h"
#include "hexbd.h"


int main() {

    // variables
    int hx, hexbd_mask, skiroc_mask0, skiroc_mask1;

    // start spi
    init_spi();

    // test each of the hexbds
    hexbd_mask = HEXBD_verify_communication(1);
    printf("hexbd_mask = 0x%02x\n", hexbd_mask);

    // show the skiroc mask
    skiroc_mask0 = 0;
    for (hx=0; hx<4; hx++) {
        if ((hexbd_mask & (1<<hx)) != 0) skiroc_mask0 |= 0xF << (4*hx);
    }
    skiroc_mask1 = 0;
    for (hx=4; hx<8; hx++) {
        if ((hexbd_mask & (1<<hx)) != 0) skiroc_mask1 |= 0xF << ((4*hx)-16);
    }
    printf("skiroc_mask = 0x%04x 0x%04x\n", skiroc_mask1, skiroc_mask0);

    // done
    end_spi();
    return 0;    
}
