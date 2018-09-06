#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>

#include "ejf_rdout.h"
#include "spi_common.h"


#define CHIP_ID 0x20ba18
#define MAX_WAIT 15

int main() {

    // startup spi
    init_spi();

    // make pins 13, 15, 16, 18 HI to keep hexaboards powered
    bcm2835_gpio_fsel(RPI_GPIO_P1_13, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_13, HIGH);
    bcm2835_gpio_fsel(RPI_GPIO_P1_15, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_15, HIGH);
    bcm2835_gpio_fsel(RPI_GPIO_P1_16, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_16, HIGH);
    bcm2835_gpio_fsel(RPI_GPIO_P1_18, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_18, HIGH);

    // power cycle
    int orm;
    for(orm = 0; orm < 5; orm++) {
        power_cycle(orm);
    }

    // wait until we can read the chip id back
    int retval = 0;
    int chip_id, sec;
    int success[5] = {0,0,0,0,0};
    for(sec = 0; sec < MAX_WAIT; sec++) {
        for(orm = 0; orm < 5; orm++) {
            if(read_chip_id(orm) == CHIP_ID) success[orm] = 1;
        }
        sleep(1);
    }
    for(orm = 0; orm < 5; orm++) {
        if(success[orm] != 1) {
            printf("power cycle failed for orm%d - could not read chip id in %d seconds\n", orm, MAX_WAIT);
            retval = 1;
        }
    }

    // done
    end_spi();
    return retval;
}

