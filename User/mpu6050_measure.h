#ifndef __MPU6050_H
#define	__MPU6050_H

#include "stm32f10x.h"
#include "include.h"
#include "math.h"
#include "bsp_mpu6050.h"

#define To_CM_S2 0.2392615f	// 980cm/s2 * 16G / 65535	±8g   -> 16g
#define To_Angle 0.0610361f	// 4000°          / 65535   ±2000 -> 4000

typedef struct 
{	
	xyz_f_t Acc_f;	//加速度
	xyz_f_t Gyro_f;	//角速度
	s16 Tempreature;
	
	xyz_f_t Acc_f_g;	//加速度(单位是g)
	xyz_f_t Gyro_f_g;	//角速度(单位是角度)
	
	xyz_s16_t Acc_temp[10];
	xyz_s16_t Gyro_temp[10];
	
}MPU6050_OUTPUT_STRUCT;

int mpu6050_init(u16 lpf);
void mpu6050_Input_And_Measure(MPU6050_STRUCT *mpu6050,MPU6050_OUTPUT_STRUCT *mpu6050_output,u32 T);
void Accel_To_Angle(xyz_f_t * Accel,xyz_f_t * Angle_t);

#endif /* __MPU6050_H */
