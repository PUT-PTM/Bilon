//=====================================================================================================
// minIMU.h
//=====================================================================================================
#ifndef minIMU_h
#define minIMU_h

#include "stm32f4xx_conf.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_tim.h"

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "usb_dcd_int.h"

#define GYRO_ADDRESS 0xD2	 // adres zyroskopu
#define ACCEL_ADDRESS 0x30	 // adres akcelerometru
#define COMPASS_ADDRESS 0x3C // adres magnetometru

void configInit();
void I2C1_init(void);
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);
void I2C_stop(I2C_TypeDef* I2Cx);

void Read_Gyro();
void Read_Accel();
void Read_Compass();

extern int16_t AN_OFFSET[6];
extern int16_t AN[6];
extern int8_t SENSOR_SIGN[9];
extern int16_t gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z, magnetom_x, magnetom_y, magnetom_z;

#endif
