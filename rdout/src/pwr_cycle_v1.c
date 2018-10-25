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
    power_cycle_just_fpgas();

    // wait til we can read the chip ID back
    int retval = 0;
    int orm, chip_id, sec;
    for(orm = 0; orm < 5; orm++) {
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

