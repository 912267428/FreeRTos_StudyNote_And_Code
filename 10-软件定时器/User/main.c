#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "timers.h"


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
#define Key_TASK_PRIO		3
//任务堆栈大小	
#define Key_STK_SIZE 		50  
//任务句柄
TaskHandle_t KeyTask_Handler;
//任务函数
void key_task(void *pvParameters);

TimerHandle_t Timer_1=NULL;
TimerHandle_t Timer_2=NULL;

static uint32_t TmrCb_Count1 = 0; /* 记录软件定时器1回调函数执行次数 */
static uint32_t TmrCb_Count2 = 0; /* 记录软件定时器2回调函数执行次数 */

static void Swtmr1_Callback(void* parameter);
static void Swtmr2_Callback(void* parameter);

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
	printf("FreeRTOS软件定时器实验\r\n");
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
	
	//创建第一个软件定时器周期模式
	Timer_1 = xTimerCreate((const char* 	) "AutoReloadTimer",
							(TickType_t		) 1000,
							(UBaseType_t	) pdTRUE,
							(void*			) 1,
							(TimerCallbackFunction_t) Swtmr1_Callback);
	if (NULL != Timer_1)
	{
		xTimerStart(Timer_1, 0);
	}
	
	//创建第二个软件定时器 单次模式
	Timer_2 = xTimerCreate((const char* 	) "OneShotTimer",
							(TickType_t		) 5000,
							(UBaseType_t	) pdFALSE,
							(void*			) 2,
							(TimerCallbackFunction_t) Swtmr2_Callback);
	if (NULL != Timer_2)
	{
		xTimerStart(Timer_2, 0);
	}
      
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
		}
		else if(KeyNum == KEY1_PRESS)
		{
			printf("KEY1_PRESSED\r\n");
		}
		vTaskDelay(20);
	}
}

void Swtmr1_Callback(void* parameter)
{
	TickType_t tick_num1;
	
	TmrCb_Count1++;
	
	tick_num1 = xTaskGetTickCount();
	
	LED1 = !LED1;
	
	printf("定时器1的回调函数执行%d次\r\n", TmrCb_Count1);
	
	printf("滴答定时器数值=%d\n\r\n", tick_num1);
}
void Swtmr2_Callback(void* parameter)
{
	TickType_t tick_num2;

	TmrCb_Count2++;						

	tick_num2 = xTaskGetTickCount();

	printf("定时器2的回调函数执行 %d 次\r\n", TmrCb_Count2);
	printf("滴答定时器数值=%d\r\n", tick_num2);
}
