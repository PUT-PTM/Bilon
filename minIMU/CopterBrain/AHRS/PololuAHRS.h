//=====================================================================================================
// PololuAHRS.h
//=====================================================================================================
#ifndef PololuAHRS_h
#define PololuAHRS_h

#define Kp_ROLLPITCH 0.02
#define Ki_ROLLPITCH 0.00002
#define Kp_YAW 1.2
#define Ki_YAW 0.00002
#define M_X_MIN -602
#define M_Y_MIN -627
#define M_Z_MIN -511
#define M_X_MAX 370
#define M_Y_MAX 356
#define M_Z_MAX 439
#define GRAVITY 256

#define ToRad(x) ((x)*0.01745329252)  // *pi/180
#define ToDeg(x) ((x)*57.2957795131)  // *180/pi
#define Gyro_Gain_X 0.07 //X axis Gyro gain
#define Gyro_Gain_Y 0.07 //Y axis Gyro gain
#define Gyro_Gain_Z 0.07 //Z axis Gyro gain
#define Gyro_Scaled_X(x) ((x)*ToRad(Gyro_Gain_X)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Y(x) ((x)*ToRad(Gyro_Gain_Y)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Z(x) ((x)*ToRad(Gyro_Gain_Z)) //Return the scaled ADC raw data of the gyro in radians for second
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#include <math.h>
#include "stm32f4xx_conf.h"
#include "stm32f4xx_gpio.h"

void Normalize(void);
void Drift_correction(void);
void Matrix_update(void);
void Euler_angles(void);
void Compass_Heading();
void Matrix_Multiply(float a[3][3], float b[3][3], float mat[3][3]) ;
float Vector_Dot_Product(float vector1[3], float vector2[3]);
void Vector_Cross_Product(float vectorOut[3], float v1[3], float v2[3]);
void Vector_Scale(float vectorOut[3], float vectorIn[3], float scale2);
void Vector_Add(float vectorOut[3], float vectorIn1[3], float vectorIn2[3]);

extern float c_magnetom_x, c_magnetom_y, c_magnetom_z;
extern uint8_t gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z, magnetom_x, magnetom_y, magnetom_z;

extern float Omega_P[3]; //Omega Proportional correction
extern float Omega_I[3]; //Omega Integrator
extern float Omega[3];

extern float G_Dt;
extern float MAG_Heading;
extern uint8_t SENSOR_SIGN[9];

extern float roll;
extern float pitch;
extern float yaw;

extern float DCM_Matrix[3][3];
extern float Update_Matrix[3][3];
extern float Temporary_Matrix[3][3];

extern float Accel_Vector[3];
extern float Gyro_Vector[3];
extern float Omega_Vector[3];

extern float errorYaw[3];
extern float errorRollPitch[3];

#endif
