#include "stm32f10x.h"
#include "init.h"
#include "ioi2c.h"
#include "bsp_usart2.h"
#include "time.h"
#include "bsp_encoder.h"
#include "bsp_motor.h"
#include "movement_control.h"
#include "bsp_led.h"
#include "bsp_key.h"


u8 All_Init(void)
{
	int init_error = 0;	//初始化错误返回，如果初始化存在错误，就将此变量置1
						//主要用在初始化外部设备（比如MPU6050）时，检查外部设备是否正常工作
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	Time_Configuration();	//系统时间和延时相关定时器初始化
	
	LED_GPIO_Config();
	EXTI_PA0_Config();
	
	USART1_Config();		//USART1 配置模式为 115200 8-N-1，中断接收
	USART2_Config();
	
	IIC_Init();	//软件IIC初始化
	
	init_error = mpu6050_init(5);	//MPU6050的初始化，要放在systick调度的后面，因为其中调用了延时函数
									//此函数的入参是对mpu6050内部数字低通滤波器的频带宽度设置，输入0代表关闭内部低通滤波器
	
	Encoder_Configuration();	//码盘初始化
	Motor_Init();				//电机初始化
	
	Attitude_Init();	//初始化姿态相关变量
	
	Balance_Init();		//自稳变量初始化
	
	//主循环初始化与启动
	Loop_Init();		//循环控制变量初始化
	SysTick_Init();		//滴答定时器初始化，1ms中断周期
						//这个定时器用于驱动整个循环运行，放在初始化的最后
						//也就是硬件初始化完毕后定时器开始运行，进而推动任务开始调度
	
	return init_error;
}
