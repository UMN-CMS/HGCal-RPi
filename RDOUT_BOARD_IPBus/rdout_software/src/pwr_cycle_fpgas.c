#include <bcm2835.h>
#include <stdio.h>

#include "hexbd_config.h"
#include "ctl_orm.h"
#include "data_orm.h"
#include "ejf_rdout.h"
#include "spi_common.h"


int main()
{

  // Startup the SPI interface on the Pi.
  init_spi();

  // Power cycle the ORMs.
  fprintf(stderr, "power cycling all fpga's...");
  power_cycle_just_fpgas();
  fprintf(stderr, "done.\n");
  /*
    fprintf(stderr,"power cycle orm: data_0...");
    power_cycle(0); // DATA_0
    fprintf(stderr,"done.\n");
    sleep(1);
    fprintf(stderr,"power cycle orm: data_1...");
    power_cycle(1); // DATA_1
    fprintf(stderr,"done.\n");
    sleep(1);
    fprintf(stderr,"power cycle orm: data_2...");
    power_cycle(2); // DATA_2
    fprintf(stderr,"done.\n");
    sleep(1);
    fprintf(stderr,"power cycle orm: data_3...");
    power_cycle(3); // DATA_3
    fprintf(stderr,"done.\n");
    sleep(1);
    fprintf(stderr,"power cycle orm: ctl...");
    power_cycle(4); // CTL
    fprintf(stderr,"done.\n");
    sleep(5);
  */

  end_spi();
  return(0);    
  
}// Main ends here

