#include "YCKey.h"

extern void vTaskDelay( const uint32_t xTicksToDelay );

/*******************************************************************************
* �� �� ��         : YCKey_Init
* ��������		   : ������Ƶ433M������ʼ������
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void YCKey_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//����ṹ�����
	
	RCC_APB2PeriphClockCmd(YCKey_PORT_RCC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=YCKey_PIN;  //ѡ����Ҫ���õ�IO��
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPD;	 //�����������ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //���ô�������
	GPIO_Init(YCKey_PORT,&GPIO_InitStructure); 	   /* ��ʼ��GPIO */
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




