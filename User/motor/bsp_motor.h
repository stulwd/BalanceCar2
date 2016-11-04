#ifndef __BSP_MOTOR_H
#define	__BSP_MOTOR_H

#include "stm32f10x.h"

void Motor_Init(void);
void Left_Motor(u8 direction,u16 speed);
void Right_Motor(u8 direction,u16 speed);

void LC_CHG(u16 ChannelPulse);
void RC_CHG(u16 ChannelPulse);

#endif /* __BSP_MOTOR_H */

