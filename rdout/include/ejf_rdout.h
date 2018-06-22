#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

/*****************************************************/
/* define commands for Master FPGA */
/*****************************************************/
#define CMD_IDLE         0x80
#define CMD_RESETPULSE   0x88
#define CMD_WRPRBITS     0x90
#define CMDH_WRPRBITS    0x12 
#define CMD_SETSTARTACQ  0x98
#define CMD_STARTCONPUL  0xA0
#define CMD_STARTROPUL   0xA8
#define CMD_SETSELECT    0xB0
#define CMD_RSTBPULSE    0xD8
#define CMD_READSTATUS   0xC0
#define CMDH_READSTATUS  0x18
#define CMD_LOOPBRFIFO   0xF0
#define CMDH_LOOPBRFIFO  0x1E
#define CMD_LOOPBACK     0xF8
#define CMDH_LOOPBACK    0x1F

#define MAXEVENTS 100
#define MAXHEXBDS 8


/////////////////////Various delays///////////////
#define HX_DELAY1 100
#define HX_DELAY2 100
#define HX_DELAY3 3000
#define HX_DELAY4 100
/////////////////////////////////////////////////

#define RAWSIZE 30787
unsigned char raw[RAWSIZE];

uint32_t raw_32bit_new[RAWSIZE+1];

unsigned char tmp_raw[MAXHEXBDS][RAWSIZE];
uint32_t raw_32bit[RAWSIZE+1];


unsigned int ev[4][1924];
unsigned int data[4][128][15];

#define CRCSIZE 16
unsigned char crc[CRCSIZE];

int power_cycle_just_fpgas();
int power_cycle(int orm);
int read_raw(int hexbd);
int read_raw_faster(int hexbd) ;
int decode_raw();
int format_channels();
