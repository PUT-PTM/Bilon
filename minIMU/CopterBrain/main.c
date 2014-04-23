#include <math.h>
#include "stm32f4xx_conf.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "MadgwickAHRS.h"

#define GYRO_ADDRESS 0xD2	 // adres zyroskopu
#define ACCEL_ADDRESS 0x30	 // adres akcelerometru
#define COMPASS_ADDRESS 0x3C // adres magnetometru

// i co z tymi globalsami
int gx,gy,gz,ax,ay,az,mx,my,mz;
float  Roll, Pitch, Yaw;

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
/*
        double a_xBias = 32634.2779917682;            // accelerometer bias
        double a_yBias = 32300.1140276867;
        double a_zBias = 32893.0853282136;
        double a_xGain = -0.00150042985864975;        // accelerometer gains
        double a_yGain = -0.00147414192905898;
        double a_zGain = 0.00152294825926844;
        double w_xBias = 25247;                             // gyroscope bias
        double w_yBias = 25126;
        double w_zBias = 24463;
        double w_xGain = 0.00102058528925813;         // gyroscope gains
        double w_yGain = -0.00110455853342484;
        double w_zGain = 0.00107794298635984;
        double m_xBias = -8.20750399495073;           // magnetometer baises
        double m_yBias = 15.6531909021474;
        double m_zBias = 7.32498941411782;
        double m_xGain = -0.00160372297752976;        // magnetometer gains
        double m_yGain = 0.0016037818986323;
        double m_zGain = 0.00182483736430979;
*/
void SysTick_Handler(void)
{
	Read_Gyro();
	Read_Accel();
	Read_Compass();
/*
	gx = (gx - w_xBias) * w_xGain;
	gy = (gy - w_yBias) * w_yGain;
	gz = (gz - w_zBias) * w_zGain;

	ax = (ax - a_xBias) * a_xGain;
	ay = (ay - a_yBias) * a_yGain;
	az = (az - a_zBias) * a_zGain;

	mx = (mx - m_xBias) * m_xGain;
	my = (my - m_yBias) * m_yGain;
	mz = (mz - m_zBias) * m_zGain;
*/
	MadgwickAHRSupdate(deg2rad(gx), deg2rad(gy), deg2rad(gz), ax, ay, az, mx, my, mz);

    float q12 =q1 *q1;
    float q22 =q2 *q2;
    float q32 =q3 *q3;

    Roll = rad2deg((float)atan2(2 * (q2 *q3 +q0 *q1), (1 - 2 * (q12 + q22))));
    Pitch = rad2deg((float)-asin(2 * (q1 *q3 -q0 *q2)));
    Yaw = rad2deg((float)atan2(2 * (q1 *q2 +q0 *q3), (1 - 2 * (q22 + q32))));
}

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();

	I2C1_init();

	/* GPIOD Periph clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef  GPIO_InitStructure;
	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

		//zyroskop (Gyro)
		I2C_start(I2C1, GYRO_ADDRESS, I2C_Direction_Transmitter);
			I2C_write(I2C1, 0x20);   // L3G_CTRL_REG1 0x20
			I2C_write(I2C1, 0xBF);   // 0x0F = 0b00001111
									 // ODR 100Hz Cut-off 12.5
									 // Normal power mode, all axes enabled


		I2C_stop(I2C1);
		I2C_start(I2C1, GYRO_ADDRESS, I2C_Direction_Transmitter);

			I2C_write(I2C1, 0x23); // CTRL_REG4
			I2C_write(I2C1, 0x10);

		I2C_stop(I2C1);

		//akcelerometr (Accel)
		I2C_start(I2C1, ACCEL_ADDRESS, I2C_Direction_Transmitter);

			I2C_write(I2C1,0x20);   // LSM303_CTRL_REG1_A  0x20
			I2C_write(I2C1,0x37);   // Enable Accelerometer
									// 0x27 = 0b00100111
									// ODR 50Hz Cut-off 37
									// Normal power mode, all axes enabled
		I2C_stop(I2C1);
		I2C_start(I2C1, ACCEL_ADDRESS, I2C_Direction_Transmitter);


			I2C_write(I2C1,0x23);
			I2C_write(I2C1,0x30);

		I2C_stop(I2C1);

		//magnetometr (Compass)
		I2C_start(I2C1, COMPASS_ADDRESS, I2C_Direction_Transmitter);


			I2C_write(I2C1,0x00);
			I2C_write(I2C1,0x20);

		I2C_stop(I2C1);
		I2C_start(I2C1, COMPASS_ADDRESS, I2C_Direction_Transmitter);

			I2C_write(I2C1,0x01);
			I2C_write(I2C1,0x20);

		I2C_stop(I2C1);
		I2C_start(I2C1, COMPASS_ADDRESS, I2C_Direction_Transmitter);


			I2C_write(I2C1,0x02);	  //LSM303_MR_REG_M   0x02
			I2C_write(I2C1,0x00);     // Enable Magnetometer
									  // 0x00 = 0b00000000
			  	  	  	  	  	  	  // Continuous conversion mode
		I2C_stop(I2C1);

	unsigned int i;
	if (SysTick_Config(SystemCoreClock/200))  while (1);

	for(;;)
	{
		GPIO_ToggleBits (GPIOD, GPIO_Pin_14);
		for (i=0;i<1000000;i++);
	}

}

void I2C1_init(void){

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
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// init GPIOB

	// Connect I2C1 pins to AF
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);	// SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA

	// configure I2C1
	I2C_InitStruct.I2C_ClockSpeed = 400000; 		// 400kHz
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;			// I2C mode
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;	// 50% duty cycle --> standard
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;			// own address, not relevant in master mode
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;		// disable acknowledge when reading (can be changed later on)
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
	I2C_Init(I2C1, &I2C_InitStruct);				// init I2C1

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
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction){
	// wait until I2C1 is not busy any more
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

	// Send I2C1 START condition
	I2C_GenerateSTART(I2Cx, ENABLE);

	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	// Send slave Address for write
	I2C_Send7bitAddress(I2Cx, address, direction);

	/* wait for I2Cx EV6, check if
	 * either Slave has acknowledged Master transmitter or
	 * Master receiver mode, depending on the transmission
	 * direction
	 */
	if(direction == I2C_Direction_Transmitter){
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	}
	else if(direction == I2C_Direction_Receiver){
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

/* This function transmits one byte to the slave device
 * Parameters:
 *		I2Cx --> the I2C peripheral e.g. I2C1
 *		data --> the data byte to be transmitted
 */
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data)
{
	I2C_SendData(I2Cx, data);
	// wait for I2C1 EV8 --> last byte is still being transmitted (last byte in SR, buffer empty), next byte can already be written
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
}

