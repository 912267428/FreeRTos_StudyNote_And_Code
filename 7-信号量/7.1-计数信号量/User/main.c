#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "Semphr.h"


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
#define Key_TASK_PRIO		1
//任务堆栈大小	
#define Key_STK_SIZE 		50  
//任务句柄
TaskHandle_t KeyTask_Handler;
//任务函数
void key_task(void *pvParameters);

//任务优先级
#define SEND_TASK_PRIO		4
//任务堆栈大小	
#define SEND_STK_SIZE 		50  
//任务句柄
TaskHandle_t SendTask_Handler;
//任务函数
void send_task(void *pvParameters);

SemaphoreHandle_t CountSem_Handle =NULL;  //初始化计数信号量句柄

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
	printf("计数信号量实验\r\n");
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
	
	/* 创建 CountSem */
	CountSem_Handle = xSemaphoreCreateCounting(5,5); 
      
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
				
	//创建接收任务
    xTaskCreate((TaskFunction_t )send_task,     
                (const char*    )"Send_task",   
                (uint16_t       )SEND_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )SEND_TASK_PRIO,
                (TaskHandle_t*  )&SendTask_Handler);
				
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
		LED2 = !LED2;
    }
}

void key_task(void *pvParameters)
{
	u8 KeyNum = 0;
	BaseType_t xReturn = pdPASS;
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if(KeyNum==KEY1_PRESS)
		{
			xReturn = xSemaphoreTake( CountSem_Handle,0 );//获取计数信号量
			if( xReturn == pdTRUE )
				printf( "释放1个停车位。\r\n" );
			else
				printf( "已无车位可以释放！\r\n" );
		}
		vTaskDelay(20);
	}
}

void send_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
	
	u8 KeyNum = 0;
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum == KEY0_PRESS)
		{
			xReturn = xSemaphoreGive( CountSem_Handle );//给出计数信号量
			if( xReturn == pdTRUE )
				printf("成功申请到停车位。\r\n");
			else
				printf("不好意思，现在停车场已满！\r\n");
		}
		vTaskDelay(20);
	}
}
