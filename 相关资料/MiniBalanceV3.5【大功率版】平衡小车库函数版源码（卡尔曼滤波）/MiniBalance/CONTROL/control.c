#include "control.h"	
#include "filter.h"
#include "usart.h"
#include "sys.h" 
  /**************************************************************************
作者：平衡小车之家
我的淘宝小店：http://shop114407458.taobao.com/
**************************************************************************/
int Balance_Pwm,Velocity_Pwm,Turn_Pwm;
u8 Flag_Target;
/**************************************************************************
函数功能：所有的控制代码都在这里面
         5ms定时中断由MPU6050的INT引脚触发
         严格保证采样和数据处理的时间同步				 
**************************************************************************/
int EXTI9_5_IRQHandler(void) 
{    
	if(INT==0)		
	{   
		EXTI->PR=1<<5;                                                      //清除LINE5上的中断标志位   
		Flag_Target=!Flag_Target;


		if(delay_flag==1)
		{
			if(++delay_50==10)	 delay_50=0,delay_flag=0;                     //给主函数提供50ms的精准延时
		}



		if(Flag_Target==1)                                                  //5ms读取一次陀螺仪和加速度计的值，更高的采样频率可以改善卡尔曼滤波和互补滤波的效果
		{
			Get_Angle(Way_Angle);                                               //===更新姿态	
			return 0;	                                               
		}                                                                   //10ms控制一次，为了保证M法测速的时间基准，首先读取编码器数据

		//每两次运行此函数中，有一次到此为止被返回。这一次在返回之前只是采集了一次角度数据。


		/*

		另一次中先读取编码器的值，然后才读取角度值

		所以总体时序为：

		5ms
		读取角度
		5ms
		读取编码器
		读取角度
		PID控制
		5ms
		读取角度
		5ms
		读取编码器
		读取角度
		PID控制
		5ms
		。。。

		*/

		Encoder_Left=-Read_Encoder(2);                                      //===读取编码器的值，因为两个电机的旋转了180度的，所以对其中一个取反，保证输出极性一致
		Encoder_Right=Read_Encoder(4);                                      //===读取编码器的值

		Get_Angle(Way_Angle);                                               //===更新姿态	


		Read_Distane();                                                     //===获取超声波测量距离值
		if(Bi_zhang==0)Led_Flash(100);                                      //===LED闪烁;常规模式 1s改变一次指示灯的状态	
		if(Bi_zhang==1)Led_Flash(0);                                        //===LED闪烁;避障模式 指示灯常亮	
		Voltage=Get_battery_volt();                                         //===获取电池电压	          
		Key();                                                              //===扫描按键状态 单击双击可以改变小车运行状态


		Balance_Pwm  = balance(Angle_Balance,Gyro_Balance);                   //===平衡PID控制	
		Velocity_Pwm = velocity(Encoder_Left,Encoder_Right);                  //===速度环PID控制	 记住，速度反馈是正反馈，就是小车快的时候要慢下来就需要再跑快一点
		Turn_Pwm     = turn(Encoder_Left,Encoder_Right,Gyro_Turn);            //===转向环PID控制     

		Moto1 = Balance_Pwm-Velocity_Pwm+Turn_Pwm;                            	//===计算左轮电机最终PWM
		Moto2 = Balance_Pwm-Velocity_Pwm-Turn_Pwm;                           	//===计算右轮电机最终PWM
		Xianfu_Pwm();                                                       	//===PWM限幅

		if(Pick_Up(Acceleration_Z,Angle_Balance,Encoder_Left,Encoder_Right))	//===检查是否小车被那起
			Flag_Stop=1;	                                                	//===如果被拿起就关闭电机
		if(Put_Down(Angle_Balance,Encoder_Left,Encoder_Right))              	//===检查是否小车被放下
			Flag_Stop=0;	                                                	//===如果被放下就启动电机

		if(Turn_Off(Angle_Balance,Voltage)==0)                              //===如果不存在异常
			Set_Pwm(Moto1,Moto2);                                             //===赋值给PWM寄存器  
	}       	
	return 0;	  
} 

