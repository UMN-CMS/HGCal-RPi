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
  /*char prog_string[48] = 
    {  0xda, 0xab, 0xff, 0x32, 0xe0, 0xd5, 0x3f, 0xe7, 0xf8, 0xb0,	\
       0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1f, 0xff,	\
       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	\
       0xff, 0xff, 0xe9, 0xd5, 0x22, 0x8a, 0x80, 0x25}; */
char prog_string[48] = { 0xDA, 0xA0, 0xF9, 0x32, 0xE0, 0xC1, 0x2E, 0x10, 0x98, 0xB0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE9, 0xD7, 0xAE, 0xBA, 0x80, 0x25 };
  
  int status;

  HEXBD_send_command(hexbd, CMD_RESETPULSE);
  HEXBD_send_command(hexbd, CMD_SETSELECT | 1);
  HEXBD_send_command(hexbd, CMD_RSTBPULSE);

  status = progandverify48(hexbd, prog_string, return_string, verbose);    

  HEXBD_send_command(hexbd, CMD_SETSELECT | 0);

  return(status);
}
