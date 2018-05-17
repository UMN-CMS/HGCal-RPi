#include "spi_common.h"

#define CTL_RESET_PULSE      (0x100) // resets[15:0] { 12'h0, reset_clk_count, reset_trig_count, reset_fifos, reset_all }

#define CTL_FIRMWARE_VERSION (0x101) // firmware_version[15:0]
#define CTL_CONSTANT0        (0x102) // constant0[15:0] // 0xDEAD
#define CTL_CONSTANT1        (0x103) // constant1[15:0] // 0xBEEF
#define CTL_DUMMY_REG0       (0x104) // dummy_reg0[15:0] // writable, but does nothing
#define CTL_DUMMY_REG1       (0x105) // dummy_reg1[15:0] // writable, but does nothing

#define CTL_DISABLE_DEBUG    (0x110) // 1-bit disables the automatic data dump into the skiroc fifos
#define CTL_MAC_ADDRESS0     (0x111)
#define CTL_MAC_ADDRESS1     (0x112)
#define CTL_MAC_ADDRESS2     (0x113)
#define CTL_IP_ADDRESS0      (0x114)
#define CTL_IP_ADDRESS1      (0x115)
#define CTL_SKIROC_MASK0     (0x116) // 16-bit active skirocs
#define CTL_SKIROC_MASK1     (0x117) // 16-bit active skirocs
#define CTL_DATE_STAMP0      (0x118) 
#define CTL_DATE_STAMP1      (0x119) 
#define CTL_BLOCK_SIZE       (0x11a) 
#define CTL_PEDESTAL_MODE    (0x11b) // 1-bit 
#define CTL_LEGACY_MODE      (0x11c) // 1-bit 
#define CTL_RDOUT_DONE       (0x11d) // 1-bit 

#define CTL_TRIG_COUNT0      (0x120) 
#define CTL_TRIG_COUNT1      (0x121)  
#define CTL_CLK_COUNT0       (0x123) 
#define CTL_CLK_COUNT1       (0x124) 
#define CTL_CLK_COUNT2       (0x125) 
#define CTL_BLOCK_READY      (0x126)
#define CTL_ALMOST_FULL      (0x128)
#define CTL_EMPTY            (0x12a)
#define CTL_FULL             (0x12b)
#define CTL_OCCUPANCY        (0x12c)
#define CTL_FIFO_0           (0x140)
#define CTL_FIFO_1           (0x141)

// IPbus address space

#define IPB_RATE20           (0x300)
#define IPB_RATE40           (0x301)
#define IPB_RATE80           (0x302)
#define IPB_RATE160          (0x303)
#define IPB_RATE320          (0x304)
#define IPB_RATE320b         (0x305)
#define IPB_RATE25           (0x306)
#define IPB_RATE50           (0x307)
#define IPB_RATE100          (0x308)
#define IPB_RATE125          (0x309)

#define IPB_FIRMWARE_VERSION (0x511) // firmware_version[15:0]
#define IPB_CONSTANT0        (0x512) // constant0[15:0] // 0xDEAD
#define IPB_CONSTANT1        (0x513) // constant1[15:0] // 0xBEEF

#define IPB_DISABLE_DEBUG    (0x514)
#define IPB_SKIROC_MASK      (0x515) 
#define IPB_DATE_STAMP       (0x516) 
#define IPB_BLOCK_SIZE       (0x517)
#define IPB_PEDESTAL_MODE    (0x518)
#define IPB_TRIG_COUNT       (0x519) 
#define IPB_CLK_COUNT0       (0x51b) 
#define IPB_CLK_COUNT1       (0x51c) 
#define IPB_BLOCK_READY      (0x51d)
#define IPB_ALMOST_FULL      (0x51e)
#define IPB_EMPTY            (0x51f)

#define IPB_FIFO             (0x520)

#define IPB_FULL             (0x563)
#define IPB_OCCUPANCY        (0x564)

#define IPB_RDOUT_DONE       (0x565)
#define RDOUT_DONE_MAGIC     (0xABCD4321) // match this to make a pulse

int CTL_reset_all(void);
int CTL_reset_fifos(void);
int CTL_reset_trig_count(void);
int CTL_reset_clk_count(void);
int CTL_get_firmware_version(void);
int CTL_get_constant0(void);
int CTL_get_constant1(void);
int CTL_get_dummy0(void);
int CTL_put_dummy0(int value16bits);
int CTL_get_dummy1(void);
int CTL_put_dummy1(int value16bits);
int CTL_get_disable_debug(void);
int CTL_put_disable_debug(int value1bit);
int CTL_get_mac_address0(void);
int CTL_put_mac_address0(int value16bits);
int CTL_get_mac_address1(void);
int CTL_put_mac_address1(int value16bits);
int CTL_get_mac_address2(void);
int CTL_put_mac_address2(int value16bits);
int CTL_get_ip_address0(void);
int CTL_put_ip_address0(int value16bits);
int CTL_get_ip_address1(void);
int CTL_put_ip_address1(int value16bits);
int CTL_get_skiroc_mask0(void);
int CTL_put_skiroc_mask0(int value16bits);
int CTL_get_skiroc_mask1(void);
int CTL_put_skiroc_mask1(int value16bits);
int CTL_get_date_stamp0(void);
int CTL_put_date_stamp0(int value16bits);
int CTL_get_date_stamp1(void);
int CTL_put_date_stamp1(int value16bits);
int CTL_get_block_size(void);
int CTL_put_block_size(int value16bits);
int CTL_get_pedestal_mode(void);
int CTL_put_pedestal_mode(int value1bit);
int CTL_get_legacy_mode(void);
int CTL_put_legacy_mode(int value1bit);
int CTL_put_done(void);
int CTL_get_trig_count0(void);
int CTL_get_trig_count1(void);
int CTL_get_clk_count0(void);
int CTL_get_clk_count1(void);
int CTL_get_clk_count2(void);
int CTL_get_block_ready(void);
int CTL_get_almost_full(void);
int CTL_get_empty(void);
int CTL_get_full(void);
int CTL_get_occupancy(void);
int CTL_get_fifo_LS16(void);
int CTL_get_fifo_MS16(void);