/**************************************************************************
函数功能：直立PD控制
入口参数：角度、角速度
返回  值：直立控制PWM
作    者：平衡小车之家
**************************************************************************/
int balance(float Angle,float Gyro)
{  
   float Bias,kp=300,kd=1;
	 int balance;
	static int i = 0;
	
	//这个是代表传感器没安正，有点歪？
	 Bias=Angle-ZHONGZHI;       //===求出平衡的角度中值 和机械相关
	
	//这行PID控制还真是简单啊，看来难度都在姿态解算和速度控制上了
	 balance=kp*Bias+Gyro*kd;   //===计算平衡控制的电机PWM  PD控制   kp是P系数 kd是D系数 
	
	i++;
	if(i>7)
	{
		i = 0;
		printf("Example	A:%f	G:%f	\n",Angle,Gyro);
	}

	
	 return balance;
}

/**************************************************************************
函数功能：速度PI控制 修改前进后退速度，请修Target_Velocity，比如，改成60就比较慢了
入口参数：左轮编码器、右轮编码器
返回  值：速度控制PWM
作    者：平衡小车之家
**************************************************************************/
int velocity(int encoder_left,int encoder_right)
{  
     static float Velocity,Encoder_Least,Encoder,Movement;
	  static float Encoder_Integral,Target_Velocity;
	  float kp=80,ki=0.4;
	  //=============遥控前进后退部分=======================// 
	/*
	这一段就干了两件事：1.控制前后（正负）
											2.切换速度（Movement的大小）
	*/
	  if(Bi_zhang==1&&Flag_sudu==1)  Target_Velocity=45;                 //如果进入避障模式,自动进入低速模式
    else 	                         Target_Velocity=90;      
	
		if(1==Flag_Qian)    	//前进
			
			Movement=Target_Velocity/Flag_sudu;	         //===前进标志位置1 
		
		else if(1==Flag_Hou)	//后退
			
			Movement=-Target_Velocity/Flag_sudu;         //===后退标志位置1
		
	  else  Movement=0;			//停
	
	  if(Bi_zhang==1&&Distance<500&&Flag_Left!=1&&Flag_Right!=1)        //避障标志位置1且非遥控转弯的时候，进入避障模式
			Movement=-Target_Velocity/Flag_sudu;
		
   //=============速度PI控制器=======================//	
		Encoder_Least =(Encoder_Left+Encoder_Right)-0;                    //==获取最新速度偏差==
																																			// = 测量速度（左右编码器之和）- 目标速度（此处为零） 
		
		//对速度偏差进行低通滤波，这个滤波的滤波程度还挺大的啊！
		//我觉得这个叫速度偏差不大正确，应该叫做平均前进速度，以为左右编码器取的是和
		Encoder *= 0.8;		                                                //===一阶低通滤波器       
		Encoder += Encoder_Least*0.2;	                                    //===一阶低通滤波器    
		
		//记得有一句话，说速度控制是正反馈。跑得快时想慢一点，就要跑的再快点。
		Encoder_Integral +=Encoder;                                       //===积分出位移 积分时间：10ms
		Encoder_Integral=Encoder_Integral-Movement;                       //===接收遥控器数据，控制前进后退
		if(Encoder_Integral>10000)  	Encoder_Integral=10000;             //===积分限幅
		if(Encoder_Integral<-10000)	Encoder_Integral=-10000;              //===积分限幅	
		Velocity=Encoder*kp+Encoder_Integral*ki;                          //===速度控制	
		if(Turn_Off(Angle_Balance,Voltage)==1||Flag_Stop==1)   Encoder_Integral=0;      //===电机关闭后清除积分
	  return Velocity;
}

/**************************************************************************
函数功能：转向控制  修改转向速度，请修改Turn_Amplitude即可
入口参数：左轮编码器、右轮编码器、Z轴陀螺仪
返回  值：转向控制PWM
作    者：平衡小车之家
**************************************************************************/
int turn(int encoder_left,int encoder_right,float gyro)//转向控制
{
	 static float Turn_Target,Turn,Encoder_temp,Turn_Convert=0.9,Turn_Count;
	  float Turn_Amplitude=88/Flag_sudu,Kp=42,Kd=0;     
	  //=============遥控左右旋转部分=======================//
  	if(1==Flag_Left||1==Flag_Right)                      //这一部分主要是根据旋转前的速度调整速度的起始速度，增加小车的适应性
		{
			if(++Turn_Count==1)
			Encoder_temp=myabs(encoder_left+encoder_right);
			Turn_Convert=50/Encoder_temp;
			if(Turn_Convert<0.6)Turn_Convert=0.6;
			if(Turn_Convert>3)Turn_Convert=3;
		}	
	  else
		{
			Turn_Convert=0.9;
			Turn_Count=0;
			Encoder_temp=0;
		}			
		if(1==Flag_Left)	           Turn_Target-=Turn_Convert;
		else if(1==Flag_Right)	     Turn_Target+=Turn_Convert; 
		else Turn_Target=0;
	
    if(Turn_Target>Turn_Amplitude)  Turn_Target=Turn_Amplitude;    //===转向速度限幅
	  if(Turn_Target<-Turn_Amplitude) Turn_Target=-Turn_Amplitude;
		if(Flag_Qian==1||Flag_Hou==1)  Kd=0.5;        
		else Kd=0;   //转向的时候取消陀螺仪的纠正 有点模糊PID的思想
  	//=============转向PD控制器=======================//
		Turn=-Turn_Target*Kp -gyro*Kd;                 //===结合Z轴陀螺仪进行PD控制
	  return Turn;
}

