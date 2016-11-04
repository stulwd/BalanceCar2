代码开发记录

2016.9.24
更新了卡尔曼相关的数据结构，使其同 卡尔曼滤波C代码分析.pdf 相对应
存在问题：Gyro_x的赋值代码被杠调，不知原因

mpu6050及卡尔曼滤波相关结构体：

1.
kalman.h：

KALMAN_STRUCT

2.
mpu6050_measure.h:

typedef struct 
{	
	xyz_f_t Acc_f;	//加速度
	xyz_f_t Gyro_f;	//角速度
	s16 Tempreature;
	
	xyz_f_t Acc_f_g;	//加速度(单位是g)
	xyz_f_t Gyro_f_g;	//角速度(单位是角度)
	
	xyz_s16_t Acc_temp[10];	//滑动滤波缓冲区
	xyz_s16_t Gyro_temp[10];	//滑动滤波缓冲区
	
}MPU6050_OUTPUT_STRUCT;

3.
bsp_mpu6050.h:

typedef struct 
{
	u8 mpu6050_buffer[14];
	
	xyz_s16_t Acc_I16;	//加速度输入原始值
	xyz_s16_t Gyro_I16;	//角速度输入原始值
	
	s16 Tempreature;
	
}MPU6050_STRUCT;

2016.9.25
加入 attitude computation.c 和 attitude computation.h
设计思路：
bsp_mpu6050提供数据读取
mpu6050_measure提供数据倍率变换和相关滤波
kalman提供姿态解算算法

在attitude computation.c中定义姿态结构体（卡尔曼结构体）作为姿态存储空间，调用卡尔曼相关函数进行解算
也就是说上面三个文件都是提供一些方法，在attitude computation.c将这些方法按照顺序组合，形成姿态解算

9.26
改造bsp_mpu6050和mpu6050_measure内函数结构，入参全部改为指针形式

mpu6050相关结构体移入attitude_computation.c
MPU6050_STRUCT mpu6050;
MPU6050_OUTPUT_STRUCT mpu6050_output;

9.27
关于MPU6050模块：
MPU6050模块的AD0是内部通过一个电阻接GND。
悬空相当于低电平。接VCC能够得到高电平，不会造成VCC与GND直连。

10.3
工程更新为C8和ZE双编译版本。
对时间单位问题的解决：
由于使用的芯片是F103，理论上应该尽量减少float型变量的操作，所以在主函数内的时间参数均设定为
以u32型，毫秒为单位的变量
也就是说，1s在变量中被表示为 u32 dt = 1000;	//1000ms
对于kalman等需要float类型时间的函数，采用在函数内部手动转换的方式解决。

将attitude computation.c 和 attitude computation.h 改名为 attitude_computation.c 和 attitude_computation.h

参数类型选择问题：
u32 = uint32_t = typedef unsigned int
s32 = int32_t  = typedef   signed int

参考匿名飞控，决定把角度和角速度值用float型变量表示。在pid算法里面用p值调整放大倍数。
如果用s32类型表示，在乘p值的时候就有可能造成变量整体过大，有可能接近变量最大值。所以先用float，然后用p的倍率放大。


更新Accel_To_Angle函数

旧函数：
void Accel_To_Angle(xyz_f_t * Accel,xyz_f_t * Angle_t)
{
//	xyz_f_t Angle_t;	//用加速度计数据计算角度的估价值
	
	Angle_t->z = atan2(Accel->y,Accel->x)*180.0/PI;	//计算倾角，并转化为弧度制，取值范围0-2π，这个地方转化为弧度制好像很没用，还有点捣乱
	Angle_t->x = atan2(Accel->z,Accel->y)*180.0/PI;
	Angle_t->y = atan2(Accel->x,Accel->z)*180.0/PI;
	
//	return Angle_t;
}

新函数：
void Accel_To_Angle(xyz_f_t * Accel,xyz_f_t * Angle_t)
{
	Angle_t->z = atan2(Accel->y,Accel->x);	//计算倾角
	Angle_t->x = atan2(Accel->z,Accel->y);
	Angle_t->y = atan2(Accel->x,Accel->z);
}

10.4
更新了bsp_pwm_out中的速度输入函数。
完善了schedule.c。

