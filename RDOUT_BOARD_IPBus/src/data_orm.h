#include "spi_common.h"

#define DATA_RESET_PULSE      (0x100) // resets[15:0] { 4'h0, reset_crc1_fifo, reset_crc1, reset_local_fifo1, reset_all1, 
                                      //                4'h0, reset_crc0_fifo, reset_crc0, reset_local_fifo0, reset_all0 }

#define DATA_FIRMWARE_VERSION (0x101) // firmware_version[15:0]
#define DATA_CONSTANT0        (0x102) // constant0[15:0] // 0xDEAD
#define DATA_CONSTANT1        (0x103) // constant1[15:0] // 0xBEEF
#define DATA_DUMMY_REG0       (0x104) // dummy_reg0[15:0] // writable, but does nothing
#define DATA_DUMMY_REG1       (0x105) // dummy_reg1[15:0] // writable, but does nothing

#define DATA_TRIG_PULSE       (0x110) // trigger_pulse_spi // software generated trigger pulse
#define DATA_XFER_DISABLE     (0x111) // 2-bits, disable the transfer of data from hexaboards to CTL_ORM

#define DATA_CALIB_MODE       (0x120) // calib_mode // ignore external triggers, and prepare for calibration triggers
#define DATA_CALIB_PULSE      (0x121) // calib_pulse_spi //  software generated calibration trigger pulse
#define DATA_CALIB_DELAY_EN0  (0x122) // calib_delay_enable[15:0] // number of 40 MHz ticks to delay the calibration enable pulse
#define DATA_CALIB_FINE_DLY0  (0x123) // calib_fine_delay[15:0] // number of 320 MHz ticks to delay the calibration enable pulse
#define DATA_CALIB_DELAY_EN1  (0x124) // calib_delay_enable[15:0] // number of 40 MHz ticks to delay the calibration enable pulse
#define DATA_CALIB_FINE_DLY1  (0x125) // calib_fine_delay[15:0] // number of 320 MHz ticks to delay the calibration enable pulse
#define DATA_CALIB_DELAY_TRG  (0x126) // calib_delay_trigger[15:0] // number of 40 MHz ticks to delay the calibration trigger pulse
#define DATA_CALIB_AMPLITUDE  (0x127) // calib_amplitude[15:0] // counter value for ct_pwm threshold
#define DATA_PEDESTAL_MODE    (0x128) // ignore external triggers, and prepare for pedestal triggers
#define DATA_DEBUG_MODE       (0x129)

#define DATA_HEXA0_COMMAND    (0x130) // hexa0_command[15:0]
#define DATA_HEXA0_RESPONSE   (0x131) // hexa0_response[7:0]
#define DATA_HEXA0_STATUS     (0x132) // hexa0_status[15:0]
#define DATA_CRC0_SKIROC0     (0x133) // crc0_skiroc0[15:0]
#define DATA_CRC0_SKIROC1     (0x134) // crc0_skiroc1[15:0]
#define DATA_CRC0_SKIROC2     (0x135) // crc0_skiroc2[15:0]
#define DATA_CRC0_SKIROC3     (0x136) // crc0_skiroc3[15:0]
#define DATA_CRC0_FIFO0       (0x137) // crc0_fifo0[15:0] // 2K_X_16 fifo
#define DATA_CRC0_FIFO1       (0x138) // crc0_fifo1[15:0] // 2K_X_16 fifo
#define DATA_CRC0_FIFO2       (0x139) // crc0_fifo2[15:0] // 2K_X_16 fifo
#define DATA_CRC0_FIFO3       (0x13A) // crc0_fifo3[15:0] // 2K_X_16 fifo
#define DATA_CRC0_FIFO_EMPTY  (0x13B) // crc0_fifo_emty[3:0]
#define DATA_CRC0_FIFO_ENABLE (0x13C) // crc0_fifo_enable

