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

uint8_t *Test_Ptr = NULL;		//测试内存申请的指针

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
	printf("FreeRTOS内存管理实验\r\n");
	LED1 = 0;
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
	uint32_t g_memsize;
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum == KEY0_PRESS)
		{
			if(NULL == Test_Ptr)
			{
				//获取当前内存大小
				g_memsize = xPortGetFreeHeapSize();
				printf("当前内存块大小为：%d，开始申请内存\r\n", g_memsize);
				Test_Ptr = pvPortMalloc(1024);
				if(NULL != Test_Ptr)
				{
					printf("内存申请成功\r\n");
					printf("申请到的地址为：%#x\r\n",(int)Test_Ptr);
					
					//获取当前内存大小
					g_memsize = xPortGetFreeHeapSize();
					printf("当前内存块大小为：%d，开始申请内存\r\n", g_memsize);
					
					//向申请到的内存中写入数据
					sprintf((char*)Test_Ptr,"当前系统TickCount = %d \r\n",xTaskGetTickCount());
					printf("写入的数据是 %s \r\n",(char*)Test_Ptr);
				}
			}
			else
					printf("请先按下KEY1释放内存再申请\r\n");
		}
		else if(KeyNum == KEY1_PRESS)
		{
			if(NULL != Test_Ptr)
			{
				printf("释放内存\n");
				vPortFree(Test_Ptr);//释放内存
				Test_Ptr=NULL;
				/* 获取当前内剩余存大小 */
				g_memsize = xPortGetFreeHeapSize();
				printf("系统当前内存大小为 %d 字节，内存释放完成\r\n",g_memsize);
			}
			else
				printf("请先按下KEY0申请内存再释放\r\n");
		}
		vTaskDelay(20);
	}
}
