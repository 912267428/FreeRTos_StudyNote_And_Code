#ifndef _YCKey_H
#define _YCKey_H

#include "system.h"

/*  LEDʱ�Ӷ˿ڡ����Ŷ��� */
#define YCKey_PORT 			GPIOB
#define YCKey0_PIN 			GPIO_Pin_10
#define YCKey1_PIN 			GPIO_Pin_11
#define YCKey_PORT_RCC		RCC_APB2Periph_GPIOB

//ʹ��λ��������
#define KEY0 	PBin(10)
#define KEY1 	PBin(11)

//�����������ֵ
#define KEY_NO			0
#define KEY0_PRESS		1
#define KEY1_PRESS		2

void YCKey_Init(void);
u8 YCKey_GetNum(u8 mode);


#endif
