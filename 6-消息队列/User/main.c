#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "queue.h"
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
#define OLED_TASK_PRIO		1
//任务堆栈大小	
#define OLED_STK_SIZE 		50  
//任务句柄
TaskHandle_t OLEDTask_Handler;
//任务函数
void oled_task(void *pvParameters);

//任务优先级		检测按键任务，不同按键发送不同的消息
#define Key_TASK_PRIO		1
//任务堆栈大小	
#define Key_STK_SIZE 		50  
//任务句柄
TaskHandle_t KeyTask_Handler;
//任务函数
void key_task(void *pvParameters);

//任务优先级		LED任务，将受到的消息通过串口发送给电脑，并根据消息切换LED状态
#define Led_TASK_PRIO		1
//任务堆栈大小	
#define Led_STK_SIZE 		50  
//任务句柄
TaskHandle_t LedTask_Handler;
//任务函数
void Led_task(void *pvParameters);

QueueHandle_t Test_Queue =NULL;
#define QUEUE_LEN	4
#define QUEUE_SIZE	4
#define QUEUE_DATA1 0   //翻转LED
#define QUEUE_DATA2 1	//进入or推出LED闪烁模式

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
	
	LED_Init();
	USART1_Init(115200);
	YCKey_Init();
	
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
	
	//创建消息队列
	Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN, (UBaseType_t ) QUEUE_SIZE);
      
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
				
	//创建LED任务
    xTaskCreate((TaskFunction_t )Led_task,     
                (const char*    )"led_task",   
                (uint16_t       )Led_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Led_TASK_PRIO,
                (TaskHandle_t*  )&LedTask_Handler);
				
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
    }
}

void key_task(void *pvParameters)
{
	u8 KeyNum = 0;
	BaseType_t xReturn = pdPASS;
	u32 send_data1 = QUEUE_DATA1;
	u32 send_data2 = QUEUE_DATA2;
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum == KEY0_PRESS)
		{
			//检测到按键0按下，发送send_data1到消息队列
			printf("发送消息send_data1\r\n");
			xReturn = xQueueSend(Test_Queue,&send_data1,0);
			if(pdPASS == xReturn)
				printf("消息send_data1发送成功\r\n");
		}
		else if(KeyNum == KEY1_PRESS)
		{
			printf("发送消息send_data2\r\n");
			xReturn = xQueueSend( Test_Queue, &send_data2, 0 );       
			if(pdPASS == xReturn)
				printf("消息send_data2发送成功\r\n");
		}
		vTaskDelay(20);
	}
}

void Led_task(void *pvParameters)
{
	BaseType_t xReturn = pdTRUE;
	u32 r_queue;
	
	LED1 = 0;
	LED2 = 0;
	u8 mode = 0;  //默认为0（常亮或长灭）；1（闪烁）

	while(1)
	{
		xReturn = xQueueReceive(Test_Queue, &r_queue, 0);  //不等
		if(xReturn == pdTRUE)
		{
			printf("接收到消息\r\n");
			if (r_queue == QUEUE_DATA1 && mode == 0)	//翻转亮灭状态
			{
				LED1 = ! LED1_in;
				LED2 = ! LED2_in;
			}
			if (r_queue == QUEUE_DATA2)					//开始或退出闪烁
			{
				mode = !mode;
				if(mode == 0)
				{
					LED1 = 0;
					LED2 = 0;
				}
			}	
		}
		if(mode == 1)
		{
			LED1 = ! LED1_in;
			LED2 = ! LED2_in;
		}
		
		vTaskDelay(20);
	}
}
