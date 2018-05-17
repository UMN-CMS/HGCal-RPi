#include "data_orm.h"
#include <stdio.h>
#include <stdlib.h>

int HEXBD_send_generic(int hexbd, int rw, int a2, int a1, int a0, char c)
{
  int spi_addr, value;

  a0 &= 1; // Paranoia.
  a1 &= 1; // Paranoia.
  a2 &= 1; // Paranoia.

  if ((hexbd & 1) == 0) spi_addr = DATA_HEXA0_COMMAND;
  else spi_addr = DATA_HEXA1_COMMAND;
  
  value = c & 0xFF;
  value |= (rw<<11) | (a2<<10) | (a1<<9) | (a0<<8);

  spi_put_16bits((hexbd>>1)&3, spi_addr, value);
  
  return(0);
}

int HEXBD_read_generic(int hexbd, int a2, int a1, int a0)
{
  int rw, result, spi_addr, empty_flag;
  char c;

   a0 &= 1; // Paranoia.
   a1 &= 1; // Paranoia.
   a2 &= 1; // Paranoia.
   
   // Start by sending the command, then get the response.
   rw = 1; // Do a read.
   c = 0x00; // unused
   HEXBD_send_generic(hexbd, rw, a2, a1, a0, c);
   
   // Wait while this response fifo is empty.
   empty_flag = 1;
   int loop_count;
   loop_count = 0;
   while (empty_flag != 0) {
     loop_count++;
     result = HEXBD_queue_status(hexbd);
     empty_flag = result & 1;
     if (loop_count > 20) exit(-1);
   }
   
   // Get the response.
   if ((hexbd & 1) == 0) spi_addr = DATA_HEXA0_RESPONSE;
   else spi_addr = DATA_HEXA1_RESPONSE;
   result = spi_get_16bits((hexbd>>1)&3, spi_addr);
   return(result & 0xFF);
}

int HEXBD_queue_status(int hexbd)
{
  int spi_addr, result;
  if ((hexbd & 1) == 0) spi_addr = DATA_HEXA0_STATUS;
  else spi_addr = DATA_HEXA1_STATUS;
  result = spi_get_16bits((hexbd>>1)&3, spi_addr);
  if (0) fprintf(stderr,"queue_status: command=0x%02x response=0x%02x\n",
		 (int)((result>>8)&0xFF),(int)(result&0xFF));
  return(result);
}

int HEXBD_send_command(int hexbd, char c)
{
  int rw, a2, a1, a0, status;
   rw = 0; // Do a write.
   a2 = 0;
   a1 = 0;
   a0 = 0;
   status = HEXBD_send_generic(hexbd, rw, a2, a1, a0, c);
   return(status);
}

int HEXBD_read_command(int hexbd)
{
   int a2, a1, a0, result;
   a2 = 0;
   a1 = 0;
   a0 = 0;
   result = HEXBD_read_generic(hexbd, a2, a1, a0);
   return(result);
}

int HEXBD_write_local_fifo(int hexbd, char c)
{
  int rw, a2, a1, a0, status;
  rw = 0; // Do a write.
  a2 = 0;
  a1 = 0;
  a0 = 1;
  status = HEXBD_send_generic(hexbd, rw, a2, a1, a0, c);
  return(status);
}

int HEXBD_read_local_fifo(int hexbd)
{
   int a2, a1, a0, result;
   a2 = 0;
   a1 = 0;
   a0 = 1;
   result = HEXBD_read_generic(hexbd, a2, a1, a0);
   return(result);
}

int HEXBD_read_usedwh(int hexbd)
{
   int a2, a1, a0, result;
   a2 = 0;
   a1 = 1;
   a0 = 1;
   result = HEXBD_read_generic(hexbd, a2, a1, a0);
   return(result);
}

int HEXBD_read_usedwl(int hexbd)
{
   int a2, a1, a0, result;
   a2 = 0;
   a1 = 1;
   a0 = 0;
   result = HEXBD_read_generic(hexbd, a2, a1, a0);
   return(result);
}