建立data_transfer.c和data_transfer.h。
在usart1的接受中断里加入接受中断处理函数，
后续准备在接收中断中做命令识别，支持串口调节pid。

想起来了，uint8_t中的t是typedef的意思。

10.5
增加movement_control文件，形成集 电机驱动 和 速度采集 为一体的硬件抽象层

完成协议及其解析函数
/*
	长度：		7 Byte

	Byte1	包头		0xAA
	Byte2	识别符		1~255
	Byte3	内容1
	Byte4	内容2
	Byte5	内容3
	Byte6	内容4
	Byte7	包尾		0xBB
	
*/

2016.10.14
对scheduler.c里面的delay_ms相关逻辑进行改动
	if(delay_ms_counter != 0)
改为
	if(delay_ms_counter >= 0)
	
删除：	
//这句话应该不会起作用，算是增强鲁棒性吧
//如果delay_ms_counter<0了，整个延时逻辑就出大问题了
if(delay_ms_counter < 0)
	delay_ms_counter = 0;
	
bsp_i2c.c中I2C_1_Init()里加入

//**************************************************************************
//新加入部分，用于释放I2C总线
//**************************************************************************
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;//IIC??PB6--SCL,PB7--SDA
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	   //复用输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   //50MHZ
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	for(i=0;i<10;i++)
	{
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
	delay_us(100);
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
	delay_us(100);
	}
	
//**************************************************************************
//结束
//**************************************************************************

和

//百度文库里面的一片文档说要使能应答功能
I2C_AcknowledgeConfig(I2C1, ENABLE);//使能应答功能

发现加入的代码都没有用，好像还带来了新问题

在IICwriteBit里面加了一个100us的delay就好了
void IICwriteBit(u8 dev, u8 reg, u8 bitNum, u8 data){
	u8 b;
	Delay_Us(100);		//加入了100us延迟
	IIC_Read_nByte(dev, reg, 1, &b);
	b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
	mpu6050_ok = !( IIC_Write_1Byte(dev, reg, b) );	//IIC_Write_1Byte正常情况返回0，经过去反mpu6050_ok = 1
}


10.17

改用软件I2C

Accel_To_Angle内改用角度制

发现mpu6050在上电后直接初始化返回数据是0,在初始化前加200ms延时解决问题

增加串口2驱动，开串口2接收中断，但没有转接printf函数，printf函数还在usart1上

在All_Init函数里统一配置中断优先级分组方式

调整All_Init函数结构

弃用以systick为计时器的Delay_ms()函数，延时改用以while循环方式实现的Delay_Ms()和Delay_Us()
后期逐步开始使用基于timer的延时函数

车轮转向控制部分还没有写完， 引脚选用还没有经过检查


后续工作：
1.确定Timer使用情况
2.确定GPIO使用情况

定时器：
C8：3个通用定时器，1个高级定时器
ZE：4个通用定时器，2个高级定时器，1个基本定时器

GPIO：
C8：PA0-15
	PB0-15
	PC13-15
	
	USART1 TX PA9
		   RX PA10
	
	USART2 TX PA2
		   Rx PA3
	
	USART3 TX PB10
		   RX PB11
		   
	Soft_IIC PB8
		     PB9
	
	输出：
	Timer3 CH3 PB0 
		   CH4 PB1
		   
	输入：
	Timer1 CH1 PA8
	Timer3 CH1 PA6
	
	延时：
	Timer4
	
	系统时间测量
	Timer2
	
	电机控制：
	PA1
	PA2
	PA3
	PA4
	
10.20
初步制作了电机控制函数，但存在输入变量为s32型，但是pwm输出只支持u16的问题，此处可能导致数值溢出
PWM输出函数里面设置的最大计数值为1000，不知道1000是不是最大计数值

Get_Speed里缺少含有物理意义的单位换算

11.1
对IO端口进行官方例程匹配

完成电机控制的4个引脚配置为
AIN2:GPIOB 15
AIN1:GPIOB 14
BIN1:GPIOB 13
BIN2:GPIOB 12

把 movement_control、Timer_In_Out 拆成 bsp_motor、bsp_encoder、movement_control。
	
		   



