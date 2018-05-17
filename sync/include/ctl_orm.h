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
#define CTL_BLOCK_READY0     (0x126)
#define CTL_BLOCK_READY1     (0x127)
#define CTL_HALF_EMPTY0      (0x128)
#define CTL_HALF_EMPTY1      (0x129)
#define CTL_EMPTY0           (0x12a)
#define CTL_EMPTY1           (0x12b)

#define CTL_FIFO_00_0        (0x140)
#define CTL_FIFO_00_1        (0x141)
#define CTL_FIFO_01_0        (0x142)
#define CTL_FIFO_01_1        (0x143)
#define CTL_FIFO_02_0        (0x144)
#define CTL_FIFO_02_1        (0x145)
#define CTL_FIFO_03_0        (0x146)
#define CTL_FIFO_03_1        (0x147)
#define CTL_FIFO_04_0        (0x148)
#define CTL_FIFO_04_1        (0x149)
#define CTL_FIFO_05_0        (0x14a)
#define CTL_FIFO_05_1        (0x14b)
#define CTL_FIFO_06_0        (0x14c)
#define CTL_FIFO_06_1        (0x14d)
#define CTL_FIFO_07_0        (0x14e)
#define CTL_FIFO_07_1        (0x14f)
#define CTL_FIFO_08_0        (0x150)
#define CTL_FIFO_08_1        (0x151)
#define CTL_FIFO_09_0        (0x152)
#define CTL_FIFO_09_1        (0x153)
#define CTL_FIFO_10_0        (0x154)
#define CTL_FIFO_10_1        (0x155)
#define CTL_FIFO_11_0        (0x156)
#define CTL_FIFO_11_1        (0x157)
#define CTL_FIFO_12_0        (0x158)
#define CTL_FIFO_12_1        (0x159)
#define CTL_FIFO_13_0        (0x15a)
#define CTL_FIFO_13_1        (0x15b)
#define CTL_FIFO_14_0        (0x15c)
#define CTL_FIFO_14_1        (0x15d)
#define CTL_FIFO_15_0        (0x15e)
#define CTL_FIFO_15_1        (0x15f)
#define CTL_FIFO_16_0        (0x160)
#define CTL_FIFO_16_1        (0x161)
#define CTL_FIFO_17_0        (0x162)
#define CTL_FIFO_17_1        (0x163)
#define CTL_FIFO_18_0        (0x164)
#define CTL_FIFO_18_1        (0x165)
#define CTL_FIFO_19_0        (0x166)
#define CTL_FIFO_19_1        (0x167)
#define CTL_FIFO_20_0        (0x168)
#define CTL_FIFO_20_1        (0x169)
#define CTL_FIFO_21_0        (0x16a)
#define CTL_FIFO_21_1        (0x16b)
#define CTL_FIFO_22_0        (0x16c)
#define CTL_FIFO_22_1        (0x16d)
#define CTL_FIFO_23_0        (0x16e)
#define CTL_FIFO_23_1        (0x16f)
#define CTL_FIFO_24_0        (0x170)
#define CTL_FIFO_24_1        (0x171)
#define CTL_FIFO_25_0        (0x172)
#define CTL_FIFO_25_1        (0x173)
#define CTL_FIFO_26_0        (0x174)
#define CTL_FIFO_26_1        (0x175)
#define CTL_FIFO_27_0        (0x176)
#define CTL_FIFO_27_1        (0x177)
#define CTL_FIFO_28_0        (0x178)
#define CTL_FIFO_28_1        (0x179)
#define CTL_FIFO_29_0        (0x17a)
#define CTL_FIFO_29_1        (0x17b)
#define CTL_FIFO_30_0        (0x17c)
#define CTL_FIFO_30_1        (0x17d)
#define CTL_FIFO_31_0        (0x17e)
#define CTL_FIFO_31_1        (0x17f)

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
#define IPB_HALF_EMPTY       (0x51e)
#define IPB_EMPTY            (0x51f)

#define IPB_FIFO_00          (0x520)
#define IPB_FIFO_01          (0x521)
#define IPB_FIFO_02          (0x522)
#define IPB_FIFO_03          (0x523)
#define IPB_FIFO_04          (0x524)
#define IPB_FIFO_05          (0x525)
#define IPB_FIFO_06          (0x526)
#define IPB_FIFO_07          (0x527)
#define IPB_FIFO_08          (0x528)
#define IPB_FIFO_09          (0x529)
#define IPB_FIFO_10          (0x52a)
#define IPB_FIFO_11          (0x52b)
#define IPB_FIFO_12          (0x52c)
#define IPB_FIFO_13          (0x52d)
#define IPB_FIFO_14          (0x52e)
#define IPB_FIFO_15          (0x52f)
#define IPB_FIFO_16          (0x530)
#define IPB_FIFO_17          (0x531)
#define IPB_FIFO_18          (0x532)
#define IPB_FIFO_19          (0x533)
#define IPB_FIFO_20          (0x534)
#define IPB_FIFO_21          (0x535)
#define IPB_FIFO_22          (0x536)
#define IPB_FIFO_23          (0x537)
#define IPB_FIFO_24          (0x538)
#define IPB_FIFO_25          (0x539)
#define IPB_FIFO_26          (0x53a)
#define IPB_FIFO_27          (0x53b)
#define IPB_FIFO_28          (0x53c)
#define IPB_FIFO_29          (0x53d)
#define IPB_FIFO_30          (0x53e)
#define IPB_FIFO_31          (0x53f)

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
int CTL_get_block_ready0(void);
int CTL_get_block_ready1(void);
int CTL_get_half_empty0(void);
int CTL_get_half_empty1(void);
int CTL_get_empty0(void);
int CTL_get_empty1(void);
int CTL_get_fifo_LS16(int index);
int CTL_get_fifo_MS16(int index);