#define DATA_HEXA1_COMMAND    (0x140) // hexa1_command[15:0]
#define DATA_HEXA1_RESPONSE   (0x141) // hexa1_response[7:0]
#define DATA_HEXA1_STATUS     (0x142) // hexa1_status[15:0]
#define DATA_CRC1_SKIROC0     (0x143) // crc1_skiroc0[15:0]
#define DATA_CRC1_SKIROC1     (0x144) // crc1_skiroc1[15:0]
#define DATA_CRC1_SKIROC2     (0x145) // crc1_skiroc2[15:0]
#define DATA_CRC1_SKIROC3     (0x146) // crc1_skiroc3[15:0]
#define DATA_CRC1_FIFO0       (0x147) // crc1_fifo0[15:0] // 2K_X_16 fifo
#define DATA_CRC1_FIFO1       (0x148) // crc1_fifo1[15:0] // 2K_X_16 fifo
#define DATA_CRC1_FIFO2       (0x149) // crc1_fifo2[15:0] // 2K_X_16 fifo
#define DATA_CRC1_FIFO3       (0x14A) // crc1_fifo3[15:0] // 2K_X_16 fifo
#define DATA_CRC1_FIFO_EMPTY  (0x14B) // crc1_fifo_emty[3:0]
#define DATA_CRC1_FIFO_ENABLE (0x14C) // crc1_fifo_enable

int DATA_reset_crc_fifo(int hexbd);
int DATA_reset_crc(int hexbd);
int DATA_reset_local_fifo(int hexbd);
int DATA_reset_all(int hexbd);
int DATA_get_firmware_version(int orm);
int DATA_get_constant0(int orm);
int DATA_get_constant1(int orm);
int DATA_get_dummy0(int orm);
int DATA_put_dummy0(int orm, int value16bits);
int DATA_get_dummy1(int orm);
int DATA_put_dummy1(int orm, int value16bits);
int DATA_put_trigger_pulse(int orm);
int DATA_get_xfer_disable(int orm);
int DATA_put_xfer_disable(int orm, int value2bits);
int DATA_get_calib_mode(int orm);
int DATA_put_calib_mode(int orm, int value1bit);
int DATA_put_calib_pulse(int orm);
int DATA_get_calib_delay_en0(int orm);
int DATA_put_calib_delay_en0(int orm, int value16bits);
int DATA_get_calib_fine_dly0(int orm);
int DATA_put_calib_fine_dly0(int orm, int value16bits);
int DATA_get_calib_delay_en1(int orm);
int DATA_put_calib_delay_en1(int orm, int value16bits);
int DATA_get_calib_fine_dly1(int orm);
int DATA_put_calib_fine_dly1(int orm, int value16bits);
int DATA_get_calib_delay_trig(int orm);
int DATA_put_calib_delay_trig(int orm, int value16bits);
int DATA_get_calib_amplitude(int orm);
int DATA_put_calib_amplitude(int orm, int value16bits);
int DATA_get_pedestal_mode(int orm);
int DATA_put_pedestal_mode(int orm, int value1bit);
int DATA_get_debug_mode(int orm);
int DATA_put_debug_mode(int orm, int value1bit);
int DATA_get_crc(int hexbd, int skiroc);
int DATA_get_crc_fifo(int hexbd, int skiroc);
int DATA_get_crc_fifo_empty(int hexbd);
int DATA_get_crc_fifo_enable(int hexbd);
int DATA_put_crc_fifo_enable(int hexbd, int value);

int HEXBD_send_generic(int hexbd, int rw, int a2, int a1, int a0, char c);
int HEXBD_read_generic(int hexbd, int a2, int a1, int a0);
int HEXBD_queue_status(int hexbd);
int HEXBD_send_command(int hexbd, char c);
int HEXBD_read_command(int hexbd);
int HEXBD_write_local_fifo(int hexbd, char c);
int HEXBD_read_local_fifo(int hexbd);
int HEXBD_read_usedwh(int hexbd);
int HEXBD_read_usedwl(int hexbd);
int HEXBD_trigger_gen(int hexbd);
int HEXBD_read200_local_fifo(int hexbd, int result[200]);
int HEXBD_read1000_local_fifo(int hexbd, int result[1000]);
int HEXBD_verify_communication(int verbose);
