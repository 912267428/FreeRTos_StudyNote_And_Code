#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "timers.h"


//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define OLED_TASK_PRIO		2
//�����ջ��С	
#define OLED_STK_SIZE 		50  
//������
TaskHandle_t OLEDTask_Handler;
//������
void oled_task(void *pvParameters);

//�������ȼ�
#define Key_TASK_PRIO		3
//�����ջ��С	
#define Key_STK_SIZE 		50  
//������
TaskHandle_t KeyTask_Handler;
//������
void key_task(void *pvParameters);

TimerHandle_t Timer_1=NULL;
TimerHandle_t Timer_2=NULL;

static uint32_t TmrCb_Count1 = 0; /* ��¼�����ʱ��1�ص�����ִ�д��� */
static uint32_t TmrCb_Count2 = 0; /* ��¼�����ʱ��2�ص�����ִ�д��� */

static void Swtmr1_Callback(void* parameter);
static void Swtmr2_Callback(void* parameter);

/*******************************************************************************
* �� �� ��         : main
* ��������		   : ������
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
int main()
{
	vTaskSuspendAll();
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4

	USART1_Init(9600);
	YCKey_Init();
	LED_Init();
	printf("FreeRTOS�����ʱ��ʵ��\r\n");
	xTaskResumeAll();
	
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
	
	//������һ�������ʱ������ģʽ
	Timer_1 = xTimerCreate((const char* 	) "AutoReloadTimer",
							(TickType_t		) 1000,
							(UBaseType_t	) pdTRUE,
							(void*			) 1,
							(TimerCallbackFunction_t) Swtmr1_Callback);
	if (NULL != Timer_1)
	{
		xTimerStart(Timer_1, 0);
	}
	
	//�����ڶ��������ʱ�� ����ģʽ
	Timer_2 = xTimerCreate((const char* 	) "OneShotTimer",
							(TickType_t		) 5000,
							(UBaseType_t	) pdFALSE,
							(void*			) 2,
							(TimerCallbackFunction_t) Swtmr2_Callback);
	if (NULL != Timer_2)
	{
		xTimerStart(Timer_2, 0);
	}
      
    //����OLED����
    xTaskCreate((TaskFunction_t )oled_task,     
                (const char*    )"oled_task",   
                (uint16_t       )OLED_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )OLED_TASK_PRIO,
                (TaskHandle_t*  )&OLEDTask_Handler);
				
	//����Զ�̰������ڷ�������
    xTaskCreate((TaskFunction_t )key_task,     
                (const char*    )"key_task",   
                (uint16_t       )Key_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Key_TASK_PRIO,
                (TaskHandle_t*  )&KeyTask_Handler); 
				
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
} 

//OLED������
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
	
	printf("��ʱ��1�Ļص�����ִ��%d��\r\n", TmrCb_Count1);
	
	printf("�δ�ʱ����ֵ=%d\n\r\n", tick_num1);
}
void Swtmr2_Callback(void* parameter)
{
	TickType_t tick_num2;

	TmrCb_Count2++;						

	tick_num2 = xTaskGetTickCount();

	printf("��ʱ��2�Ļص�����ִ�� %d ��\r\n", TmrCb_Count2);
	printf("�δ�ʱ����ֵ=%d\r\n", tick_num2);
}
