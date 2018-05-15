// gcc -O sync_debug.c sync_orm.c spi_common.c -l bcm2835 -o sync_debug.exe

#include "sync_orm.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
  // Startup the SPI interface on the Pi.
  init_spi();

  int enable_trig = SYNC_get_enable_trig();
  printf("%i\n", enable_trig);

  // Close the SPI interface on the Pi.
  end_spi();

  return(0);
}
