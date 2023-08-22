#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "semphr.h"
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
#define Receive_TASK_PRIO		3
//�����ջ��С	
#define Receive_STK_SIZE 		50  
//������
TaskHandle_t ReceiveTask_Handler;
//������
void receive_task(void *pvParameters);

SemaphoreHandle_t BinarySem_Handle = NULL;  //��ʼ����ֵ�ź����ľ��
//ʵ���Ͼ���QueueHandle_t��Ϊ����Ϣ���й���һ�����п��ƿ�     typedef QueueHandle_t SemaphoreHandle_t;

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
	
	vTaskSuspendAll();  //��ͣ��������
	USART1_Init(9600);
	YCKey_Init();
	LED_Init();

	printf("��ֵ�ź���ʵ��\r\n");
	
	xTaskResumeAll(); //��ʼ��������
	
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
	
	/* ������ֵ�ź���*/
	BinarySem_Handle = xSemaphoreCreateBinary();
      
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
				
	//���������ź���������
    xTaskCreate((TaskFunction_t )receive_task,     
                (const char*    )"Receive_task_task",   
                (uint16_t       )Receive_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Receive_TASK_PRIO,
                (TaskHandle_t*  )&ReceiveTask_Handler);
				
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
		LED2 = !LED2_IN;
    }
}

void key_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	u8 KeyNum = 0;
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum != KEY_NO)
		{
			xReturn = xSemaphoreGive(BinarySem_Handle);
			if(pdTRUE == xReturn)
				printf("��ֵ�ͺ����ͷųɹ�\r\n");
			else
				printf("��ֵ�ͺ����ͷ�ʧ��\r\n");
		}
		vTaskDelay(50);
	}
}

void receive_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
	
	while(1)
	{
		xReturn = xSemaphoreTake(BinarySem_Handle,portMAX_DELAY);
		if(xReturn == pdTRUE)
		{
			printf("��ֵ�ź�����ȡ�ɹ�\r\n\r\n");
			LED1 = !LED1_IN;
		}
	}
}
