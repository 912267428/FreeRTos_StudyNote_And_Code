#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "event_groups.h"


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
#define Led_TASK_PRIO		3
//任务堆栈大小	
#define Led_STK_SIZE 		50  
//任务句柄
TaskHandle_t LedTask_Handler;
//任务函数
void Led_task(void *pvParameters);

EventGroupHandle_t Event_Handle=NULL;	//初始化需要创建的事件的句柄
#define Event_0 (0x01<<0)				//定义事件0 ：按键0按下
#define Event_1 (0x01<<1)				//定义事件1 ：按键1按下


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
	
	LED1 = 0;
	LED2 = 1;
	printf("FreeRTOS事件实验\r\n");
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
	
	Event_Handle = xEventGroupCreate();
	if (NULL != Event_Handle)
		printf("事件创建成功\r\n");
	else
		printf("事件创建失败\r\n");
      
    //创建OLED任务
    xTaskCreate((TaskFunction_t )oled_task,     
                (const char*    )"oled_task",   
                (uint16_t       )OLED_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )OLED_TASK_PRIO,
                (TaskHandle_t*  )&OLEDTask_Handler);
				
	//创建远程按键串口发送任务，同时用于发生事件
    xTaskCreate((TaskFunction_t )key_task,     
                (const char*    )"key_task",   
                (uint16_t       )Key_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Key_TASK_PRIO,
                (TaskHandle_t*  )&KeyTask_Handler); 
	
	//创建led任务,用于等待事件
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
		if(i%20 == 0)
			OLED_ShowNum(2,1,i/20,10);
		vTaskDelay(50);
		i++;
		LED2 = !LED2;
    }
}

void key_task(void *pvParameters)
{
	u8 KeyNum = 0;
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum == KEY0_PRESS)
		{
			printf("KEY0_PRESSED\r\n");
			xEventGroupSetBits(Event_Handle, Event_0);
			
		}
		else if(KeyNum == KEY1_PRESS)
		{
			printf("KEY1_PRESSED\r\n");
			xEventGroupSetBits(Event_Handle, Event_1);
		}
		vTaskDelay(20);
	}
}

void Led_task(void *pvParameters)
{
	//事件接收变量
	EventBits_t r_event;
	
	while(1)
	{
		r_event = xEventGroupWaitBits(Event_Handle, (Event_0 | Event_1), pdTRUE, pdTRUE, portMAX_DELAY);
		//需要对事件等待函数的返回值进行检查
		if (r_event & (Event_0 | Event_1) == (Event_0 | Event_1))
		{
			printf("两个按键均按下\r\n");
			LED1 = !LED1;
		}
		else
			printf("事件错误\r\n");
	}
}
