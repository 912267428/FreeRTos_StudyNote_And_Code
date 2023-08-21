#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "queue.h"
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

//�������ȼ�		��ⰴ�����񣬲�ͬ�������Ͳ�ͬ����Ϣ
#define Key_TASK_PRIO		1
//�����ջ��С	
#define Key_STK_SIZE 		50  
//������
TaskHandle_t KeyTask_Handler;
//������
void key_task(void *pvParameters);

//�������ȼ�		LED���񣬽��ܵ�����Ϣͨ�����ڷ��͸����ԣ���������Ϣ�л�LED״̬
#define Led_TASK_PRIO		1
//�����ջ��С	
#define Led_STK_SIZE 		50  
//������
TaskHandle_t LedTask_Handler;
//������
void Led_task(void *pvParameters);

QueueHandle_t Test_Queue =NULL;
#define QUEUE_LEN	4
#define QUEUE_SIZE	4
#define QUEUE_DATA1 0   //��תLED
#define QUEUE_DATA2 1	//����or�Ƴ�LED��˸ģʽ

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
	
	LED_Init();
	USART1_Init(115200);
	YCKey_Init();
	
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
	
	//������Ϣ����
	Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN, (UBaseType_t ) QUEUE_SIZE);
      
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
				
	//����LED����
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
		OLED_ShowNum(2,1,i,10);
		vTaskDelay(1000);
		i++;
    }
}

void key_task(void *pvParameters)
{
	u8 KeyNum = 0;
	BaseType_t xReturn = pdPASS;
	u32 send_data1 = QUEUE_DATA1;
	u32 send_data2 = QUEUE_DATA2;
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum == KEY0_PRESS)
		{
			//��⵽����0���£�����send_data1����Ϣ����
			printf("������Ϣsend_data1\r\n");
			xReturn = xQueueSend(Test_Queue,&send_data1,0);
			if(pdPASS == xReturn)
				printf("��Ϣsend_data1���ͳɹ�\r\n");
		}
		else if(KeyNum == KEY1_PRESS)
		{
			printf("������Ϣsend_data2\r\n");
			xReturn = xQueueSend( Test_Queue, &send_data2, 0 );       
			if(pdPASS == xReturn)
				printf("��Ϣsend_data2���ͳɹ�\r\n");
		}
		vTaskDelay(20);
	}
}

void Led_task(void *pvParameters)
{
	BaseType_t xReturn = pdTRUE;
	u32 r_queue;
	
	LED1 = 0;
	LED2 = 0;
	u8 mode = 0;  //Ĭ��Ϊ0���������𣩣�1����˸��

	while(1)
	{
		xReturn = xQueueReceive(Test_Queue, &r_queue, 0);  //����
		if(xReturn == pdTRUE)
		{
			printf("���յ���Ϣ\r\n");
			if (r_queue == QUEUE_DATA1 && mode == 0)	//��ת����״̬
			{
				LED1 = ! LED1_in;
				LED2 = ! LED2_in;
			}
			if (r_queue == QUEUE_DATA2)					//��ʼ���˳���˸
			{
				mode = !mode;
				if(mode == 0)
				{
					LED1 = 0;
					LED2 = 0;
				}
			}	
		}
		if(mode == 1)
		{
			LED1 = ! LED1_in;
			LED2 = ! LED2_in;
		}
		
		vTaskDelay(20);
	}
}
