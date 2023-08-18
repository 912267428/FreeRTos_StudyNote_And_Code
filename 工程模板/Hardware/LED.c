#include "stm32f10x.h"                  // Device header

/**
  * @brief 初始化LED对应的IO口
  * @param 
  * @retval 
  */
void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能GPIOA的时钟
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief 打开LED1
  * @param 
  * @retval 
  */
void LED1_ON(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

/**
  * @brief 关闭LED1
  * @param 
  * @retval 
  */
void LED1_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

/**
  * @brief 切换LED1
  * @param 
  * @retval 
  */
void LED1_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_1) == 0)
	{
		LED1_OFF();
	}
	else{
		LED1_ON();
	}
}

/**
  * @brief 打开LED2
  * @param 
  * @retval 
  */
void LED2_ON(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}

/**
  * @brief 关闭LED2
  * @param 
  * @retval 
  */
void LED2_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
}

/**
  * @brief 切换LED2
  * @param 
  * @retval 
  */
void LED2_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == 0)
	{
		LED2_OFF();
	}
	else{
		LED2_ON();
	}
}