/**************************************************************************
函数功能：赋值给PWM寄存器
入口参数：左轮PWM、右轮PWM
返回  值：无
**************************************************************************/
void Set_Pwm(int moto1,int moto2)
{
    	if(moto1<0)			AIN2=1,			AIN1=0;
			else 	          AIN2=0,			AIN1=1;
			PWMA=myabs(moto1);
		  if(moto2<0)	BIN1=0,			BIN2=1;
			else        BIN1=1,			BIN2=0;
			PWMB=myabs(moto2);	
}

/**************************************************************************
函数功能：限制PWM赋值 
入口参数：无
返回  值：无
**************************************************************************/
void Xianfu_Pwm(void)
{	
	  int Amplitude=6900;    //===PWM满幅是7200 限制在6900
	  if(Flag_Qian==1)  Moto1+=DIFFERENCE;  //DIFFERENCE是一个衡量平衡小车电机和机械安装差异的一个变量。直接作用于输出，让小车具有更好的一致性。
	  if(Flag_Hou==1)   Moto2-=DIFFERENCE;
    if(Moto1<-Amplitude) Moto1=-Amplitude;	
		if(Moto1>Amplitude)  Moto1=Amplitude;	
	  if(Moto2<-Amplitude) Moto2=-Amplitude;	
		if(Moto2>Amplitude)  Moto2=Amplitude;		
	
}
/**************************************************************************
函数功能：按键修改小车运行状态 
入口参数：无
返回  值：无
**************************************************************************/
void Key(void)
{	
	u8 tmp,tmp2;
	tmp=click_N_Double(50); 
	if(tmp==1)Flag_Stop=!Flag_Stop;//单击控制小车的启停
	if(tmp==2)Flag_Show=!Flag_Show;//双击控制小车的显示状态
	tmp2=Long_Press();                   
  if(tmp2==1) Bi_zhang=!Bi_zhang;		//长按控制小车是否进入超声波避障模式 
}

/**************************************************************************
函数功能：异常关闭电机
入口参数：倾角和电压
返回  值：1：异常  0：正常
**************************************************************************/
u8 Turn_Off(float angle, int voltage)
{
	    u8 temp;
			if(angle<-40||angle>40||1==Flag_Stop||voltage<1110)//电池电压低于11.1V关闭电机
			{	                                                 //===倾角大于40度关闭电机
      temp=1;                                            //===Flag_Stop置1关闭电机
			AIN1=0;                                            
			AIN2=0;
			BIN1=0;
			BIN2=0;
      }
			else
      temp=0;
      return temp;			
}
	
