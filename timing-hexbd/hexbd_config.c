#include "hexbd_config.h"
#include "data_orm.h"

#include <string.h>

#define TIMING_HEXBD (7)

void setup_prog_strings(char prog_strings[4][48]) {
    char default_prog_string[48] = 
    {   0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2c, 0xe0, 0x98, 0xb0, \
        0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff, \
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
        0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
    };
    char maskch22_prog_string[48] =
    {   0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2c, 0xe0, 0x98, 0xb0, \
        0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff, \
        0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
        0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, \
        0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
    };

    // copy the default prog string in for all 4 chips
    int chip;
    for(chip = 0; chip < 4; chip++) {
        memcpy(prog_strings[chip], default_prog_string, sizeof(default_prog_string));
    }

    // copy the special string into the first element of the array
    // this will get pushed into the last chip (chip 3)
    memcpy(prog_strings[0], maskch22_prog_string, sizeof(maskch22_prog_string));
}

void setup_prog_strings_timing(char prog_strings[4][48]) {
    char timing_prog_string[48] = 
    {   0xDA,0xA0,0xFF,0x32,0xE0,0xC1,0x2E,0x10,0x98,0xB0,  \
        0x40,0x00,0x20,0x08,0x00,0x00,0x00,0x00,0x1F,0xFF,  \
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  \
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  \
        0xFF,0xFF,0xE9,0xD7,0xAE,0xBA,0x80,0x25
    };

    // copy the timing prog string in for all 4 chips
    int chip;
    for(chip = 0; chip < 4; chip++) {
        memcpy(prog_strings[chip], timing_prog_string, sizeof(timing_prog_string));
    }
}

int ConvertProgrStrBytetoBit(char * bytes, char * bits)
{
    int i, j;
    char b;
    for (i = 0; i < 48; i = i + 1){
        b = *(bytes + sizeof(char) * i);
        for(j = 0; j < 8; j = j + 1){
            *(bits + sizeof(char) * j + sizeof(char) * i * 8) = 1 & (b >> (7-j));
        }
    }
    return(0);
}

int ConvertProgrStrBittoByte(char * bits, char * bytes)
{  
    int i, j;
    char b;
    for (i = 0; i < 48; i = i + 1){
        b = 0;
        for(j = 0; j < 8; j = j + 1){
            b = b | ( *(bits + sizeof(char) * i*8 + sizeof(char) * j) << (7 - j));
        }
        *(bytes + sizeof(char) * i) = b;
    }
    return(0);
}

// Program the 48 bytes configuration string into the SK2 3 bits at a time
// for all 4 chips on Hexaboard.
int prog384(int hexbd, char * pNew, char * pPrevious, int verbose)
{
    int chip, bit;
    char bit0, bit1, bit2, bits, cmd;
    char dout;
    int status;
    status = 0;
    for(chip = 0; chip < 4; chip = chip+1){
        for(bit = 0; bit < 384; bit = bit + 3){
            bit2 = *(pNew + sizeof(char) * bit + 0);
            bit1 = *(pNew + sizeof(char) * bit + 1);
            bit0 = *(pNew + sizeof(char) * bit + 2);
            bits = (bit2 << 2) | (bit1 << 1) | bit0;
            cmd = CMD_WRPRBITS | bits;
            HEXBD_send_command(hexbd, cmd);
            dout = HEXBD_read_command(hexbd);
            if (cmd != dout) {
                if (verbose) fprintf(stderr,"ERROR: Command Sent %2x,", cmd);
                if (verbose) fprintf(stderr,"Command Returned %2x\n", dout);
                status = -1;
            }
            bits = dout & 7;
            bit2 = (bits >> 2) & 1;
            bit1 = (bits >> 1) & 1;
            bit0 = bits & 1;
            *(pPrevious + sizeof(char) * bit + 0) = bit2;
            *(pPrevious + sizeof(char) * bit + 1) = bit1;
            *(pPrevious + sizeof(char) * bit + 2) = bit0;
        }
    }
    return(status);
}

int progandverify48 (int hexbd, char * pConfBytes, char * pPrevious, int verbose)
{
    char *pNewConfBits;
    char *pOldConfBits;
    int status1, status2;

    pNewConfBits = (char *) malloc(sizeof(char) * 384);
    pOldConfBits = (char *) malloc(sizeof(char) * 384);

    ConvertProgrStrBytetoBit(pConfBytes, pNewConfBits);

    status1 = prog384(hexbd, pNewConfBits, pOldConfBits, verbose);
    status2 = prog384(hexbd, pNewConfBits, pOldConfBits, verbose);

    ConvertProgrStrBittoByte(pOldConfBits, pPrevious);

    free(pNewConfBits);
    free(pOldConfBits);

    return(status1+status2);
}

