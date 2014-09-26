#include <math.h>
#include "stm32f4xx_conf.h"
#include "stm32f4xx_gpio.h"



//#include "MadgwickAHRS.h"
#include "minIMU.h"
#include "PololuAHRS.h"

//---------------------------------------------------------

//---------------------------------------------------------

#define GRAVITY 256  //this equivalent to 1G in the raw data coming from the accelerometer
#define ToRad(x) ((x)*0.01745329252)  // *pi/180
#define ToDeg(x) ((x)*57.2957795131)  // *180/pi
#define Gyro_Gain_X 0.07 //X axis Gyro gain
#define Gyro_Gain_Y 0.07 //Y axis Gyro gain
#define Gyro_Gain_Z 0.07 //Z axis Gyro gain
#define Gyro_Scaled_X(x) ((x)*ToRad(Gyro_Gain_X)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Y(x) ((x)*ToRad(Gyro_Gain_Y)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Z(x) ((x)*ToRad(Gyro_Gain_Z)) //Return the scaled ADC raw data of the gyro in radians for second
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

#define OUTPUTMODE 1

#define PRINT_ANALOGS 0 //Will print the analog raw data
#define PRINT_EULER 1   //Will print the Euler angles Roll, Pitch and Yaw
#define STATUS_LED 13

// !-!
uint8_t SENSOR_SIGN[9] = { 1, 1, 1, -1, -1, -1, 1, 1, 1 }; //Correct directions x,y,z - gyro, accelerometer, magnetometer

float G_Dt = 0.02; // Integration time (DCM algorithm)  We will run the integration loop at 50Hz if possible

long timer = 0; //general purpuse timer
long timer_old;
long timer24 = 0; //Second timer used to print values
uint8_t AN[6]; //array that stores the gyro and accelerometer data
uint8_t AN_OFFSET[6] = { 0, 0, 0, 0, 0, 0 }; //Array that stores the Offset of the sensors

uint8_t gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z, magnetom_x,
		magnetom_y, magnetom_z;
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

uint8_t theByte;


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
	return 0;
}


