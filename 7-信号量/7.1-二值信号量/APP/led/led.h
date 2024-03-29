#ifndef _led_H
#define _led_H

#include "system.h"

/*  LED时钟端口、引脚定义 */
#define LED1_PORT 			GPIOB   
#define LED1_PIN 			GPIO_Pin_5
#define LED1_PORT_RCC		RCC_APB2Periph_GPIOB

#define LED2_PORT 			GPIOA   
#define LED2_PIN 			GPIO_Pin_5
#define LED2_PORT_RCC		RCC_APB2Periph_GPIOA


#define LED1 PBout(5)  	
#define LED2 PAout(5)  	

#define LED1_IN PBin(5)  	
#define LED2_IN PAin(5)

void LED_Init(void);


#endif
