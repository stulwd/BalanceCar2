#include "mpu6050_measure.h"

int mpu6050_init(u16 lpf)
{
	return MPU6050_Init(lpf);
}

float TEM_LPF;	//温度数值低通滤波中间变量
float mpu6050_Temperature_Prepare(MPU6050_STRUCT * mpu6050,u32 T)
{
	//温度数据滤波换算
	TEM_LPF += 2 *3.14f *T *(mpu6050->Tempreature - TEM_LPF);	//过一下低通滤波
	return (TEM_LPF/340.0f + 36.5f);
}

void mpu6050_6axis_Prepare(MPU6050_STRUCT * original,MPU6050_OUTPUT_STRUCT * output)
{
	static int i = 0;
	int j;
	xyz_f_t Acc_sum = {0,0,0};
	xyz_f_t Gyro_sum = {0,0,0};
	
//*******************************滑动滤波*********************************************************
	//循环赋值
	output->Acc_temp[i] = original->Acc_I16;
	output->Gyro_temp[i] = original->Gyro_I16;
	i++;
	if(i > 9) i = 0;
	
	//求和
	for(j=0;j<10;j++)
	{
		Acc_sum.x += output->Acc_temp[j].x;
		Acc_sum.y += output->Acc_temp[j].y;
		Acc_sum.z += output->Acc_temp[j].z;
		
		Gyro_sum.x += output->Gyro_temp[j].x;
		Gyro_sum.y += output->Gyro_temp[j].y;
		Gyro_sum.z += output->Gyro_temp[j].z;
	}
	
	//取平均
	output->Acc_f.x = Acc_sum.x / 10.0;
	output->Acc_f.y = Acc_sum.y	/ 10.0;
	output->Acc_f.z = Acc_sum.z	/ 10.0;	
	output->Gyro_f.x = Gyro_sum.x / 10.0;
	output->Gyro_f.y = Gyro_sum.y / 10.0;
	output->Gyro_f.z = Gyro_sum.z / 10.0;
//******************************滑动滤波结束********************************************************
	
	//进行单位转换（除以2^16,再乘以最大量程）
	//所以换算后的单位应该是 cm/s2 和 °/s
	output->Acc_f_g.x  = output->Acc_f.x * To_CM_S2;	//加速度度最大量程 +-8G
	output->Acc_f_g.y  = output->Acc_f.y * To_CM_S2;	//读取数据位数 16
	output->Acc_f_g.z  = output->Acc_f.z * To_CM_S2;
	output->Gyro_f_g.x = output->Gyro_f.x * To_Angle;	//陀螺仪最大量程 +-2000度每秒
	output->Gyro_f_g.y = output->Gyro_f.y * To_Angle;	//读取数据位数 16
	output->Gyro_f_g.z = output->Gyro_f.z * To_Angle;
}

void mpu6050_Input_And_Measure(MPU6050_STRUCT *mpu6050,MPU6050_OUTPUT_STRUCT *mpu6050_output,u32 T)
{
	MPU6050_Read(mpu6050);	//调用MPU6050驱动，读取数据，自动存入相关结构体
	mpu6050_6axis_Prepare(mpu6050,mpu6050_output);
	mpu6050_output->Tempreature = mpu6050_Temperature_Prepare(mpu6050,T);
}


void Accel_To_Angle(xyz_f_t * Accel,xyz_f_t * Angle_t)
{
//	xyz_f_t Angle_t;	//用加速度计数据计算角度的估价值
	
	Angle_t->z = atan2(Accel->y,Accel->x)*180/PI;	//计算倾角
	Angle_t->x = atan2(Accel->z,Accel->y)*180/PI;
	Angle_t->y = atan2(Accel->x,Accel->z)*180/PI;
	
//	return Angle_t;
}