/* This function reads one byte from the slave device
 * and acknowledges the byte (requests another byte)
 */
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx){
	// enable acknowledge of received data
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This function reads one byte from the slave device
 * and doesn't acknowledge the received data
 * after that a STOP condition is transmitted
 */
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx){
	// disable acknowledge of received data
	// nack also generates stop condition after last byte received
	// see reference manual for more info
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This function issues a stop condition and therefore
 * releases the bus
 */
void I2C_stop(I2C_TypeDef* I2Cx){

	// Send I2C1 STOP Condition after last byte has been transmitted
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait for I2C1 EV8_2 --> byte has been transmitted
	//while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}



// minIMU
void Read_Gyro(){
	I2C_start(I2C1, GYRO_ADDRESS, I2C_Direction_Transmitter);
	I2C_write(I2C1, 0x28| (1 << 7) ); //L3G_OUT_X_L   0x28
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

void Read_Accel(){
	I2C_start(I2C1, ACCEL_ADDRESS, I2C_Direction_Transmitter);
	I2C_write(I2C1,0x28| (1 << 7));
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

void Read_Compass(){
	I2C_start(I2C1, COMPASS_ADDRESS, I2C_Direction_Transmitter);
	I2C_write(I2C1,0x03); //LSM303_OUT_X_H_M    0x03
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

float deg2rad(float degrees)
{
    return (float)(M_PI / 180) * degrees;
}

float rad2deg(float radians)
{
    return (radians * 180) / (float)M_PI;
}
