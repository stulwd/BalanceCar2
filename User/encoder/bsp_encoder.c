#include "bsp_encoder.h"
#include "time.h"

void TIM_Input_Output_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/*输入接口配置*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* 输出接口配置 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

	/* 输出接口配置 */	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void TIM_Input_Output_NVIC_Configuration(void)
{   
	NVIC_InitTypeDef NVIC_InitStructure; 

	/*TIM1捕获中断*/
	// 设置中断来源
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn; 	
	// 设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	 
	// 设置子优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/*TIM3捕获中断*/
	// 设置中断来源
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 	
	// 设置抢占优先级	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	 
	// 设置子优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM_Input_Output_Mode_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	/* 配置左轮输入模式的捕获通道*/
	/* 配置基本定时器TIM1 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 720-1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;    
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_PWMIConfig(TIM1, &TIM_ICInitStructure);	
	TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);
	
	/* 配置右轮输入模式的基本定时器TIM3 */	
	/* 配置基本定时器TIM3 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = 1000-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 36-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;    
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_PWMIConfig(TIM3, &TIM_ICInitStructure);
	TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
	
	/* 配置输出模式的比较输出通道3和4*/
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 500;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);	
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);	
	TIM_Cmd(TIM3, ENABLE);
	TIM_CtrlPWMOutputs(TIM3, ENABLE);	
	
	TIM_Cmd(TIM1, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

void Encoder_Configuration(void)
{
	TIM_Input_Output_GPIO_Config();
	TIM_Input_Output_NVIC_Configuration();
	TIM_Input_Output_Mode_Config();
}




//**********************************************************************************************
//							获取转速
//**********************************************************************************************
uint16_t Lhighlowtime;
u16 Get_Left_Speed(void)
{
//	printf("左轮电平%d\n",Lhighlowtime);
	return Lhighlowtime;
}

uint16_t Rhighlowtime;
u16 Get_Right_Speed(void)
{
//	printf("右轮电平%d\n",Rhighlowtime);
	return Rhighlowtime;
}

void TIM1_CC_IRQHandler(void)
{  
	uint16_t lnow;
	static uint16_t llst = 0;
	
	TIM_ClearITPendingBit(TIM1,TIM_IT_CC1);
	lnow = readnowtime();		
	Lhighlowtime = (lnow-llst);		
	llst = lnow;
}

void TIM3_IRQHandler(void)
{
	uint16_t rnow;
	static uint16_t rlst=0;
	
	TIM_ClearITPendingBit(TIM3,TIM_IT_CC1);
	rnow=readnowtime();
	Rhighlowtime = (rnow-rlst);
	rlst = rnow;
}
