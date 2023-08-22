#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "semphr.h"
#include "led.h"

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define OLED_TASK_PRIO		2
//任务堆栈大小	
#define OLED_STK_SIZE 		50  
//任务句柄
TaskHandle_t OLEDTask_Handler;
//任务函数
void oled_task(void *pvParameters);

//任务优先级
#define Key_TASK_PRIO		4
//任务堆栈大小	
#define Key_STK_SIZE 		50  
//任务句柄
TaskHandle_t KeyTask_Handler;
//任务函数
void key_task(void *pvParameters);

//任务优先级
#define Receive_TASK_PRIO		3
//任务堆栈大小	
#define Receive_STK_SIZE 		50  
//任务句柄
TaskHandle_t ReceiveTask_Handler;
//任务函数
void receive_task(void *pvParameters);

SemaphoreHandle_t BinarySem_Handle = NULL;  //初始化二值信号量的句柄
//实际上就是QueueHandle_t因为与消息队列公用一个队列控制块     typedef QueueHandle_t SemaphoreHandle_t;

/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
int main()
{
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	
	vTaskSuspendAll();  //暂停所有任务
	USART1_Init(9600);
	YCKey_Init();
	LED_Init();

	printf("二值信号量实验\r\n");
	
	xTaskResumeAll(); //开始所有任务
	
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
	
	/* 创建二值信号量*/
	BinarySem_Handle = xSemaphoreCreateBinary();
      
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
				
	//创建接收信号量的任务
    xTaskCreate((TaskFunction_t )receive_task,     
                (const char*    )"Receive_task_task",   
                (uint16_t       )Receive_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Receive_TASK_PRIO,
                (TaskHandle_t*  )&ReceiveTask_Handler);
				
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
		OLED_ShowNum(2,1,i,10);
		vTaskDelay(1000);
		i++;
		LED2 = !LED2_IN;
    }
}

void key_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	u8 KeyNum = 0;
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum != KEY_NO)
		{
			xReturn = xSemaphoreGive(BinarySem_Handle);
			if(pdTRUE == xReturn)
				printf("二值型号量释放成功\r\n");
			else
				printf("二值型号量释放失败\r\n");
		}
		vTaskDelay(50);
	}
}

void receive_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
	
	while(1)
	{
		xReturn = xSemaphoreTake(BinarySem_Handle,portMAX_DELAY);
		if(xReturn == pdTRUE)
		{
			printf("二值信号量获取成功\r\n\r\n");
			LED1 = !LED1_IN;
		}
	}
}
