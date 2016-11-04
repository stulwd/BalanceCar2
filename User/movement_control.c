#include "movement_control.h"
#include "bsp_encoder.h"
#include "bsp_motor.h"
#include "stdlib.h"

u8 Direction_Left = 0,Rirection_Right = 0;

void Speed_OutPut(s32 Speed_Left,s32 Speed_Right)
{
	//车轮转向判断
	if(Speed_Left > 0)
		Direction_Left = 1;
	else if(Speed_Left < 0)
		Direction_Left = 2;
	else 
		Direction_Left = 0;
	
	if(Speed_Right > 0)
		Rirection_Right = 1;
	else if(Speed_Right < 0)
		Rirection_Right = 2;
	else
		Rirection_Right = 0;
	
	Speed_Left = abs(Speed_Left);
	Speed_Right = abs(Speed_Right);
	
	Left_Motor(Direction_Left,Speed_Left);
	Right_Motor(Rirection_Right,Speed_Right);
}

void Get_Speed(u32 * Speed_Left_CM_S,u32 * Speed_Right_CM_S)	//这个速度从码盘获得，所以用u32表示，不涉及方向
{
	*Speed_Left_CM_S = Get_Left_Speed();	//获取转速
	*Speed_Right_CM_S = Get_Right_Speed();
}

void Get_Direction(u8 * Direction_Left_out,u8 * Direction_Right_out)	//获取方向，0停止 1向前 2向后
{
	(*Direction_Left_out)  = Direction_Left;
	(*Direction_Right_out) = Rirection_Right;
}