int HEXBD_trigger_gen(int hexbd)
{
   int rw, a2, a1, a0, status;
   char c;
   rw = 0;
   a2 = 1;
   a1 = 1;
   a0 = 1;
   c = 0x00; // unused
   status = HEXBD_send_generic(hexbd, rw, a2, a1, a0, c);
   return(0);
}

int HEXBD_read200_local_fifo(int hexbd, int result[200])
{
  unsigned indx, a2, a1, a0, rw;
  unsigned char page[4], c;
  unsigned spi_read, spi_auto_inc, spi_addr, spi_command;
  unsigned almost_empty_flag, almost_full_flag;
  unsigned char status_cmd[4];
  unsigned char send_cmd[(2*200)+12];
  unsigned char response_cmd[(2*200)+12];
  unsigned char data[(2*200)+12];
  int i;

  // Decode the index of the hexaboard.
  if      (hexbd == 0) { page[0] = 2; indx = 0; } // ORM_0
  else if (hexbd == 1) { page[0] = 2; indx = 1; } // ORM_0
  else if (hexbd == 2) { page[0] = 4; indx = 0; } // ORM_1
  else if (hexbd == 3) { page[0] = 4; indx = 1; } // ORM_1
  else if (hexbd == 4) { page[0] = 6; indx = 0; } // ORM_2
  else if (hexbd == 5) { page[0] = 6; indx = 1; } // ORM_2
  else if (hexbd == 6) { page[0] = 8; indx = 0; } // ORM_3
  else if (hexbd == 7) { page[0] = 8; indx = 1; } // ORM_3
  else return(-1);
  
  // Select the appropriate oRM.
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  bcm2835_spi_writenb(page,1);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
  
  // Create the SPI command for FIFO status.
  spi_read = 1;
  spi_auto_inc = 0;
  if (indx == 0) spi_addr = DATA_HEXA0_STATUS;
  else spi_addr = DATA_HEXA1_STATUS;
  spi_command = (spi_read<<31)|(spi_auto_inc<<30)|(spi_addr<<20);
  status_cmd[0] = spi_command >> 24;
  status_cmd[1] = spi_command >> 16;
  status_cmd[2] = spi_command >> 8;
  status_cmd[3] = spi_command >> 0;
  
  // Create the SPI commands for read_local_fifo.
  a2 = 0;
  a1 = 0;
  a0 = 1;
  rw = 1; // Do a read.
  c = 0x00; // unused, since {a2,a1,a0} determine the command
  spi_read = 0;
  spi_auto_inc = 0;
  if (indx == 0) spi_addr = DATA_HEXA0_COMMAND;
  else spi_addr = DATA_HEXA1_COMMAND;
  spi_command = (spi_read<<31)|(spi_auto_inc<<30)|(spi_addr<<20)|
                (rw<<11)|(a2<<10)|(a1<<9)|(a0<<8)|(c<<0);
  send_cmd[0] = spi_command >> 24;
  send_cmd[1] = spi_command >> 16;
  send_cmd[2] = spi_command >>  8;
  send_cmd[3] = spi_command >>  0;
  for (i=1; i<200; i++) send_cmd[2+(2*i)] = send_cmd[2];
  for (i=1; i<200; i++) send_cmd[3+(2*i)] = send_cmd[3];
  
  // Create the SPI command for the response.
  spi_read = 1;
  spi_auto_inc = 0;
  if (indx == 0) spi_addr = DATA_HEXA0_RESPONSE;
  else spi_addr = DATA_HEXA1_RESPONSE;
  spi_command = (spi_read<<31)|(spi_auto_inc<<30)|(spi_addr<<20);
  response_cmd[0] = spi_command >> 24;
  response_cmd[1] = spi_command >> 16;
  response_cmd[2] = spi_command >> 8;
  response_cmd[3] = spi_command >> 0;
  
  //--------------------------------------------------------------------

  // Wait until the command fifo is not almost_full.
  almost_full_flag = 1;
  while (almost_full_flag == 1) {
    bcm2835_spi_transfernb(status_cmd, data, 4); 
    almost_full_flag = (data[2] >> 3) & 1; // ??? check this
  }
  
  // Since the SPI speed is slower than firmware, proceed 
  // with transmition of all commands.
  bcm2835_spi_transfernb(send_cmd, data, 2+(2*200));
  
  // Wait while the response fifo is not almost_empty.
  almost_empty_flag = 1;
  while (almost_empty_flag == 1) {
    bcm2835_spi_transfernb(status_cmd, data, 4);
    almost_empty_flag = (data[3] >> 2) & 1; // ??? check this
  }
  
  // Since the SPI speed is slower than firmware, proceed 
  // with reception of all responses.
  bcm2835_spi_transfernb(response_cmd, data, 2+(2*200));
  
  // Return the results.
  for (i=0; i<200; i++) result[i] = data[3 + (2*i)];
  return(0);
}

