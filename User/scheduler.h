#ifndef __SCHEDULER_H
#define	__SCHEDULER_H

#include "stm32f10x.h"

typedef struct
{
	s16 check_flag;
	
	s16 cnt_2ms;
	s16 cnt_5ms;
	s16 cnt_10ms;
	s16 cnt_20ms;
	s16 cnt_50ms;
	
	s16 error_flag;
	
}loop_t;

extern loop_t loop;

void Loop_Init(void);

void Loop_check(void);
void Duty_Loop(void);

//void Delay_ms(u32 nms);

#endif /* __SCHEDULER_H */
