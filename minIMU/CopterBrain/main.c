#include <math.h>
#include "stm32f4xx_conf.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_tim.h"
#include "MadgwickAHRS.h"

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "usb_dcd_int.h"

#define GYRO_ADDRESS 0xD2	 // adres zyroskopu
#define ACCEL_ADDRESS 0x30	 // adres akcelerometru
#define COMPASS_ADDRESS 0x3C // adres magnetometru
#define GRAVITY 256  //this equivalent to 1G in the raw data coming from the accelerometer
//---------------------------------------------------------

__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev __ALIGN_END;

//---------------------------------------------------------

uint8_t gx, gy, gz, ax, ay, az, mx, my, mz, theByte;
float Roll, Pitch, Yaw;

// !-!
uint8_t SENSOR_SIGN[9] = { 1, 1, 1, -1, -1, -1, 1, 1, 1 }; //Correct directions x,y,z - gyro, accelerometer, magnetometer

// L3G4200D gyro: 2000 dps full scale
// 70 mdps/digit; 1 dps = 0.07
#define Gyro_Gain_X 0.07 //X axis Gyro gain
#define Gyro_Gain_Y 0.07 //Y axis Gyro gain
#define Gyro_Gain_Z 0.07 //Z axis Gyro gain
#define Gyro_Scaled_X(x) ((x)*ToRad(Gyro_Gain_X)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Y(x) ((x)*ToRad(Gyro_Gain_Y)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Z(x) ((x)*ToRad(Gyro_Gain_Z)) //Return the scaled ADC raw data of the gyro in radians for second
// LSM303 magnetometer calibration constants; use the Calibrate example from
// the Pololu LSM303 library to find the right values for your board
#define M_X_MIN -602
#define M_Y_MIN -627
#define M_Z_MIN -511
#define M_X_MAX 370
#define M_Y_MAX 356
#define M_Z_MAX 439

#define Kp_ROLLPITCH 0.02
#define Ki_ROLLPITCH 0.00002
#define Kp_YAW 1.2
#define Ki_YAW 0.00002

/*For debugging purposes*/
//OUTPUTMODE=1 will print the corrected data,
//OUTPUTMODE=0 will print uncorrected data of the gyros (with drift)
#define OUTPUTMODE 1

//#define PRINT_DCM 0     //Will print the whole direction cosine matrix
#define PRINT_ANALOGS 0 //Will print the analog raw data
#define PRINT_EULER 1   //Will print the Euler angles Roll, Pitch and Yaw
#define STATUS_LED 13

float G_Dt = 0.02; // Integration time (DCM algorithm)  We will run the integration loop at 50Hz if possible

long timer = 0; //general purpuse timer
long timer_old;
long timer24 = 0; //Second timer used to print values
int AN[6]; //array that stores the gyro and accelerometer data
int AN_OFFSET[6] = { 0, 0, 0, 0, 0, 0 }; //Array that stores the Offset of the sensors

int gyro_x;
int gyro_y;
int gyro_z;
int accel_x;
int accel_y;
int accel_z;
int magnetom_x;
int magnetom_y;
int magnetom_z;
float c_magnetom_x;
float c_magnetom_y;
float c_magnetom_z;
float MAG_Heading;

float Accel_Vector[3] = { 0, 0, 0 }; //Store the acceleration in a vector
float Gyro_Vector[3] = { 0, 0, 0 }; //Store the gyros turn rate in a vector
float Omega_Vector[3] = { 0, 0, 0 }; //Corrected Gyro_Vector data
float Omega_P[3] = { 0, 0, 0 }; //Omega Proportional correction
float Omega_I[3] = { 0, 0, 0 }; //Omega Integrator
float Omega[3] = { 0, 0, 0 };

// Euler angles
float roll;
float pitch;
float yaw;

float errorRollPitch[3] = { 0, 0, 0 };
float errorYaw[3] = { 0, 0, 0 };

unsigned int counter = 0;
short gyro_sat = 0;

