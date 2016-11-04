#include "stm32f10x.h"
#include "init.h"
#include "scheduler.h"

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int init_error;

int main(void)
{	
	init_error = All_Init();
	
	if(init_error)
		printf("硬件初始化存在问题\n");	
	else
		printf("硬件初始化正常\n");	

	while (1)
	{	 
		Duty_Loop();
	}

}
/*********************************************END OF FILE**********************/
