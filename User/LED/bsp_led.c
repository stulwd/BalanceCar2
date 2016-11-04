#include "bsp_led.h"

void LED_GPIO_Config(void)	
{
  GPIO_InitTypeDef GPIO_InitStructure;
	
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE); // 使能PC端口时钟  
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	//选择对应的引脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);  //初始化PC端口
	
  GPIO_SetBits(GPIOC, GPIO_Pin_13 );	 // 关闭所有LED
}
