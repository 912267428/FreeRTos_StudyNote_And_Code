#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "semphr.h"

#define 


//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define OLED_TASK_PRIO		30
//任务堆栈大小	
#define OLED_STK_SIZE 		50  
//任务句柄
TaskHandle_t OLEDTask_Handler;
//任务函数
void oled_task(void *pvParameters);

//L优先级任务
void L_task(void *pvParameters);
#define L_STK_SIZE			50
#define L_TASK_PRIO			2
TaskHandle_t LTask_Handler;

//M优先级任务
void M_task(void *pvParameters);
#define M_STK_SIZE			50
#define M_TASK_PRIO			3
TaskHandle_t MTask_Handler;

//H优先级任务
void H_task(void *pvParameters);
#define H_STK_SIZE			50
#define H_TASK_PRIO			4
TaskHandle_t HTask_Handler;

SemaphoreHandle_t MuxSem_Handle = NULL;

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
	printf("FreeRTOS互斥量实验\r\n");
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
	
	MuxSem_Handle = xSemaphoreCreateMutex();
      
    //创建OLED任务
    xTaskCreate((TaskFunction_t )oled_task,     
                (const char*    )"oled_task",   
                (uint16_t       )OLED_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )OLED_TASK_PRIO,
                (TaskHandle_t*  )&OLEDTask_Handler);
	
	xTaskCreate((TaskFunction_t )L_task,
				(const char*	)"L_task",
				(uint16_t		)L_STK_SIZE,
				(void *			)NULL,
				(UBaseType_t	)L_TASK_PRIO,
				(TaskHandle_t*	)&LTask_Handler);
	
	xTaskCreate((TaskFunction_t )M_task,
				(const char*	)"M_task",
				(uint16_t		)M_STK_SIZE,
				(void *			)NULL,
				(UBaseType_t	)M_TASK_PRIO,
				(TaskHandle_t*	)&MTask_Handler);
				
	xTaskCreate((TaskFunction_t )H_task,
				(const char*	)"H_task",
				(uint16_t		)H_STK_SIZE,
				(void *			)NULL,
				(UBaseType_t	)H_TASK_PRIO,
				(TaskHandle_t*	)&HTask_Handler);
				
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

void L_task(void *pvParameters)
{
	u32 i;
	BaseType_t xReturn = pdTRUE;
	while(1)
	{
		OLED_ShowString(3,1,"L");
		printf("低优先级任务获取互斥量\r\n");
		xReturn = xSemaphoreTake(MuxSem_Handle,portMAX_DELAY);
		if (xReturn == pdTRUE)
			printf("低优先级任务正在运行\r\n");
		for(i=0;i<1000000;i++)
		{
			taskYIELD();
		}
		
		printf("低优先级任务释放互斥量!\r\n\r\n");
		xSemaphoreGive(MuxSem_Handle);
		vTaskDelay(500);
	}
}

void M_task(void *pvParameters)
{
	
	while(1)
	{
		OLED_ShowString(3,1,"M");
		printf("中优先级任务正在运行\r\n");

		vTaskDelay(500);
	}
}

void H_task(void *pvParameters)
{
	BaseType_t xReturn = pdTRUE;
	while(1)
	{
		OLED_ShowString(3,1,"G");
		printf("高优先级任务获取互斥量\r\n");
		xReturn = xSemaphoreTake(MuxSem_Handle,portMAX_DELAY);
		if (xReturn == pdTRUE)
			printf("高优先级任务正在运行\r\n");
		
		printf("高优先级任务释放互斥量!\r\n\r\n");
		xSemaphoreGive(MuxSem_Handle);
		vTaskDelay(500);
	}
}
