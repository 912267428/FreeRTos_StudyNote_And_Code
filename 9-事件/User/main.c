#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "event_groups.h"


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
#define Key_TASK_PRIO		4
//�����ջ��С	
#define Key_STK_SIZE 		50  
//������
TaskHandle_t KeyTask_Handler;
//������
void key_task(void *pvParameters);

//�������ȼ� 
#define Led_TASK_PRIO		3
//�����ջ��С	
#define Led_STK_SIZE 		50  
//������
TaskHandle_t LedTask_Handler;
//������
void Led_task(void *pvParameters);

EventGroupHandle_t Event_Handle=NULL;	//��ʼ����Ҫ�������¼��ľ��
#define Event_0 (0x01<<0)				//�����¼�0 ������0����
#define Event_1 (0x01<<1)				//�����¼�1 ������1����


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
	
	LED1 = 0;
	LED2 = 1;
	printf("FreeRTOS�¼�ʵ��\r\n");
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
	
	Event_Handle = xEventGroupCreate();
	if (NULL != Event_Handle)
		printf("�¼������ɹ�\r\n");
	else
		printf("�¼�����ʧ��\r\n");
      
    //����OLED����
    xTaskCreate((TaskFunction_t )oled_task,     
                (const char*    )"oled_task",   
                (uint16_t       )OLED_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )OLED_TASK_PRIO,
                (TaskHandle_t*  )&OLEDTask_Handler);
				
	//����Զ�̰������ڷ�������ͬʱ���ڷ����¼�
    xTaskCreate((TaskFunction_t )key_task,     
                (const char*    )"key_task",   
                (uint16_t       )Key_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Key_TASK_PRIO,
                (TaskHandle_t*  )&KeyTask_Handler); 
	
	//����led����,���ڵȴ��¼�
    xTaskCreate((TaskFunction_t )Led_task,     
                (const char*    )"led_task",   
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
	//�¼����ձ���
	EventBits_t r_event;
	
	while(1)
	{
		r_event = xEventGroupWaitBits(Event_Handle, (Event_0 | Event_1), pdTRUE, pdTRUE, portMAX_DELAY);
		//��Ҫ���¼��ȴ������ķ���ֵ���м��
		if (r_event & (Event_0 | Event_1) == (Event_0 | Event_1))
		{
			printf("��������������\r\n");
			LED1 = !LED1;
		}
		else
			printf("�¼�����\r\n");
	}
}
