#include "data_orm.h"
#include <stdio.h>
#include <stdlib.h>

int HEXBD_send_generic(int hexbd, int rw, int a2, int a1, int a0, char c);
int HEXBD_read_generic(int hexbd, int a2, int a1, int a0);
int HEXBD_queue_status(int hexbd);
int HEXBD_send_command(int hexbd, char c);
int HEXBD_read_command(int hexbd);
int HEXBD_write_local_fifo(int hexbd, char c);
int HEXBD_read_local_fifo(int hexbd);
int HEXBD_read_usedwh(int hexbd);
int HEXBD_read_usedwl(int hexbd);
int HEXBD_trigger_gen(int hexbd);
int HEXBD_read200_local_fifo(int hexbd, int result[200]);
int HEXBD_read1000_local_fifo(int hexbd, int result[1000]);
int HEXBD_verify_communication(int verbose);