/**************************************************************************
函数功能：获取角度 三种算法经过我们的调校，都非常理想 
入口参数：获取角度的算法 1：DMP  2：卡尔曼 3：互补滤波
返回  值：无
**************************************************************************/
void Get_Angle(u8 way)
{ 
	float Accel_Y,Accel_X,Accel_Z,Gyro_Y,Gyro_Z;
	Temperature=Read_Temperature();      //===读取MPU6050内置温度传感器数据，近似表示主板温度。

	//MPU6050的DMP方法获取姿态
	if(way==1)                           //===DMP的读取在数据采集中断提醒的时候，严格遵循时序要求
	{	
			Read_DMP();                      //===读取加速度、角速度、倾角
			Angle_Balance=Pitch;             //===更新平衡倾角
			Gyro_Balance=gyro[1];            //===更新平衡角速度
			Gyro_Turn=gyro[2];               //===更新转向角速度
		  Acceleration_Z=accel[2];         //===更新Z轴加速度计
	}			

	//采用 卡尔曼滤波 或 互补滤波
	else
	{
		//读取所需传感器数值
		Gyro_Y=(I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_YOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_YOUT_L);    //读取Y轴陀螺仪
		Gyro_Z=(I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_ZOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_ZOUT_L);    //读取Z轴陀螺仪

		Accel_X=(I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_XOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_XOUT_L); //读取X轴加速度计
		Accel_Z=(I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_ZOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_ZOUT_L); //读取Z轴加速度计

		if(Gyro_Y>32768)  Gyro_Y-=65536;                       //数据类型转换  也可通过short强制类型转换
		if(Gyro_Z>32768)  Gyro_Z-=65536;                       //数据类型转换
		if(Accel_X>32768) Accel_X-=65536;                      //数据类型转换
		if(Accel_Z>32768) Accel_Z-=65536;                      //数据类型转换

		Gyro_Balance=-Gyro_Y;                                  //更新平衡角速度

		Accel_Y = atan2(Accel_X,Accel_Z)*180/PI;                 //计算倾角

		Gyro_Y=Gyro_Y/16.4;                                    //陀螺仪量程转换	

		//卡尔曼滤波
		if(Way_Angle==2)		  	
		Kalman_Filter(Accel_Y,-Gyro_Y);//卡尔曼滤波	

		//互补滤波
		else if(Way_Angle==3)   
		Yijielvbo(Accel_Y,-Gyro_Y);    //互补滤波


		Angle_Balance=angle;                                   //更新平衡倾角
		Gyro_Turn=Gyro_Z;                                      //更新转向角速度
		Acceleration_Z=Accel_Z;                                //===更新Z轴加速度计	
	}
}
/**************************************************************************
函数功能：绝对值函数
入口参数：int
返回  值：unsigned int
**************************************************************************/
int myabs(int a)
{ 		   
	  int temp;
		if(a<0)  temp=-a;  
	  else temp=a;
	  return temp;
}
/**************************************************************************
函数功能：检测小车是否被拿起
入口参数：int
返回  值：unsigned int
**************************************************************************/
int Pick_Up(float Acceleration,float Angle,int encoder_left,int encoder_right)
{ 		   
	 static u16 flag,count0,count1,count2;
	if(flag==0)                                                                   //第一步
	 {
	      if(myabs(encoder_left)+myabs(encoder_right)<30)                         //条件1，小车接近静止
				count0++;
        else 
        count0=0;		
        if(count0>10)				
		    flag=1,count0=0; 
	 } 
	 if(flag==1)                                                                  //进入第二步
	 {
		    if(++count1>200)       count1=0,flag=0;                                 //超时不再等待2000ms
	      if(Acceleration>26000&&(Angle>(-20+ZHONGZHI))&&(Angle<(20+ZHONGZHI)))   //条件2，小车是在0度附近被拿起
		    flag=2; 
	 } 
	 if(flag==2)                                                                  //第三步
	 {
		  if(++count2>100)       count2=0,flag=0;                                   //超时不再等待1000ms
	    if(myabs(encoder_left+encoder_right)>135)                                 //条件3，小车的轮胎因为正反馈达到最大的转速   
      {
				flag=0;                                                                                     
				return 1;                                                               //检测到小车被拿起
			}
	 }
	return 0;
}
/**************************************************************************
函数功能：检测小车是否被放下
入口参数：int
返回  值：unsigned int
**************************************************************************/
int Put_Down(float Angle,int encoder_left,int encoder_right)
{ 		   
	 static u16 flag,count;	 
	 if(Flag_Stop==0)                           //防止误检      
   return 0;	                 
	 if(flag==0)                                               
	 {
	      if(Angle>(-10+ZHONGZHI)&&Angle<(10+ZHONGZHI)&&encoder_left==0&&encoder_right==0)         //条件1，小车是在0度附近的
		    flag=1; 
	 } 
	 if(flag==1)                                               
	 {
		  if(++count>50)                                          //超时不再等待 500ms
		  {
				count=0;flag=0;
		  }
	    if(encoder_left>3&&encoder_right>3&&encoder_left<60&&encoder_right<60)                //条件2，小车的轮胎在未上电的时候被人为转动  
      {
				flag=0;
				flag=0;
				return 1;                                             //检测到小车被放下
			}
	 }
	return 0;
}


