#include "bsp_mpu6050.h"
#include "scheduler.h"
#include "time.h"
#include "ioi2c.h"


#define MPU6050_SLAVE_ADDRESS 0xD0	//AD0接低

//**********************************************领航者mpu6050驱动移植**********************************************888

//读、写两个转接函数都没有使用SlaveAddress入参，此参数在本文件头部以宏定义形式预置
//IIC读n字节数据转接函数
u8 IIC_Read_nByte(u8 SlaveAddress, u8 REG_Address, u8 len, u8 *buf)
{
	return IICreadBytes(SlaveAddress, REG_Address, len,buf);
}

//I2C接口写入转接函数
u8 IIC_Write_1Byte(u8 SlaveAddress,u8 REG_Address,u8 REG_data)
{
	IICwriteBytes(SlaveAddress, REG_Address, 1, &REG_data);
	return 0;
}



/***********************************************************************************************************
												功能实现函数
***********************************************************************************************************/

/**************************实现函数********************************************
*函数原型:		void MPU6050_setSleepEnabled(uint8_t enabled)
*功　　能:	    MPU6050 的睡眠模式开关
				0 关
				1 开
*******************************************************************************/
void MPU6050_setSleepEnabled(uint8_t enabled) {
	IICwriteBit(MPU6050_SLAVE_ADDRESS, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setClockSource(uint8_t source)
*功　　能:	    设置  MPU6050 的时钟源
* CLK_SEL | Clock Source
* --------+--------------------------------------
* 0       | Internal oscillator
* 1       | PLL with X Gyro reference
* 2       | PLL with Y Gyro reference
* 3       | PLL with Z Gyro reference
* 4       | PLL with external 32.768kHz reference
* 5       | PLL with external 19.2MHz reference
* 6       | Reserved
* 7       | Stops the clock and keeps the timing generator in reset
*******************************************************************************/
void MPU6050_setClockSource(uint8_t source)
{
	IICwriteBits(MPU6050_SLAVE_ADDRESS, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source);
	
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_set_SMPLRT_DIV(uint16_t hz)
*功　　能:	    设置 MPU6050 的陀螺仪输出频率
*******************************************************************************/
void MPU6050_set_SMPLRT_DIV(uint16_t hz)
{
	IIC_Write_1Byte(MPU6050_SLAVE_ADDRESS, MPU6050_RA_SMPLRT_DIV,1000/hz - 1);
//	I2C_Single_Write(MPU6050_ADDRESS,MPU_RA_SMPLRT_DIV, (1000/sample_rate - 1));
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setFullScaleGyroRange(uint8_t range)
*功　　能:	    设置  MPU6050 陀螺仪 的最大量程
*******************************************************************************/
void MPU6050_setFullScaleGyroRange(uint8_t range) {
	IICwriteBits(MPU6050_SLAVE_ADDRESS, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
	IICwriteBits(MPU6050_SLAVE_ADDRESS, MPU6050_RA_GYRO_CONFIG,7, 3, 0x00);   //不自检
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setFullScaleAccelRange(uint8_t range)
*功　　能:	    设置  MPU6050 加速度计的最大量程
*******************************************************************************/
void MPU6050_setFullScaleAccelRange(uint8_t range) {
	IICwriteBits(MPU6050_SLAVE_ADDRESS, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
	IICwriteBits(MPU6050_SLAVE_ADDRESS, MPU6050_RA_ACCEL_CONFIG,7, 3, 0x00);   //不自检
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setDLPF(uint8_t mode)
*功　　能:	    设置  MPU6050 数字低通滤波的频带宽度
*******************************************************************************/
void MPU6050_setDLPF(uint8_t mode)
{
	IICwriteBits(MPU6050_SLAVE_ADDRESS, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setI2CMasterModeEnabled(uint8_t enabled)
*功　　能:	    设置 MPU6050 是否为AUX I2C线的主机
*******************************************************************************/
void MPU6050_setI2CMasterModeEnabled(uint8_t enabled) {
	IICwriteBit(MPU6050_SLAVE_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, enabled);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setI2CBypassEnabled(uint8_t enabled)
*功　　能:	    设置 MPU6050 是否为AUX I2C线的主机
*******************************************************************************/
void MPU6050_setI2CBypassEnabled(uint8_t enabled) {
	IICwriteBit(MPU6050_SLAVE_ADDRESS, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_I2C_BYPASS_EN_BIT, enabled);
}

//*********************************************************************************************************
//							应用函数
//*********************************************************************************************************

/**************************实现函数********************************************
*函数原型:		void MPU6050_initialize(void)
*功　　能:	    初始化 	MPU6050 以进入可用状态。改自匿名飞控领航者mpu6050驱动。
*******************************************************************************/
int MPU6050_Init(u16 lpf)
{ 
	u16 default_filter = 1;
	
	//选择mpu6050内部数字低筒滤波器带宽
	//不开启内部低通滤波，陀螺仪采样率 8MHz
	//  开启内部低通滤波，陀螺仪采样率 1MHz
	//加速度计采样率1MHz
	switch(lpf)
	{
	case 5:
		default_filter = MPU6050_DLPF_BW_5;
		break;
	case 10:
		default_filter = MPU6050_DLPF_BW_10;
		break;
	case 20:
		default_filter = MPU6050_DLPF_BW_20;
		break;
	case 42:
		default_filter = MPU6050_DLPF_BW_42;
		break;
	case 98:
		default_filter = MPU6050_DLPF_BW_98;
		break;
	case 188:
		default_filter = MPU6050_DLPF_BW_188;
		break;
	case 256:
		default_filter = MPU6050_DLPF_BW_256;
		break;
	default:
		default_filter = MPU6050_DLPF_BW_42;
		break;
	}

	delay_ms(200);
	
	//设备复位
//	IIC_Write_1Byte(MPU6050_SLAVE_ADDRESS,MPU6050_RA_PWR_MGMT_1, 0x80);
	
	//这里使用的Delay()只能在初始化阶段使用，任务调度中使用这种Delay()，会卡死整个调度
	MPU6050_setSleepEnabled(0); //进入工作状态
	delay_ms(10);
	MPU6050_setClockSource(MPU6050_CLOCK_PLL_ZGYRO);	//设置时钟  0x6b   0x03
														//时钟源选择，MPU6050_CLOCK_INTERNAL表示内部8M晶振
	delay_ms(10);
	MPU6050_set_SMPLRT_DIV(1000);  //1000hz
	delay_ms(10);
	MPU6050_setFullScaleGyroRange(MPU6050_GYRO_FS_2000);//陀螺仪最大量程 +-2000度每秒
	delay_ms(10);
	MPU6050_setFullScaleAccelRange(MPU6050_ACCEL_FS_8);	//加速度度最大量程 +-8G
	delay_ms(10);
	MPU6050_setDLPF(default_filter);  //42hz
	delay_ms(10);
	MPU6050_setI2CMasterModeEnabled(0);	 //不让MPU6050 控制AUXI2C
	delay_ms(10);
	MPU6050_setI2CBypassEnabled(1);	 //主控制器的I2C与	MPU6050的AUXI2C	直通。控制器可以直接访问HMC5883L
	delay_ms(10);
	
	return 0;
}



//读取MPU6050输出寄存器数值
void MPU6050_Read(MPU6050_STRUCT * mpu6050)
{
	IIC_Read_nByte(MPU6050_SLAVE_ADDRESS,MPU6050_RA_ACCEL_XOUT_H,14,mpu6050->mpu6050_buffer);
	
	/*拼接buffer原始数据*/
	mpu6050->Acc_I16.x = ((((int16_t)mpu6050->mpu6050_buffer[0]) << 8) | mpu6050->mpu6050_buffer[1]) ;
	mpu6050->Acc_I16.y = ((((int16_t)mpu6050->mpu6050_buffer[2]) << 8) | mpu6050->mpu6050_buffer[3]) ;
	mpu6050->Acc_I16.z = ((((int16_t)mpu6050->mpu6050_buffer[4]) << 8) | mpu6050->mpu6050_buffer[5]) ;
 
	mpu6050->Gyro_I16.x = ((((int16_t)mpu6050->mpu6050_buffer[ 8]) << 8) | mpu6050->mpu6050_buffer[ 9]) ;
	mpu6050->Gyro_I16.y = ((((int16_t)mpu6050->mpu6050_buffer[10]) << 8) | mpu6050->mpu6050_buffer[11]) ;
	mpu6050->Gyro_I16.z = ((((int16_t)mpu6050->mpu6050_buffer[12]) << 8) | mpu6050->mpu6050_buffer[13]) ;
	
	mpu6050->Tempreature = ((((int16_t)mpu6050->mpu6050_buffer[6]) << 8) | mpu6050->mpu6050_buffer[7]); //tempreature
	
}



