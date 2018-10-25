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

    // power cycle
    // power_cycle_just_fpgas();

    // make pins 13, 15, 16, 18 HI
    bcm2835_gpio_fsel(RPI_GPIO_P1_13, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_13, HIGH);
    bcm2835_gpio_fsel(RPI_GPIO_P1_15, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_15, HIGH);
    bcm2835_gpio_fsel(RPI_GPIO_P1_16, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_16, HIGH);
    bcm2835_gpio_fsel(RPI_GPIO_P1_18, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_18, HIGH);

    // power cycle and wait til we can read the chip ID back
    int retval = 0;
    int orm, chip_id, sec;
    for(orm = 0; orm < 5; orm++) {
        power_cycle(orm);
        sec = 0;
        chip_id = read_chip_id(orm);
        while(chip_id != CHIP_ID) {
            if(sec > MAX_WAIT) {
                printf("Power cycle failed for ORM%d. Could not read the chip ID back in %d seconds.\n", orm, MAX_WAIT);
                retval = 1;
                break;
            }
            sleep(1);
            chip_id = read_chip_id(orm);
            sec++;
        }
    }

    // done
    end_spi();
    return retval;
}

