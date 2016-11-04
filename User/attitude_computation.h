#ifndef __ATTITUDE_COMPUTATION_H
#define	__ATTITUDE_COMPUTATION_H

#include "stm32f10x.h"

#include "include.h"

#include "bsp_mpu6050.h"
#include "mpu6050_measure.h"
#include "kalman.h"

//×ËÌ¬Êä³ö
extern xyz_f_t Gyro;
extern xyz_f_t Angle;

void Attitude_Init(void);
void Attitude_sensor_Update(u32 T);
void Attitude_sensor_Read(u32 T);

#endif /* __ATTITUDE_COMPUTATION_H */
