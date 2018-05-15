// gcc -O sync_debug.c sync_orm.c spi_common.c -l bcm2835 -o sync_debug.exe

#include "sync_orm.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
  if(argc != 2) {
    fprintf(stderr, "Need to specify to turn triggers on or off (1/0)\n");
    exit(1);
  }

  // Startup the SPI interface on the Pi.
  init_spi();

  SYNC_put_enable_trig(atoi(argv[1]));
  
  // Close the SPI interface on the Pi.
  end_spi();

  return(0);
}
