#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "Semphr.h"


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
#define Key_TASK_PRIO		1
//�����ջ��С	
#define Key_STK_SIZE 		50  
//������
TaskHandle_t KeyTask_Handler;
//������
void key_task(void *pvParameters);

//�������ȼ�
#define SEND_TASK_PRIO		4
//�����ջ��С	
#define SEND_STK_SIZE 		50  
//������
TaskHandle_t SendTask_Handler;
//������
void send_task(void *pvParameters);

SemaphoreHandle_t CountSem_Handle =NULL;  //��ʼ�������ź������

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
	printf("�����ź���ʵ��\r\n");
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
	
	/* ���� CountSem */
	CountSem_Handle = xSemaphoreCreateCounting(5,5); 
      
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
				
	//������������
    xTaskCreate((TaskFunction_t )send_task,     
                (const char*    )"Send_task",   
                (uint16_t       )SEND_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )SEND_TASK_PRIO,
                (TaskHandle_t*  )&SendTask_Handler);
				
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
			xReturn = xSemaphoreTake( CountSem_Handle,0 );//��ȡ�����ź���
			if( xReturn == pdTRUE )
				printf( "�ͷ�1��ͣ��λ��\r\n" );
			else
				printf( "���޳�λ�����ͷţ�\r\n" );
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
			xReturn = xSemaphoreGive( CountSem_Handle );//���������ź���
			if( xReturn == pdTRUE )
				printf("�ɹ����뵽ͣ��λ��\r\n");
			else
				printf("������˼������ͣ����������\r\n");
		}
		vTaskDelay(20);
	}
}
