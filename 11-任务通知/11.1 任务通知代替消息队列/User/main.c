#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "limits.h"


//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define OLED_TASK_PRIO		1
//任务堆栈大小	
#define OLED_STK_SIZE 		50  
//任务句柄
TaskHandle_t OLEDTask_Handler;
//任务函数
void oled_task(void *pvParameters);

//任务优先级
#define Key_TASK_PRIO		3
//任务堆栈大小	
#define Key_STK_SIZE 		50  
//任务句柄
TaskHandle_t KeyTask_Handler;
//任务函数
void key_task(void *pvParameters);

//任务优先级
#define Receive1_TASK_PRIO		2
//任务堆栈大小	
#define Receive1_STK_SIZE 		512  
//任务句柄
TaskHandle_t Receive1Task_Handler;
//任务函数
void Receive1_task(void *pvParameters);

//任务优先级
#define Receive2_TASK_PRIO		2
//任务堆栈大小	
#define Receive2_STK_SIZE 		512  
//任务句柄
TaskHandle_t Receive2Task_Handler;
//任务函数
void Receive2_task(void *pvParameters);

#define  USE_CHAR  0  /* 测试字符串的时候配置为 1 ，测试变量配置为 0  */

/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
int main()
{
	vTaskSuspendAll();
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4

	USART1_Init(9600);
	YCKey_Init();
	LED_Init();
	printf("FreeRTOS任务通知代替消息队列实验\r\n");
	xTaskResumeAll();
	
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
      
    //创建OLED任务
    xTaskCreate((TaskFunction_t )oled_task,     
                (const char*    )"oled_task",   
                (uint16_t       )OLED_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )OLED_TASK_PRIO,
                (TaskHandle_t*  )&OLEDTask_Handler);
				
	//创建远程按键串口发送任务
    xTaskCreate((TaskFunction_t )key_task,     
                (const char*    )"key_task",   
                (uint16_t       )Key_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Key_TASK_PRIO,
                (TaskHandle_t*  )&KeyTask_Handler);

	//创建Receive1任务
    xTaskCreate((TaskFunction_t )Receive1_task,     
                (const char*    )"Receive1_task",   
                (uint16_t       )Receive1_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Receive1_TASK_PRIO,
                (TaskHandle_t*  )&Receive1Task_Handler);
				
	//创建Receive2任务
    xTaskCreate((TaskFunction_t )Receive2_task,     
                (const char*    )"Receive2_task",   
                (uint16_t       )Receive2_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Receive2_TASK_PRIO,
                (TaskHandle_t*  )&Receive2Task_Handler);		
				
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
} 

//OLED任务函数
void oled_task(void *pvParameters)
{
	u32 i=0;
	OLED_Init();
	OLED_ShowString(1,1,"FreeRTOS");
    while(1)
    {
		if(i%20 == 0)
			OLED_ShowNum(2,1,i/20,10);
		vTaskDelay(50);
		i++;
		LED2 = !LED2;
    }
}

void key_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
	u8 KeyNum = 0;
#if USE_CHAR
	char test_str1[] = "this is a mail test 1";
	char test_str2[] = "this is a mail test 2";
#else
	uint32_t send1 = 1;
	uint32_t send2 = 2;
#endif
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum == KEY0_PRESS)
		{
			xReturn = xTaskNotify(Receive1Task_Handler,
#if USE_CHAR
			(uint32_t)&test_str1,
#else
			send1,
#endif
			eSetValueWithOverwrite);
			if(xReturn == pdPASS)
				printf("任务一的任务通知消息发送成功\r\n");
			
		}
		else if(KeyNum == KEY1_PRESS)
		{
			xReturn = xTaskNotify(Receive2Task_Handler,
#if USE_CHAR
			(uint32_t)&test_str2,
#else
			send2,
#endif
			eSetValueWithOverwrite);
			if(xReturn == pdPASS)
				printf("任务二的任务通知消息发送成功\r\n");
		}
		vTaskDelay(20);
	}
}

void Receive1_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
#if USE_CHAR
	char *r_char;
#else
	uint32_t r_num;
#endif
	
	while(1)
	{
		xReturn = xTaskNotifyWait(0x00,ULONG_MAX,  //退出函数的时候清楚所有bits
#if USE_CHAR
		(uint32_t *)&r_char,
#else
		&r_num,
#endif
		portMAX_DELAY);
		if(pdTRUE == xReturn)
#if USE_CHAR
		printf("任务一收到的消息为:%S \r\n", r_char);
#else
		printf("任务一收到的消息为:%d \r\n", r_num);
#endif
	}
}

void Receive2_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
#if USE_CHAR
	char *r_char;
#else
	uint32_t r_num;
#endif
	
	while(1)
	{
		xReturn = xTaskNotifyWait(0x00,ULONG_MAX,  //退出函数的时候清楚所有bits
#if USE_CHAR
		(uint32_t *)&r_char,
#else
		&r_num,
#endif
		portMAX_DELAY);
		if(pdTRUE == xReturn)
#if USE_CHAR
		printf("任务二收到的消息为:%S \r\n", r_char);
#else
		printf("任务二收到的消息为:%d \r\n", r_num);
#endif
	}
}
