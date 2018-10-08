#include "hexbd_config.h"
#include "data_orm.h"

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
    {   0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2e, 0x10, 0x98, 0xb0,	\
        0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff,	\
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
        0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
    };

    int status;

    HEXBD_send_command(hexbd, CMD_RESETPULSE);
    HEXBD_send_command(hexbd, CMD_SETSELECT | 1);
    HEXBD_send_command(hexbd, CMD_RSTBPULSE);

    status = progandverify48(hexbd, prog_string, return_string, verbose);    

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
    status2 = prog384_singlechip(hexbd, pNewConfBits, pOldConfBits, verbose);

    ConvertProgrStrBittoByte(pOldConfBits, pPrevious);

    free(pNewConfBits);
    free(pOldConfBits);

    return(status1+status2);
}


int configure_hexaboard_perskiroc(int hexbd, int verbose)
{
    // set up the config strings
    char return_strings[4][48];
    char prog_string[48] = 
    {   0xda, 0xa0, 0xf9, 0x32, 0xe0, 0xc1, 0x2e, 0x10, 0x98, 0xb0,	\
        0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff,	\
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
        0xff, 0xff, 0xe9, 0xd7, 0xae, 0xba, 0x80, 0x25
    };

    // make sure the hexaboard is ready to be configured
    HEXBD_send_command(hexbd, CMD_RESETPULSE);
    HEXBD_send_command(hexbd, CMD_SETSELECT | 1);
    HEXBD_send_command(hexbd, CMD_RSTBPULSE);

    int status1,status2,status3,status4;
    status1 = progandverify48_singlechip(hexbd, prog_string, return_strings[0], verbose);    
    status2 = progandverify48_singlechip(hexbd, prog_string, return_strings[1], verbose);    
    status3 = progandverify48_singlechip(hexbd, prog_string, return_strings[2], verbose);    
    status4 = progandverify48_singlechip(hexbd, prog_string, return_strings[3], verbose);    

    // done configuring
    HEXBD_send_command(hexbd, CMD_SETSELECT | 0);

    return(status1+status2+status3+status4);
}
