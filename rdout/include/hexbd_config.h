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

//************************************
//* define commands for Master FPGA 
//************************************
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

int ConvertProgrStrBytetoBit(char * bytes, char * bits);
int ConvertProgrStrBittoByte(char * bits, char * bytes);
int prog384(int hexbd, char * pNew, char * pPrevious, int verbose);
int progandverify48(int hexbd, char * pConfBytes, char * pPrevious, int verbose);
int configure_hexaboard(int hexbd, int verbose);
int prog384_singlechip(int hexbd, char *pNew, char *pPrevious, int verbose);
int progandverify48_singlechip(int hexbd, char * pConfBytes, char * pPrevious, int verbose);
int configure_hexaboard_perskiroc(int hexbd, int verbose);

char default_prog_string[48] = 
{   0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2e, 0x10, 0x98, 0xb0,	\
    0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff,	\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
    0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
};
