#include "data_orm.h"

int DATA_reset_crc_fifo(int hexbd)
{ 
  int indx;
  indx = hexbd & 1;
  indx *= 8;
  indx += 3; // reset_crc_fifo
  return(spi_put_16bits((hexbd>>1)&3,DATA_RESET_PULSE, 1<<indx)); 
}

int DATA_reset_crc(int hexbd)
{ 
  int indx;
  indx = hexbd & 1;
  indx *= 8;
  indx += 2; // reset_crc
  return(spi_put_16bits((hexbd>>1)&3,DATA_RESET_PULSE, 1<<indx)); 
}

int DATA_reset_local_fifo(int hexbd)
{ int indx;
  indx = hexbd & 1;
  indx *= 8;
  indx += 1; // reset_local_fifo
  return(spi_put_16bits((hexbd>>1)&3,DATA_RESET_PULSE, 1<<indx)); 
}

int DATA_reset_all(int hexbd)
{ 
  int indx;  
  indx = hexbd & 1;
  indx *= 8;
  indx += 0; // reset_all
  return(spi_put_16bits((hexbd>>1)&3,DATA_RESET_PULSE, 1<<indx)); 
}

int DATA_get_firmware_version(int orm)
{ return(spi_get_16bits(orm, DATA_FIRMWARE_VERSION)); }

int DATA_get_constant0(int orm)
{ return(spi_get_16bits(orm, DATA_CONSTANT0)); }

int DATA_get_constant1(int orm)
{ return(spi_get_16bits(orm, DATA_CONSTANT1)); }

int DATA_get_dummy0(int orm)
{ return(spi_get_16bits(orm, DATA_DUMMY_REG0)); }

int DATA_put_dummy0(int orm, int value16bits)
{ return(spi_put_16bits(orm, DATA_DUMMY_REG0, value16bits)); }

int DATA_get_dummy1(int orm)
{ return(spi_get_16bits(orm, DATA_DUMMY_REG1)); }

int DATA_put_dummy1(int orm, int value16bits)
{ return(spi_put_16bits(orm, DATA_DUMMY_REG1, value16bits)); }

int DATA_put_trigger_pulse(int orm)
{ return(spi_put_16bits(orm, DATA_TRIG_PULSE, 0)); }

int DATA_get_xfer_enable(int orm)
{ return(spi_get_16bits(orm, DATA_XFER_ENABLE)); }

int DATA_put_xfer_enable(int orm, int value2bits)
{ return(spi_put_16bits(orm, DATA_XFER_ENABLE, value2bits & 3)); }

int DATA_get_calib_mode(int orm)
{ return(spi_get_16bits(orm, DATA_CALIB_MODE)); }

int DATA_put_calib_mode(int orm, int value1bit)
{ return(spi_put_16bits(orm, DATA_CALIB_MODE, value1bit & 1)); }

int DATA_put_calib_pulse(int orm)
{ return(spi_put_16bits(orm, DATA_CALIB_PULSE, 0)); }

int DATA_get_calib_delay_en0(int orm)
{ return(spi_get_16bits(orm, DATA_CALIB_DELAY_EN0)); }

int DATA_put_calib_delay_en0(int orm, int value16bits)
{ return(spi_put_16bits(orm, DATA_CALIB_DELAY_EN0, value16bits)); }

int DATA_get_calib_fine_dly0(int orm)
{ return(spi_get_16bits(orm, DATA_CALIB_FINE_DLY0)); }

int DATA_put_calib_fine_dly0(int orm, int value16bits)
{ return(spi_put_16bits(orm, DATA_CALIB_FINE_DLY0, value16bits)); }

int DATA_get_calib_delay_en1(int orm)
{ return(spi_get_16bits(orm, DATA_CALIB_DELAY_EN1)); }

int DATA_put_calib_delay_en1(int orm, int value16bits)
{ return(spi_put_16bits(orm, DATA_CALIB_DELAY_EN1, value16bits)); }

