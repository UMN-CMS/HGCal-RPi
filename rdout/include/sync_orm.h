#include "spi_common.h"

#define SYNC_RESET_PULSE      (0x100) // resets[15:0] { 12'h0, reset_waiting, reset_rdout, reset_trig_counter, reset_all}

#define SYNC_FIRMWARE_VERSION (0x101) // firmware_version[15:0]
#define SYNC_CONSTANT0        (0x102) // constant0[15:0] // 0xDEAD
#define SYNC_CONSTANT1        (0x103) // constant1[15:0] // 0xBEEF
#define SYNC_DUMMY_REG0       (0x104) // dummy_reg0[15:0] // writable, but does nothing
#define SYNC_DUMMY_REG1       (0x105) // dummy_reg1[15:0] // writable, but does nothing

#define SYNC_ENABLE_TRIG      (0x110) 
#define SYNC_MODE             (0x111) 
#define SYNC_RDOUT_MASK       (0x112) 
#define SYNC_WAITING_STATUS   (0x113) // waiting status
#define SYNC_RDOUT_DONE_HOLD  (0x114) // rdout_done[15:0] status
#define SYNC_DELAY_DONE       (0x115) 
#define SYNC_ENABLE_VETO      (0x116) 

#define SYNC_TRIG_COUNT0      (0x130) 
#define SYNC_TRIG_COUNT1      (0x131) 

#define SYNC_MAX_TRIG_COUNT0  (0x140) 
#define SYNC_MAX_TRIG_COUNT1  (0x141) 

#define SYNC_ENABLE_FAKE25    (0x150) 
#define SYNC_THRESH_FAKE25    (0x151) 
#define SYNC_MAX0_FAKE25      (0x152) 
#define SYNC_MAX1_FAKE25      (0x153) 

#define SYNC_ENABLE_FAKE40    (0x160) 
#define SYNC_THRESH_FAKE40    (0x161) 
#define SYNC_MAX0_FAKE40      (0x162) 
#define SYNC_MAX1_FAKE40      (0x163) 

#define SYNC_ENABLE_FAKE65    (0x170) 
#define SYNC_THRESH_FAKE65    (0x171) 
#define SYNC_MAX0_FAKE65      (0x172) 
#define SYNC_MAX1_FAKE65      (0x173) 

// Send reset pulses to the firmware.
int SYNC_reset_all(void);
int SYNC_reset_trig_counter(void);
int SYNC_reset_rdout(void);
int SYNC_reset_waiting(void);

// Get the 16-bit firmware version:
int SYNC_get_firmware_version(void);

// Read  two 16-bit constants that do 
// nothing. (for debug)
int SYNC_get_constant0(void);
int SYNC_get_constant1(void);

// Read and write two 16-bit registers that do 
// nothing. (for debug)
int SYNC_get_dummy0(void);
int SYNC_put_dummy0(int value16bits);
int SYNC_get_dummy1(void);
int SYNC_put_dummy1(int value16bits);

// Enable the trigger.
int SYNC_get_enable_trig(void);
int SYNC_put_enable_trig(int value1bit);

// Unused.
int SYNC_get_sync_mode(void);
int SYNC_put_sync_mode(int value1bit);

//===============================================
// Important!
//===============================================
// Set this 16-bit register to match the 16-bit
// mask of RDOUT boards that have an HDMI cable 
// connected to this SYNC board.
int SYNC_get_rdout_mask(void);
int SYNC_put_rdout_mask(int value16bits);

// Inspect the waiting_for_trigger signal.
int SYNC_get_waiting_status(void);

// Inspect the 16-bit mask which represents the 
// RDOUT boards that are done.
int SYNC_get_rdout_done_hold(void);

// The least significant 4 bits will choose the
// number of 40 MHz ticks to delay the rdout_done
// signal.
int SYNC_get_delay_done(void);
int SYNC_put_delay_done(int value16bits);

// Enable the veto signals.
int SYNC_get_enable_veto(void);
int SYNC_put_enable_veto(int value2bits);

// The 32-bit trigger counter.
int SYNC_get_trig_count0(void);
int SYNC_get_trig_count1(void);

// The 32-bit value of max_tig_count will determine
// when the triggers are automatically halted. A value
// of zero will disable this feature.
int SYNC_get_max_trig_count0(void);
int SYNC_get_max_trig_count1(void);
int SYNC_put_max_trig_count0(int value16bits);
int SYNC_put_max_trig_count1(int value16bits);

// Fake triggers can be generated based on the 25 MHz 
// clock. The two 16-bit numbers (max1_fake25 and 
// max0_fake25) divide the 40 MHz clock by some large 
// value to create the trigger rate. The 16-bit value 
// of thresh_fake25 will determine the pulse width of 
// the fake trigger.
int SYNC_get_enable_fake25(void);
int SYNC_put_enable_fake25(int value1bit);
int SYNC_get_thresh_fake25(void);
int SYNC_put_thresh_fake25(int value16bits);
int SYNC_get_max0_fake25(void);
int SYNC_get_max1_fake25(void);
int SYNC_put_max0_fake25(int value16bits);
int SYNC_put_max1_fake25(int value16bits);

// Fake triggers can be generated based on the 40 MHz 
// clock. The two 16-bit numbers (max1_fake40 and 
// max0_fake40) divide the 40 MHz clock by some large 
// value to create the trigger rate. The 16-bit value 
// of thresh_fake40 will determine the pulse width of 
// the fake trigger.
int SYNC_get_enable_fake40(void);
int SYNC_put_enable_fake40(int value1bit);
int SYNC_get_thresh_fake40(void);
int SYNC_put_thresh_fake40(int value16bits);
int SYNC_get_max0_fake40(void);
int SYNC_get_max1_fake40(void);
int SYNC_put_max0_fake40(int value16bits);
int SYNC_put_max1_fake40(int value16bits);

// Fake triggers can be generated based on a clock that 
// is completely asynchronous to the 40 MHz clock. It is 
// ROUGHLY 65 MHz, but has large variance. The two 16-bit 
// numbers (max1_fake65 and max0_fake65) divide the 65 MHz 
// clock by some large value to create the trigger rate. 
// The 16-bit value of thresh_fake65 will determine the 
// pulse width of the fake trigger.
int SYNC_get_enable_fake65(void);
int SYNC_put_enable_fake65(int value1bit);
int SYNC_get_thresh_fake65(void);
int SYNC_put_thresh_fake65(int value16bits);
int SYNC_get_max0_fake65(void);
int SYNC_get_max1_fake65(void);
int SYNC_put_max0_fake65(int value16bits);
int SYNC_put_max1_fake65(int value16bits);

// Get a list of HDMI connectors on the front panel that 
// have cables connected.
int SYNC_cables_connected(void);