int HEXBD_read1000_local_fifo(int hexbd, int result[1000])
{
  unsigned indx, a2, a1, a0, rw;
  unsigned char page[4], c;
  unsigned spi_read, spi_auto_inc, spi_addr, spi_command;
  unsigned almost_empty_flag, almost_full_flag;
  unsigned char status_cmd[4];
  unsigned char send_cmd[(2*1000)+12];
  unsigned char response_cmd[(2*1000)+12];
  unsigned char data[(2*1000)+12];
  int i;

  // Decode the index of the hexaboard.
  if      (hexbd == 0) { page[0] = 2; indx = 0; } // ORM_0
  else if (hexbd == 1) { page[0] = 2; indx = 1; } // ORM_0
  else if (hexbd == 2) { page[0] = 4; indx = 0; } // ORM_1
  else if (hexbd == 3) { page[0] = 4; indx = 1; } // ORM_1
  else if (hexbd == 4) { page[0] = 6; indx = 0; } // ORM_2
  else if (hexbd == 5) { page[0] = 6; indx = 1; } // ORM_2
  else if (hexbd == 6) { page[0] = 8; indx = 0; } // ORM_3
  else if (hexbd == 7) { page[0] = 8; indx = 1; } // ORM_3
  else return(-1);
  
  // Select the appropriate oRM.
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  bcm2835_spi_writenb(page,1);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
  
  // Create the SPI command for FIFO status.
  spi_read = 1;
  spi_auto_inc = 0;
  if (indx == 0) spi_addr = DATA_HEXA0_STATUS;
  else spi_addr = DATA_HEXA1_STATUS;
  spi_command = (spi_read<<31)|(spi_auto_inc<<30)|(spi_addr<<20);
  status_cmd[0] = spi_command >> 24;
  status_cmd[1] = spi_command >> 16;
  status_cmd[2] = spi_command >> 8;
  status_cmd[3] = spi_command >> 0;
  
  // Create the SPI commands for read_local_fifo.
  a2 = 0;
  a1 = 0;
  a0 = 1;
  rw = 1; // Do a read.
  c = 0x00; // unused, since {a2,a1,a0} determine the command
  spi_read = 0;
  spi_auto_inc = 0;
  if (indx == 0) spi_addr = DATA_HEXA0_COMMAND;
  else spi_addr = DATA_HEXA1_COMMAND;
  spi_command = (spi_read<<31)|(spi_auto_inc<<30)|(spi_addr<<20)|
                (rw<<11)|(a2<<10)|(a1<<9)|(a0<<8)|(c<<0);
  send_cmd[0] = spi_command >> 24;
  send_cmd[1] = spi_command >> 16;
  send_cmd[2] = spi_command >>  8;
  send_cmd[3] = spi_command >>  0;
  for (i=1; i<1000; i++) send_cmd[2+(2*i)] = send_cmd[2];
  for (i=1; i<1000; i++) send_cmd[3+(2*i)] = send_cmd[3];
  
  // Create the SPI command for the response.
  spi_read = 1;
  spi_auto_inc = 0;
  if (indx == 0) spi_addr = DATA_HEXA0_RESPONSE;
  else spi_addr = DATA_HEXA1_RESPONSE;
  spi_command = (spi_read<<31)|(spi_auto_inc<<30)|(spi_addr<<20);
  response_cmd[0] = spi_command >> 24;
  response_cmd[1] = spi_command >> 16;
  response_cmd[2] = spi_command >> 8;
  response_cmd[3] = spi_command >> 0;
  
  //--------------------------------------------------------------------

 
  // Wait until the command fifo is not almost_full.
  almost_full_flag = 1;
  while (almost_full_flag == 1) {
    bcm2835_spi_transfernb(status_cmd, data, 4); 
    almost_full_flag = (data[2] >> 3) & 1; // ??? check this
  } 
  
  // Since the SPI speed is slower than firmware, proceed 
  // with transmition of all commands.
  bcm2835_spi_transfernb(send_cmd, data, 2+(2*1000));
  
  // Wait while the response fifo is not almost_empty.
  almost_empty_flag = 1;
  while (almost_empty_flag == 1) {
    bcm2835_spi_transfernb(status_cmd, data, 4);
    almost_empty_flag = (data[3] >> 2) & 1; // ??? check this
  }
  
  // Since the SPI speed is slower than firmware, proceed 
  // with reception of all responses.
  bcm2835_spi_transfernb(response_cmd, data, 2+(2*1000));
  
  // Return the results.
  for (i=0; i<1000; i++) result[i] = data[3 + (2*i)];
  return(0);
}

