#include <bcm2835.h>
#include "spi_common.h"
#include <stdio.h>

int main() {
	init_spi();
    printf("%d\n", get_board_id());
	end_spi();
	return 0;
}
