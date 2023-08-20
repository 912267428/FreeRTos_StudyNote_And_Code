#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"


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
#define Led_TASK_PRIO		2
//�����ջ��С	
#define Led_STK_SIZE 		50  
//������
TaskHandle_t LedTask_Handler;
//������
void Led_task(void *pvParameters);

/*******************************************************************************
* �� �� ��         : main
* ��������		   : ������
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
int main()
{
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4

	USART1_Init(9600);
	YCKey_Init();
	LED_Init();
	
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
				
	//����Զ�̰������ڷ�������
    xTaskCreate((TaskFunction_t )Led_task,     
                (const char*    )"Led_task",   
                (uint16_t       )Led_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Led_TASK_PRIO,
                (TaskHandle_t*  )&LedTask_Handler);
				
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
			printf("����LED����");
			vTaskSuspend(LedTask_Handler);/* ����LED���� */
			printf("����LED����ɹ���\r\n");
		}
		else if(KeyNum == KEY1_PRESS)
		{
			printf("�ָ�LED����\n");
			vTaskResume(LedTask_Handler);/* �ָ�LED����*/
			printf("�ָ�LED����ɹ���\r\n");
		}
		vTaskDelay(20);
	}
}

void Led_task(void *pvParameters)
{
	while(1)
    {
        LED1=0;
		LED2=0;
        vTaskDelay(200);
        LED1=1;
		LED2=1;
        vTaskDelay(800);
    }
}
