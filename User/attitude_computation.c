#include "attitude_computation.h"

MPU6050_STRUCT mpu6050;
MPU6050_OUTPUT_STRUCT mpu6050_output;	//mpu6050数据读取

KALMAN_STRUCT atti_x,atti_y;	//x和y轴姿态的卡尔曼滤波结构体，其中包含角速度和角度输出

//mpu6050读取的数据
xyz_f_t Gyro_mpu6050_out;	//mpu6050读取的经过滤波的角速度
xyz_f_t Accel;				//mpu6050读取的加速度
xyz_f_t Angle_estimate;		//用加速度估计出的角度

//滤波结果
xyz_f_t Gyro;
xyz_f_t Angle;

void Attitude_Init(void)
{
	Gyro_mpu6050_out.x = 0.0;
	Gyro_mpu6050_out.y = 0.0;
	Gyro_mpu6050_out.z = 0.0;
	
	Accel.x = 0.0;
	Accel.y = 0.0;
	Accel.z = 0.0;
	
	Angle_estimate.x = 0.0;
	Angle_estimate.y = 0.0;
	Angle_estimate.z = 0.0;
	
	Kanman_Init(&atti_x);	//初始化卡尔曼结构体
	Kanman_Init(&atti_y);
}

void Attitude_sensor_Read(u32 T)
{
	//从IIC读取mpu6050数据，调用函数滤波
	mpu6050_Input_And_Measure(&mpu6050,&mpu6050_output,T);	
}

void Attitude_sensor_Update(u32 T)
{
//	//从IIC读取mpu6050数据，调用函数滤波
//	mpu6050_Input_And_Measure(&mpu6050,&mpu6050_output,T);	
	
	//把mpu6050_output赋值到Gyro
	//这些赋值都是float型的
	Gyro_mpu6050_out.x = -mpu6050_output.Gyro_f_g.x;
	Gyro_mpu6050_out.y = -mpu6050_output.Gyro_f_g.y;
	Gyro_mpu6050_out.z = -mpu6050_output.Gyro_f_g.z;
	
	Accel.x = mpu6050_output.Acc_f_g.x;
	Accel.y = mpu6050_output.Acc_f_g.y;
	Accel.z = mpu6050_output.Acc_f_g.z;
	
	//使用加速度值，运算Angle估计值
	//所以倒不清楚的轴变换都在这里面进行，过了这个函数都变成符合右手螺旋定则的转动系
	Accel_To_Angle(&Accel,&Angle_estimate);	
	
	//卡尔曼滤波姿态解算
	Kanman_Filter(&atti_x,Gyro_mpu6050_out.x,Angle_estimate.x,T);
	Kanman_Filter(&atti_y,Gyro_mpu6050_out.y,Angle_estimate.y,T);
	
	//卡尔曼姿态输出
	//只有x和y轴能够估计得到，z轴在没有磁力计情况下无法估计
	Gyro.x = atti_x.Gyro_x;
	Gyro.y = atti_y.Gyro_x;
	
	Angle.x = atti_x.Angel;
	Angle.y = atti_y.Angel;
	
}
