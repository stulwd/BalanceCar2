#ifndef __USART1_H
#define	__USART1_H

#include "stm32f10x.h"
#include <stdio.h>

void USART1_Config(void);
void NVIC_USART1_Configuration(void);
void printf_test(void);

#endif /* __USART1_H */