float DCM_Matrix[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
float Update_Matrix[3][3] = { { 0, 1, 2 }, { 3, 4, 5 }, { 6, 7, 8 } }; //Gyros here

float Temporary_Matrix[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

void I2C1_init(void);
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);
void I2C_stop(I2C_TypeDef* I2Cx);

void Read_Gyro();
void Read_Accel();
void Read_Compass();

float deg2rad(float degrees);
float rad2deg(float radians);
void configInit();

void SysTick_Handler(void) {
	// clock liczy
	Read_Gyro();
	Read_Accel();
	Read_Compass();
	if (!VCP_get_char(&theByte)) {

		VCP_send_str("!ANG:22,44,11\n");
	}
}
//--------------------------------------------
int main(void) {
	SystemInit();
	SystemCoreClockUpdate();

	I2C1_init();
	// Init Accel Compass Gyro
	configInit();

	// Przerwanie 200 x na sec
	SysTick_Config(SystemCoreClock / 200);

	unsigned int i;
	for (;;) {

		GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
		for (i = 0; i < 1000000; i++)
			;
	}

}

void I2C1_init(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;

	// enable APB1 peripheral clock for I2C1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	// enable clock for SCL and SDA pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* setup SCL and SDA pins
	 * You can connect the I2C1 functions to two different
	 * pins:
	 * 1. SCL on PB6 or PB8
	 * 2. SDA on PB7 or PB9
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // we are going to use PB6 and PB7
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; // set pins to alternate function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // set GPIO speed
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD; // set output to open drain --> the line has to be only pulled low, not driven high
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; // enable pull up resistors
	GPIO_Init(GPIOB, &GPIO_InitStruct); // init GPIOB

	// Connect I2C1 pins to AF
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1); // SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA

	// configure I2C1
	I2C_InitStruct.I2C_ClockSpeed = 400000; // 400kHz
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C; // I2C mode
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2; // 50% duty cycle --> standard
	I2C_InitStruct.I2C_OwnAddress1 = 0x00; // own address, not relevant in master mode
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable; // disable acknowledge when reading (can be changed later on)
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
	I2C_Init(I2C1, &I2C_InitStruct); // init I2C1

	// enable I2C1
	I2C_Cmd(I2C1, ENABLE);
}

/* This function issues a start condition and
 * transmits the slave address + R/W bit
 *
 * Parameters:
 * 		I2Cx --> the I2C peripheral e.g. I2C1
 * 		address --> the 7 bit slave address
 * 		direction --> the transmission direction can be:
 * 						I2C_Direction_Tranmitter for Master transmitter mode
 * 						I2C_Direction_Receiver for Master receiver
 */
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction) {
	// wait until I2C1 is not busy any more
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
		;

	// Send I2C1 START condition
	I2C_GenerateSTART(I2Cx, ENABLE);

	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
		;

	// Send slave Address for write
	I2C_Send7bitAddress(I2Cx, address, direction);

	/* wait for I2Cx EV6, check if
	 * either Slave has acknowledged Master transmitter or
	 * Master receiver mode, depending on the transmission
	 * direction
	 */
	if (direction == I2C_Direction_Transmitter) {
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
			;
	} else if (direction == I2C_Direction_Receiver) {
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
			;
	}
}

/* This function transmits one byte to the slave device
 * Parameters:
 *		I2Cx --> the I2C peripheral e.g. I2C1
 *		data --> the data byte to be transmitted
 */
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data) {
	I2C_SendData(I2Cx, data);
	// wait for I2C1 EV8 --> last byte is still being transmitted (last byte in SR, buffer empty), next byte can already be written
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING))
		;
}

