#ifndef _YCKey_H
#define _YCKey_H

#include "system.h"

/*  LEDʱ�Ӷ˿ڡ����Ŷ��� */
#define YCKey_PORT 			GPIOB   
#define YCKey_PIN 			GPIO_Pin_11
#define YCKey_PORT_RCC		RCC_APB2Periph_GPIOB


void YCKey_Init(void);
u8 YCKey_GetNum(void);


#endif
