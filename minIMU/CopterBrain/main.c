#include <math.h>
#include <stdio.h>

#include "stm32f4xx_conf.h"
#include "stm32f4xx_gpio.h"

//#include "MadgwickAHRS.h"
#include "minIMU.h"
#include "PololuAHRS.h"
#include "millis_micros.h"

//---------------------------------------------------------

#define GRAVITY 256  //this equivalent to 1G in the raw data coming from the accelerometer
#define ToRad(x) ((x)*0.01745329252)  // *pi/180
#define ToDeg(x) ((x)*57.2957795131)  // *180/pi

// !-!
uint8_t SENSOR_SIGN[9] = { 1, 1, 1, -1, -1, -1, 1, 1, 1 }; //Correct directions x,y,z - gyro, accelerometer, magnetometer

float G_Dt = 0.02; // Integration time (DCM algorithm)  We will run the integration loop at 50Hz if possible

uint32_t timer = 0, timer_old;
uint32_t timer24 = 0; //Second timer used to print values
uint8_t AN[6]; //array that stores the gyro and accelerometer data
uint8_t AN_OFFSET[6] = { 0, 0, 0, 0, 0, 0 }; //Array that stores the Offset of the sensors

uint8_t gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z, magnetom_x, magnetom_y, magnetom_z;
float c_magnetom_x, c_magnetom_y, c_magnetom_z;
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

uint8_t counter = 0;
short gyro_sat = 0;

float DCM_Matrix[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
float Update_Matrix[3][3] = { { 0, 1, 2 }, { 3, 4, 5 }, { 6, 7, 8 } }; //Gyros here

float Temporary_Matrix[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

uint8_t theByte;

void printdata();
void SysTick_Handler(void) {
	// clock liczy
	counter++;
	timer_old = timer;
	timer = millis();
	if (timer > timer_old)
		G_Dt = (timer - timer_old) / 1000.0; // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
	else
		G_Dt = 0.0;

	// *** DCM algorithm
	// Data adquisition
	Read_Gyro(); // This read gyro data
	Read_Accel(); // Read I2C accelerometer

	if (counter > 5) // Read compass data at 10Hz... (5 loop runs)
			{
		counter = 0;
		Read_Compass(); // Read I2C magnetometer
		Compass_Heading(); // Calculate magnetic heading
	}

	// Calculations...
	Matrix_update();
	Normalize();
	Drift_correction();
	Euler_angles();
	// ***
	printdata();
}

//--------------------------------------------
int main(void) {
	unsigned int i,y;
	SystemInit();
	SystemCoreClockUpdate();

	I2C1_init();
	// Init Accel Compass Gyro
	configInit();
	init_millis_micros();
	// Przerwanie 50 x na sec
	SysTick_Config(SystemCoreClock / 50);

	for (i = 0; i < 32; i++) {
		Read_Gyro();
		Read_Accel();
		for (y = 0; y < 6; y++) // Cumulate values
			AN_OFFSET[y] += AN[y];
	}

	for (y = 0; y < 6; y++)
		AN_OFFSET[y] = AN_OFFSET[y] / 32;

	AN_OFFSET[5] -= GRAVITY * SENSOR_SIGN[5];

	delay(2000);

	timer = millis();

	delay(20);
	counter = 0;


	while(1);
	return 0;
}


void printdata()
{
//	uint8_t s[255];
//	if (!VCP_get_char(&theByte)) {
//		sprintf(s, "!ANG:%f,%f,%f\n", ToDeg(roll), ToDeg(pitch), ToDeg(yaw));
//		VCP_send_str(s);
//	}
}
