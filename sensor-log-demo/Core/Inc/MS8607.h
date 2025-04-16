#ifndef _MS8607_H
#define _MS8607_H

#include "stm32wlxx_ll_i2c.h"

#define MS8607_ADDR 0x76
#define MS8607_RESET_CMD 0x1E

#define MS8607_READ_C1_CMD 0xA2
#define MS8607_READ_C2_CMD 0xA4
#define MS8607_READ_C3_CMD 0xA6
#define MS8607_READ_C4_CMD 0xA8
#define MS8607_READ_C5_CMD 0xAA
#define MS8607_READ_C6_CMD 0xAC
#define MS8607_READ_CRC_CMD 0xAE

#define MS8607_PRESS_CONV_CMD 0x48 // OSR = 4096
#define MS8607_TEMP_CONV_CMD 0x58  // OSR = 4096
#define MS8607_READ_CMD 0x00

#define MS8607_RH_ADDR 0x40
#define MS807_RH_RESET_CMD 0xFE
#define MS807_RH_WRITE_CMD 0xE6
#define MS807_RH_READ_CMD 0xE7
#define MS807_RH_MEAS_HOLD_CMD 0xE5
#define MS807_RH_MEAS_NO_HOLD_CMD 0xF5

int MS8607_init(I2C_TypeDef *i2cx);
int MS8607_get_press_temp(uint32_t *pressure, int32_t *temperature);
int MS8607_get_rel_humidity(int32_t *rel_humidity, int32_t temperature);
int MS8607_reset(void);

#endif // _MS8607_H
