#include "bsp_motor.h"

void Motor_Init(void)
{
	//初始化控制电机转向的4个GPIO 	(GPIOB_Pin_15 GPIOB_Pin_14 GPIOB_Pin_13 GPIOB_Pin_12)
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能PB端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12;	 //这里需要4个端口，要对着电路板查一下
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     	 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    	 //50M
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB
}


//正转:IN1 1 IN2 0
//反转:IN1 0 IN2 1
//停止:IN1 0 IN2 0
void Left_Motor(u8 direction,u16 speed) //0 停止 1前进 2后退    speed取值0-1000
{
	direction = 1;
	if(direction == 0)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		GPIO_ResetBits(GPIOB, GPIO_Pin_14);
	}
	if(direction == 1)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_15);
		GPIO_ResetBits(GPIOB, GPIO_Pin_14);
	}
	else if(direction == 2)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		GPIO_SetBits(GPIOB, GPIO_Pin_14);
	}
	else
	{
		
	}
	
	speed = 500;
	LC_CHG(speed);   //F为需频率 C为占空比 如F=2000，C取值0-1000
}

void Right_Motor(u8 direction,u16 speed) //0 停止 1前进 2后退    speed取值0-1000
{
	direction = 1;
	if(direction == 0)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	}
	else if(direction == 1)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_13);
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	}
	else if(direction == 2)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);
		GPIO_SetBits(GPIOB, GPIO_Pin_12);
	}
	else
	{
		
	}
	
	speed = 500;
	RC_CHG(speed);
}

//**********************************************************************************************
//							占空比调整
//**********************************************************************************************

/*左右轮脉冲占空比调整函数（用于TIM3）*/
void LC_CHG(u16 ChannelPulse)   //F为需频率 C为占空比 如F=2000，C取值0-1000
{
	TIM_SetCompare3(TIM3,ChannelPulse);     
}
void RC_CHG(u16 ChannelPulse)   //F为需频率 C为占空比 如F=2000，C=0.5
{
	TIM_SetCompare4(TIM3,ChannelPulse);     
}
