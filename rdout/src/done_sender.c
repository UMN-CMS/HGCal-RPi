#include <bcm2835.h>
#include "spi_common.h"
#include "ctl_orm.h"
#include "data_orm.h"
#include "hexbd.h"
#include "hexbd_config.h"
#include "ejf_rdout.h"

#include <stdio.h>

int main() {
	init_spi();

	int i;
	for(i = 0; i < 65536; i++) {
		CTL_put_done();
	}

	end_spi();
	return 0;
}
