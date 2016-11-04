#ifndef __INIT_H
#define	__INIT_H

#include "stm32f10x.h"
#include "include.h"

#include "bsp_usart1.h"
#include "bsp_usart2.h"
#include "bsp_SysTick.h"
#include "scheduler.h"
#include "bsp_mpu6050.h"
#include "mpu6050_measure.h"
#include "attitude_computation.h"
#include "control.h"

u8 All_Init(void);

#endif /* __INIT_H */


