#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"


//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define OLED_TASK_PRIO		1
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

//�������ȼ�
#define Receive1_TASK_PRIO		2
//�����ջ��С	
#define Receive1_STK_SIZE 		512  
//������
TaskHandle_t Receive1Task_Handler;
//������
void Receive1_task(void *pvParameters);

//�������ȼ�
#define Receive2_TASK_PRIO		2
//�����ջ��С	
#define Receive2_STK_SIZE 		512  
//������
TaskHandle_t Receive2Task_Handler;
//������
void Receive2_task(void *pvParameters);

#define  USE_CHAR  1  /* �����ַ�����ʱ������Ϊ 1 �����Ա�������Ϊ 0  */

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
	printf("FreeRTOS����֪ͨ������Ϣ����ʵ��\r\n");
	LED1 = 0;
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

	//����Receive1����
    xTaskCreate((TaskFunction_t )Receive1_task,     
                (const char*    )"Receive1_task",   
                (uint16_t       )Receive1_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Receive1_TASK_PRIO,
                (TaskHandle_t*  )&Receive1Task_Handler);
				
	//����Receive2����
    xTaskCreate((TaskFunction_t )Receive2_task,     
                (const char*    )"Receive2_task",   
                (uint16_t       )Receive2_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Receive2_TASK_PRIO,
                (TaskHandle_t*  )&Receive2Task_Handler);		
				
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
	BaseType_t xReturn = pdPASS;
	u8 KeyNum = 0;
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum == KEY0_PRESS)
		{
			xReturn = xTaskNotifyGive(Receive1Task_Handler);
			if( xReturn == pdTRUE )
				printf("����֪ͨ1���ͳɹ�!\r\n");			
		}
		else if(KeyNum == KEY1_PRESS)
		{
			xReturn = xTaskNotifyGive(Receive2Task_Handler);
			if( xReturn == pdTRUE )
				printf("����֪ͨ2���ͳɹ�!\r\n");	
		}
		vTaskDelay(20);
	}
}

void Receive1_task(void *pvParameters)
{
	while(1)
	{
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		printf("������1��ȡ����֪ͨ�ɹ�\r\n");
		LED1 = !LED1;
	}
}

void Receive2_task(void *pvParameters)
{
	while(1)
	{
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		printf("������2��ȡ����֪ͨ�ɹ�\r\n");
		LED1 = !LED1;
	}
}