/* This function reads one byte from the slave device
 * and acknowledges the byte (requests another byte)
 */
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx) {
	// enable acknowledge of received data
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	// wait until one byte has been received
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
		;
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This function reads one byte from the slave device
 * and doesn't acknowledge the received data
 * after that a STOP condition is transmitted
 */
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx) {
	// disable acknowledge of received data
	// nack also generates stop condition after last byte received
	// see reference manual for more info
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait until one byte has been received
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
		;
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This function issues a stop condition and therefore
 * releases the bus
 */
void I2C_stop(I2C_TypeDef* I2Cx) {

	// Send I2C1 STOP Condition after last byte has been transmitted
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait for I2C1 EV8_2 --> byte has been transmitted
	//while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

// minIMU
void Read_Gyro() {
	I2C_start(I2C1, GYRO_ADDRESS, I2C_Direction_Transmitter);
	I2C_write(I2C1, 0x28 | (1 << 7)); //L3G_OUT_X_L   0x28
	I2C_stop(I2C1);
	I2C_start(I2C1, 0xD3, I2C_Direction_Receiver);

	uint8_t xlg = I2C_read_ack(I2C1);
	uint8_t xhg = I2C_read_ack(I2C1);
	uint8_t ylg = I2C_read_ack(I2C1);
	uint8_t yhg = I2C_read_ack(I2C1);
	uint8_t zlg = I2C_read_ack(I2C1);
	uint8_t zhg = I2C_read_nack(I2C1);

	//I2C_stop(I2C1);

	gx = (int16_t)(xhg << 8 | xlg);
	gy = (int16_t)(yhg << 8 | ylg);
	gz = (int16_t)(zhg << 8 | zlg);
}

void Read_Accel() {
	I2C_start(I2C1, ACCEL_ADDRESS, I2C_Direction_Transmitter);
	I2C_write(I2C1, 0x28 | (1 << 7));
	I2C_stop(I2C1);
	I2C_start(I2C1, 0x31, I2C_Direction_Receiver);

	uint8_t xla = I2C_read_ack(I2C1);
	uint8_t xha = I2C_read_ack(I2C1);
	uint8_t yla = I2C_read_ack(I2C1);
	uint8_t yha = I2C_read_ack(I2C1);
	uint8_t zla = I2C_read_ack(I2C1);
	uint8_t zha = I2C_read_nack(I2C1);

	//I2C_stop(I2C1);
	// 12 bitowa odpowiedz (przesuniecie o 4 bit w  prawo)
	ax = (int16_t)((xha << 8 | xla)) >> 4;
	ay = (int16_t)((yha << 8 | yla)) >> 4;
	az = (int16_t)((zha << 8 | zla)) >> 4;
}

void Read_Compass() {
	I2C_start(I2C1, COMPASS_ADDRESS, I2C_Direction_Transmitter);
	I2C_write(I2C1, 0x03); //LSM303_OUT_X_H_M    0x03
	I2C_stop(I2C1);
	I2C_start(I2C1, 0x3D, I2C_Direction_Receiver);

	// Kolejnosc kosmos datasheet
	uint8_t xhm = I2C_read_ack(I2C1);
	uint8_t xlm = I2C_read_ack(I2C1);
	uint8_t zhm = I2C_read_ack(I2C1);
	uint8_t zlm = I2C_read_ack(I2C1);
	uint8_t yhm = I2C_read_ack(I2C1);
	uint8_t ylm = I2C_read_nack(I2C1);

	//I2C_stop(I2C1);
	mx = (int16_t)(xhm << 8 | xlm);
	my = (int16_t)(yhm << 8 | ylm);
	mz = (int16_t)(zhm << 8 | zlm);
}

void configInit() {
	// USB  -> VCP
	USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb,
			&USR_cb);

	// GPIO
	/* GPIOD Periph clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
			| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// MinImu
	//zyroskop (Gyro)
	I2C_start(I2C1, GYRO_ADDRESS, I2C_Direction_Transmitter);
	I2C_write(I2C1, 0x20); // L3G_CTRL_REG1 0x20
	I2C_write(I2C1, 0xBF); // 0x0F = 0b00001111
						   // ODR 100Hz Cut-off 12.5
						   // Normal power mode, all axes enabled

	I2C_stop(I2C1);
	I2C_start(I2C1, GYRO_ADDRESS, I2C_Direction_Transmitter);

	I2C_write(I2C1, 0x23); // CTRL_REG4
	I2C_write(I2C1, 0x10);

	I2C_stop(I2C1);

	//akcelerometr (Accel)
	I2C_start(I2C1, ACCEL_ADDRESS, I2C_Direction_Transmitter);

	I2C_write(I2C1, 0x20); // LSM303_CTRL_REG1_A  0x20
	I2C_write(I2C1, 0x37); // Enable Accelerometer
						   // 0x27 = 0b00100111
						   // ODR 50Hz Cut-off 37
						   // Normal power mode, all axes enabled
	I2C_stop(I2C1);
	I2C_start(I2C1, ACCEL_ADDRESS, I2C_Direction_Transmitter);

	I2C_write(I2C1, 0x23);
	I2C_write(I2C1, 0x30);

	I2C_stop(I2C1);

	//magnetometr (Compass)
	I2C_start(I2C1, COMPASS_ADDRESS, I2C_Direction_Transmitter);

	I2C_write(I2C1, 0x00);
	I2C_write(I2C1, 0x20);

	I2C_stop(I2C1);
	I2C_start(I2C1, COMPASS_ADDRESS, I2C_Direction_Transmitter);

	I2C_write(I2C1, 0x01);
	I2C_write(I2C1, 0x20);

	I2C_stop(I2C1);
	I2C_start(I2C1, COMPASS_ADDRESS, I2C_Direction_Transmitter);

	I2C_write(I2C1, 0x02); //LSM303_MR_REG_M   0x02
	I2C_write(I2C1, 0x00); // Enable Magnetometer
						   // 0x00 = 0b00000000
						   // Continuous conversion mode
	I2C_stop(I2C1);
}

float deg2rad(float degrees) {
	return (float) (M_PI / 180) * degrees;
}

float rad2deg(float radians) {
	return (radians * 180) / (float) M_PI;
}

void OTG_FS_IRQHandler(void) {
	USBD_OTG_ISR_Handler(&USB_OTG_dev);
}
