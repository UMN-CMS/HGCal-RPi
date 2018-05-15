#include "ctl_orm.h"

int CTL_reset_all(void)
{ // { 12'h0, reset_clk_count, reset_trig_count, reset_fifos, reset_all }
  return(spi_put_16bits(4, CTL_RESET_PULSE, 1<<0)); 
}

int CTL_reset_fifos(void)
{ // { 12'h0, reset_clk_count, reset_trig_count, reset_fifos, reset_all }
  return(spi_put_16bits(4, CTL_RESET_PULSE, 1<<1)); 
}

int CTL_reset_trig_count(void)
{ // { 12'h0, reset_clk_count, reset_trig_count, reset_fifos, reset_all }
  return(spi_put_16bits(4, CTL_RESET_PULSE, 1<<2)); 
}

int CTL_reset_clk_count(void)
{ // { 12'h0, reset_clk_count, reset_trig_count, reset_fifos, reset_all }
  return(spi_put_16bits(4, CTL_RESET_PULSE, 1<<3)); 
}

int CTL_get_firmware_version(void)
{ return(spi_get_16bits(4, CTL_FIRMWARE_VERSION)); }

int CTL_get_constant0(void)
{ return(spi_get_16bits(4, CTL_CONSTANT0)); }

int CTL_get_constant1(void)
{ return(spi_get_16bits(4, CTL_CONSTANT1)); }

int CTL_get_dummy0(void)
{ return(spi_get_16bits(4, CTL_DUMMY_REG0)); }

int CTL_put_dummy0(int value16bits)
{ return(spi_put_16bits(4, CTL_DUMMY_REG0, value16bits)); }

int CTL_get_dummy1(void)
{ return(spi_get_16bits(4, CTL_DUMMY_REG1)); }

int CTL_put_dummy1(int value16bits)
{ return(spi_put_16bits(4, CTL_DUMMY_REG1, value16bits)); }

int CTL_get_disable_debug(void)
{ return(spi_get_16bits(4, CTL_DISABLE_DEBUG)); }

int CTL_put_disable_debug(int value1bit)
{ return(spi_put_16bits(4, CTL_DISABLE_DEBUG, value1bit)); }

int CTL_get_mac_address0(void)
{ return(spi_get_16bits(4, CTL_MAC_ADDRESS0)); }

int CTL_put_mac_address0(int value16bits)
{ return(spi_put_16bits(4, CTL_MAC_ADDRESS0, value16bits)); }

int CTL_get_mac_address1(void)
{ return(spi_get_16bits(4, CTL_MAC_ADDRESS1)); }

int CTL_put_mac_address1(int value16bits)
{ return(spi_put_16bits(4, CTL_MAC_ADDRESS1, value16bits)); }

int CTL_get_mac_address2(void)
{ return(spi_get_16bits(4, CTL_MAC_ADDRESS2)); }

int CTL_put_mac_address2(int value16bits)
{ return(spi_put_16bits(4, CTL_MAC_ADDRESS2, value16bits)); }

int CTL_get_ip_address0(void)
{ return(spi_get_16bits(4, CTL_IP_ADDRESS0)); }

int CTL_put_ip_address0(int value16bits)
{ return(spi_put_16bits(4, CTL_IP_ADDRESS0, value16bits)); }

int CTL_get_ip_address1(void)
{ return(spi_get_16bits(4, CTL_IP_ADDRESS1)); }

int CTL_put_ip_address1(int value16bits)
{ return(spi_put_16bits(4, CTL_IP_ADDRESS1, value16bits)); }

int CTL_get_skiroc_mask0(void)
{ return(spi_get_16bits(4, CTL_SKIROC_MASK0)); }

int CTL_put_skiroc_mask0(int value16bits)
{ return(spi_put_16bits(4, CTL_SKIROC_MASK0, value16bits)); }

int CTL_get_skiroc_mask1(void)
{ return(spi_get_16bits(4, CTL_SKIROC_MASK1)); }

int CTL_put_skiroc_mask1(int value16bits)
{ return(spi_put_16bits(4, CTL_SKIROC_MASK1, value16bits)); }

int CTL_get_date_stamp0(void)
{ return(spi_get_16bits(4, CTL_DATE_STAMP0)); }

int CTL_put_date_stamp0(int value16bits)
{ return(spi_put_16bits(4, CTL_DATE_STAMP0, value16bits)); }

int CTL_get_date_stamp1(void)
{ return(spi_get_16bits(4, CTL_DATE_STAMP1)); }

int CTL_put_date_stamp1(int value16bits)
{ return(spi_put_16bits(4, CTL_DATE_STAMP1, value16bits)); }

int CTL_get_block_size(void)
{ return(spi_get_16bits(4, CTL_BLOCK_SIZE)); }

int CTL_put_block_size(int value16bits)
{ return(spi_put_16bits(4, CTL_BLOCK_SIZE, value16bits)); }

int CTL_get_pedestal_mode(void)
{ return(spi_get_16bits(4, CTL_PEDESTAL_MODE)); }

int CTL_put_pedestal_mode(int value1bit)
{ return(spi_put_16bits(4, CTL_PEDESTAL_MODE, value1bit)); }

int CTL_get_legacy_mode(void)
{ return(spi_get_16bits(4, CTL_LEGACY_MODE)); }

int CTL_put_legacy_mode(int value1bit)
{ return(spi_put_16bits(4, CTL_LEGACY_MODE, value1bit)); }

int CTL_put_done(void)
{ return(spi_put_16bits(4, CTL_RDOUT_DONE, 0xEFEF)); }

int CTL_get_trig_count0(void)
{ return(spi_get_16bits(4, CTL_TRIG_COUNT0)); }

int CTL_get_trig_count1(void)
{ return(spi_get_16bits(4, CTL_TRIG_COUNT1)); }

int CTL_get_clk_count0(void)
{ return(spi_get_16bits(4, CTL_CLK_COUNT0)); }

int CTL_get_clk_count1(void)
{ return(spi_get_16bits(4, CTL_CLK_COUNT1)); }

int CTL_get_clk_count2(void)
{ return(spi_get_16bits(4, CTL_CLK_COUNT2)); }

int CTL_get_block_ready0(void)
{ return(spi_get_16bits(4, CTL_BLOCK_READY0)); }

int CTL_get_block_ready1(void)
{ return(spi_get_16bits(4, CTL_BLOCK_READY1)); }

int CTL_get_half_empty0(void)
{ return(spi_get_16bits(4, CTL_HALF_EMPTY0)); }

int CTL_get_half_empty1(void)
{ return(spi_get_16bits(4, CTL_HALF_EMPTY1)); }

int CTL_get_empty0(void)
{ return(spi_get_16bits(4, CTL_EMPTY0)); }

int CTL_get_empty1(void)
{ return(spi_get_16bits(4, CTL_EMPTY1)); }

int CTL_get_fifo_LS16(int index)
{ return(spi_get_16bits(4, CTL_FIFO_00_0 + (2*index))); }

int CTL_get_fifo_MS16(int index)
{ return(spi_get_16bits(4, CTL_FIFO_00_1 + (2*index))); }

