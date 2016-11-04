#ifndef __TIME_H
#define	__TIME_H

#include "stm32f10x.h"

void delay_us(u32 us);
void delay_ms(u32 ms);
	
//void Delay_Ms(u32 myMs);		//while实现的延时函数，在time.c内作为一种可选择的延时方式，不提供外部接口
//void Delay_Us(u32 myUs);

//void Delay_us(u32 us);	//用timer实现

void Time_Configuration(void);
u32 readnowtime(void);

#endif /* __TMIE_H */
