#ifndef __MOVEMENT_CONTROL_H
#define	__MOVEMENT_CONTROL_H

#include "stm32f10x.h"

//extern u8 Direction_Left,Rirection_Right;	//1前进，2后退，0停止   改用函数方式提供访问了，不再允许直接访问

void Speed_OutPut(s32 Speed_Left,s32 Speed_Right);
void Get_Speed(u32 * Speed_Left_CM_S,u32 * Speed_Right_CM_S);	//这个速度从码盘获得，所以用u32表示，不涉及方向

#endif /* __MOVEMENT_CONTROL_H */