int configure_hexaboard(int hexbd, int verbose)
{
    char return_string[48];
    char prog_string[48] = 
    {   0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2c, 0xe0, 0x98, 0xb0, \
        0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff, \
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
        0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
    };
    char timing_prog_string[48] = 
    {   0xDA,0xA0,0xFF,0x32,0xE0,0xC1,0x2E,0x10,0x98,0xB0,  \
        0x40,0x00,0x20,0x08,0x00,0x00,0x00,0x00,0x1F,0xFF,  \
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  \
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  \
        0xFF,0xFF,0xE9,0xD7,0xAE,0xBA,0x80,0x25
    };

    int status;

    HEXBD_send_command(hexbd, CMD_RESETPULSE);
    HEXBD_send_command(hexbd, CMD_SETSELECT | 1);
    HEXBD_send_command(hexbd, CMD_RSTBPULSE);

    if(hexbd == TIMING_HEXBD) {
        status = progandverify48(hexbd, timing_prog_string, return_string, verbose);
    } else {
        status = progandverify48(hexbd, prog_string, return_string, verbose);
    }

    HEXBD_send_command(hexbd, CMD_SETSELECT | 0);

    return(status);
}


// Program the 48 bytes configuration string into the SK2 3 bits at a time for one chip
int prog384_singlechip(int hexbd, char * pNew, char * pPrevious, int verbose)
{
    int chip, bit;
    char bit0, bit1, bit2, bits, cmd;
    char dout;
    int status;
    status = 0;
    for(bit = 0; bit < 384; bit = bit + 3){
        bit2 = *(pNew + sizeof(char) * bit + 0);
        bit1 = *(pNew + sizeof(char) * bit + 1);
        bit0 = *(pNew + sizeof(char) * bit + 2);
        bits = (bit2 << 2) | (bit1 << 1) | bit0;
        cmd = CMD_WRPRBITS | bits;
        HEXBD_send_command(hexbd, cmd);
        dout = HEXBD_read_command(hexbd);
        if (cmd != dout) {
            if (verbose) fprintf(stderr,"ERROR: Command Sent %2x,", cmd);
            if (verbose) fprintf(stderr,"Command Returned %2x\n", dout);
            status = -1;
        }
        bits = dout & 7;
        bit2 = (bits >> 2) & 1;
        bit1 = (bits >> 1) & 1;
        bit0 = bits & 1;
        *(pPrevious + sizeof(char) * bit + 0) = bit2;
        *(pPrevious + sizeof(char) * bit + 1) = bit1;
        *(pPrevious + sizeof(char) * bit + 2) = bit0;
    }
    return(status);
}


int progandverify48_singlechip(int hexbd, char * pConfBytes, char * pPrevious, int verbose)
{
    char *pNewConfBits;
    char *pOldConfBits;
    int status1, status2;

    pNewConfBits = (char *) malloc(sizeof(char) * 384);
    pOldConfBits = (char *) malloc(sizeof(char) * 384);

    ConvertProgrStrBytetoBit(pConfBytes, pNewConfBits);

    status1 = prog384_singlechip(hexbd, pNewConfBits, pOldConfBits, verbose);
    status2 = 0;//prog384_singlechip(hexbd, pNewConfBits, pOldConfBits, verbose);

    ConvertProgrStrBittoByte(pOldConfBits, pPrevious);

    free(pNewConfBits);
    free(pOldConfBits);

    return(status1+status2);
}


int configure_hexaboard_perskiroc(int hexbd, char prog_strings[4][48], int verbose)
{
    char return_strings[4][48];
    int status[4];
    int chip;

    // make sure the hexaboard is ready to be configured
    HEXBD_send_command(hexbd, CMD_RESETPULSE);
    HEXBD_send_command(hexbd, CMD_SETSELECT | 1);
    HEXBD_send_command(hexbd, CMD_RSTBPULSE);

    // configure the 4 skirocs
    for(chip = 0; chip < 4; chip++) {
        status[chip] = progandverify48_singlechip(hexbd, prog_strings[chip], return_strings[chip], verbose);
    }

    // done configuring
    HEXBD_send_command(hexbd, CMD_SETSELECT | 0);

    return status[0]+status[1]+status[2]+status[3];
}
