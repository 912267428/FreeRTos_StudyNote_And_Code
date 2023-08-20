#include "YCKey.h"

extern void vTaskDelay( const uint32_t xTicksToDelay );

/*******************************************************************************
* 函 数 名         : YCKey_Init
* 函数功能		   : 无线射频433M按键初始化函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void YCKey_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//定义结构体变量
	
	RCC_APB2PeriphClockCmd(YCKey_PORT_RCC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=YCKey_PIN;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPD;	 //设置下拉输出模式
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(YCKey_PORT,&GPIO_InitStructure); 	   /* 初始化GPIO */
}

u8 YCKey_GetNum(void)
{
	u8 KeyNum = 0;
	if (GPIO_ReadInputDataBit(YCKey_PORT, YCKey_PIN) == 1)
	{
		vTaskDelay(20);
		while (GPIO_ReadInputDataBit(YCKey_PORT, YCKey_PIN) == 1);
		vTaskDelay(20);
		KeyNum = 1;
	}
	
	return KeyNum;
}