int DATA_get_calib_fine_dly1(int orm)
{ return(spi_get_16bits(orm, DATA_CALIB_FINE_DLY1)); }

int DATA_put_calib_fine_dly1(int orm, int value16bits)
{ return(spi_put_16bits(orm, DATA_CALIB_FINE_DLY1, value16bits)); }

int DATA_get_calib_delay_trig(int orm)
{ return(spi_get_16bits(orm, DATA_CALIB_DELAY_TRG)); }

int DATA_put_calib_delay_trig(int orm, int value16bits)
{ return(spi_put_16bits(orm, DATA_CALIB_DELAY_TRG, value16bits)); }

int DATA_get_calib_amplitude(int orm)
{ return(spi_get_16bits(orm, DATA_CALIB_DELAY_TRG)); }

int DATA_put_calib_amplitude(int orm, int value16bits)
{ return(spi_put_16bits(orm, DATA_CALIB_DELAY_TRG, value16bits)); }

int DATA_get_pedestal_mode(int orm)
{ return(spi_get_16bits(orm, DATA_PEDESTAL_MODE)); }

int DATA_put_pedestal_mode(int orm, int value1bit)
{ return(spi_put_16bits(orm, DATA_PEDESTAL_MODE, value1bit & 1)); }

int DATA_get_debug_mode(int orm)
{ return(spi_get_16bits(orm, DATA_DEBUG_MODE)); }

int DATA_put_debug_mode(int orm, int value1bit)
{ return(spi_put_16bits(orm, DATA_DEBUG_MODE, value1bit & 1)); }

int DATA_get_crc(int hexbd, int skiroc)
{ 
  int indx, crc;
  indx = hexbd & 1;
  crc = 0;
  if (indx == 0) {
    if (skiroc == 0) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_SKIROC0);
    if (skiroc == 1) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_SKIROC1);
    if (skiroc == 2) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_SKIROC2);
    if (skiroc == 3) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_SKIROC3);
  }
  else {
    if (skiroc == 0) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_SKIROC0);
    if (skiroc == 1) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_SKIROC1);
    if (skiroc == 2) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_SKIROC2);
    if (skiroc == 3) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_SKIROC3);
  }
  return(crc);
}

int DATA_get_crc_fifo(int hexbd, int skiroc)
{ 
  int indx, crc;
  indx = hexbd & 1;
  crc = 0;
  if (indx == 0) {
    if (skiroc == 0) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_FIFO0);
    if (skiroc == 1) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_FIFO1);
    if (skiroc == 2) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_FIFO2);
    if (skiroc == 3) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_FIFO3);
  }
  else {
    if (skiroc == 0) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_FIFO0);
    if (skiroc == 1) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_FIFO1);
    if (skiroc == 2) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_FIFO2);
    if (skiroc == 3) crc = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_FIFO3);
  }
  return(crc);
}

int DATA_get_crc_fifo_empty(int hexbd)
{ 
  int indx, crc_empty;
  indx = hexbd & 1;
  if (indx == 0) crc_empty = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_FIFO_EMPTY);
  else crc_empty = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_FIFO_EMPTY);
  return(crc_empty);
}

int DATA_get_crc_fifo_enable(int hexbd)
{ 
  int indx, crc_enable;
  indx = hexbd & 1;
  if (indx == 0) crc_enable = spi_get_16bits((hexbd>>1)&3, DATA_CRC0_FIFO_ENABLE);
  else crc_enable = spi_get_16bits((hexbd>>1)&3, DATA_CRC1_FIFO_ENABLE);
  return(crc_enable);
}

int DATA_put_crc_fifo_enable(int hexbd, int value)
{ 
  int indx, status;
  indx = hexbd & 1;
  if (indx == 0) status = spi_put_16bits((hexbd>>1)&3, DATA_CRC0_FIFO_ENABLE, value);
  else status = spi_put_16bits((hexbd>>1)&3, DATA_CRC1_FIFO_ENABLE, value);
  return(status);
}