int HEXBD_verify_communication(int verbose)
{
  int i, hexbd, error[8], status, data[20];
  
  // Loop over all 8 hexaboards.
  for (hexbd=0; hexbd<8; hexbd++) {
    if (verbose) fprintf(stderr,"hexbd: %d\n",(int)hexbd);
    
    // Verify that the command and response queues are empty.
    error[hexbd] = 0;
    status = HEXBD_queue_status(hexbd);
    if ((status & 0x0101) != 0x0101) error[hexbd] = 1;
    if (verbose) fprintf(stderr,"hexbd: %d, queue error = %d\n",
			 (int)hexbd, (int)error[hexbd]);
    
    if (error[hexbd] == 0) {
      // Write 10 values to the local_fifo.
      // Read 10 values from the local_fifo.
      // Check the results from the local_fifo.
      if (0) fprintf(stderr,"hexbd: %d, writing local fifo.\n",(int)hexbd);
      for (i=0; i<10; i++) HEXBD_write_local_fifo(hexbd,5^i);
      if (0) fprintf(stderr,"hexbd: %d, reading local fifo.\n",(int)hexbd);
      for (i=0; i<10; i++) data[i] = HEXBD_read_local_fifo(hexbd);
      if (0) {
	fprintf(stderr,"hexbd: %d, checking: ",(int)hexbd);
	for (i=0; i<10; i++) fprintf(stderr,"%04x ",(int)data[i]);
	fprintf(stderr,"\n");
      }
      for (i=0; i<10; i++) if (data[i] != (5^i)) error[hexbd] = 1;
      if (verbose) fprintf(stderr,"hexbd: %d, fifo error = %d\n",
			   (int)hexbd, (int)error[hexbd]);
    }
    
    if (error[hexbd] == 0) {
      // Send 10 CMD_LOOPBACK commands to the hexaboard, and check the results.
      for (i=0; i<10; i++) {
	HEXBD_send_command(hexbd, (0xF8 | ((i^2)&7)));
	data[0] = HEXBD_read_command(hexbd);
	if (data[0] != (0xF8 | ((i^2)&7))) error[hexbd] = 1;
      }
      if (verbose) fprintf(stderr,"hexbd: %d, loop error = %d\n",
			   (int)hexbd, (int)error[hexbd]);
    }
  }

  int ok_result;
  ok_result = 0xFF;
  for (hexbd=0; hexbd<8; hexbd++) ok_result ^= error[hexbd] << hexbd;
  return(ok_result);
}
