#include "YCKey.h"
#include "SysTick.h"

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
	
	GPIO_InitStructure.GPIO_Pin=YCKey0_PIN | YCKey1_PIN;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPD;	 //设置下拉输出模式
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(YCKey_PORT,&GPIO_InitStructure); 	   /* 初始化GPIO */
}

u8 YCKey_GetNum(u8 mode)
{
	static u8 key=1;
	
	if(mode==1) //连续按键按下
		key=1;
	if (key==1&&(KEY0==1||KEY1==1))	//任意按键按下
	{
		delay_ms(10);
		key = 0;
		if (KEY0 == 1)
			return KEY0_PRESS;
		else if(KEY1 == 1)
			return KEY1_PRESS;
		//如需要增加按键个数在此添加代码
		
		//
	}
	else if (!(KEY0==1||KEY1==1)) //无按键按下
		key = 1;
	return KEY_NO;
}




