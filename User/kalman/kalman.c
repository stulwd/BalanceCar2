#include "kalman.h"

void Kanman_Init(KALMAN_STRUCT * kalman)
{
	int i;
	
	//输出
	(*kalman).Angel = 0.0;	//最优估计的角度	是最终角度结果
	(*kalman).Gyro_x = 0.0;	//最优估计角速度
	
	//固定参量
	(*kalman).Q_Angle = 0.001;		//{0.001,0.001,0.001};	//陀螺仪噪声协方差	0.001是经验值
	(*kalman).Q_Gyro = 0.003;		//{0.003,0.003,0.003};	//陀螺仪漂移噪声协方差	是mpu6050的经验值
	(*kalman).R_Angle = 0.5;		//{0.5,0.5,0.5};	//是加速度计噪声的协方差	
	
	(*kalman).C_0 = 1;		//{1,1,1};	//H矩阵的一个观测参数 是常数
	
	//中间量
	(*kalman).Q_Bias = 0;		//{0,0,0};		//陀螺仪飘移预估值
	(*kalman).Angle_err = 0;	//{0,0,0};		//计算中间值 Angle 观测值-预估值
	
	(*kalman).PCt_0 = 0;			//{0,0,0},	//计算中间值
	(*kalman).PCt_1 = 0;			//{0,0,0},
	(*kalman).E     = 0;			//{0,0,0};
	(*kalman).t_0   = 0;			//{0,0,0},	//t:计算中间变量
	(*kalman).t_1   = 0;			//{0,0,0},
	
	(*kalman).K_0 = 0;			//{0,0,0},	//K:卡尔曼增益
	(*kalman).K_1 = 0;			//{0,0,0},
	
	for(i = 0;i < 4;i++)	//{0,0,0,0}	//计算P矩阵的中间矩阵
	{
		(*kalman).Pdot[i] = 0;
	}
	
	(*kalman).PP[0][0] = 1;
	(*kalman).PP[0][1] = 0;
	(*kalman).PP[1][0] = 0;
	(*kalman).PP[1][1] = 1;
}


void Kanman_Filter(KALMAN_STRUCT * kalman,float Gyro,float Accel,u32 dt)	//Gyro陀螺仪的测量值  |  Accel加速度计的角度计  |  dt的时间考虑用小数 或 更小的分度表示
{
	float dt_f;
	
	//把dt这个单位是ms的u32型变量里的值转换为float型的以秒为单位的值
	dt_f = (float)dt;
	dt_f = dt_f / 1000;
	
	//x轴指向前，y轴指向左的坐标系   要算俯仰角
	//那么输入的应该是y轴的角速度（Gyro）和y轴的倾角加速度计估计值
	//坐标系情况大概是这样
	
	
	//角度测量模型方程 角度估计值=上一次最有角度+（角速度-上一次的最优零飘）*dt_f
	//就漂移来说，认为每次都是相同的Q_bias=Q_bias
	//估计角度
	(*kalman).Angel += (Gyro - (*kalman).Q_Bias) * dt_f;
	
	//计算估计模型的方差
	(*kalman).Pdot[0] = (*kalman).Q_Angle - (*kalman).PP[0][1] - (*kalman).PP[1][0];
	(*kalman).Pdot[1] = -(*kalman).PP[1][1];
	(*kalman).Pdot[2] = -(*kalman).PP[1][1];
	(*kalman).Pdot[3] = (*kalman).Q_Gyro;
	
	(*kalman).PP[0][0] += (*kalman).Pdot[0] * dt_f;
	(*kalman).PP[0][1] += (*kalman).Pdot[1] * dt_f;
	(*kalman).PP[1][0] += (*kalman).Pdot[2] * dt_f;
	(*kalman).PP[1][1] += (*kalman).Pdot[3] * dt_f;
	
	//计算卡尔曼增益
	(*kalman).PCt_0 = (*kalman).C_0 * (*kalman).PP[0][0];	//矩阵乘法的中间变量
	(*kalman).PCt_1 = (*kalman).C_0 * (*kalman).PP[0][1];	//C_0=1
	(*kalman).E = (*kalman).R_Angle + (*kalman).C_0 * (*kalman).PCt_0;	//分母
	(*kalman).K_0 = (*kalman).PCt_0 / (*kalman).E;	//卡尔曼增益，两个，一个是Angle的，一个是Q_bias的
	(*kalman).K_1 = (*kalman).PCt_1 / (*kalman).E;
	
	//计算最优角度、最优零飘
	(*kalman).Angle_err = Accel - (*kalman).Angel;
	(*kalman).Angel += (*kalman).K_0 * (*kalman).Angle_err;	//计算最优的角度
	(*kalman).Q_Bias += (*kalman).K_1 * (*kalman).Angle_err;	//计算最优的零飘
	
	(*kalman).Gyro_x = Gyro -(*kalman).Q_Bias;	//计算得最优角速度
	
	//更新估计模型的方差
	(*kalman).t_0 = (*kalman).PCt_0;	//矩阵计算中间变量，相当于a
	(*kalman).t_1 = (*kalman).C_0 * (*kalman).PP[0][1];	//矩阵计算中间变量，相当于b
	
	(*kalman).PP[0][0] -= (*kalman).K_0 * (*kalman).t_0;
	(*kalman).PP[0][1] -= (*kalman).K_0 * (*kalman).t_1;
	(*kalman).PP[1][0] -= (*kalman).K_1 * (*kalman).t_0;
	(*kalman).PP[1][1] -= (*kalman).K_1 * (*kalman).t_1;
}
