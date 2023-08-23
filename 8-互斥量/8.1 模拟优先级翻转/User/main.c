#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "semphr.h"


//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define OLED_TASK_PRIO		30
//�����ջ��С	
#define OLED_STK_SIZE 		50  
//������
TaskHandle_t OLEDTask_Handler;
//������
void oled_task(void *pvParameters);

//�������ȼ�
#define L_TASK_PRIO		2
//�����ջ��С	
#define L_STK_SIZE 		50  
//������
TaskHandle_t LTask_Handler;
//������
void L_task(void *pvParameters);

//�������ȼ�
#define M_TASK_PRIO		3
//�����ջ��С	
#define M_STK_SIZE 		50  
//������
TaskHandle_t MTask_Handler;
//������
void M_task(void *pvParameters);

//�������ȼ�
#define H_TASK_PRIO		4
//�����ջ��С	
#define H_STK_SIZE 		50  
//������
TaskHandle_t HTask_Handler;
//������
void H_task(void *pvParameters);

SemaphoreHandle_t BinarySem_Handle = NULL;

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
	LED_Init();
	YCKey_Init();
	
	printf("FreeRTOS���ȼ���תʵ��\r\n");
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
	BaseType_t xReturn = pdPASS;
    taskENTER_CRITICAL();           //�����ٽ���
    
	BinarySem_Handle = xSemaphoreCreateBinary();
	xReturn = xSemaphoreGive(BinarySem_Handle);	//�ͷŶ�ֵ�ź���
	
    //����OLED����
    xTaskCreate((TaskFunction_t )oled_task,     
                (const char*    )"oled_task",   
                (uint16_t       )OLED_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )OLED_TASK_PRIO,
                (TaskHandle_t*  )&OLEDTask_Handler);
				
	//
    xTaskCreate((TaskFunction_t )L_task,     
                (const char*    )"L_task",   
                (uint16_t       )L_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )L_TASK_PRIO,
                (TaskHandle_t*  )&LTask_Handler);

    xTaskCreate((TaskFunction_t )M_task,     
                (const char*    )"M_task",   
                (uint16_t       )M_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )M_TASK_PRIO,
                (TaskHandle_t*  )&MTask_Handler);
				
    xTaskCreate((TaskFunction_t )H_task,     
                (const char*    )"H_task",   
                (uint16_t       )H_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )H_TASK_PRIO,
                (TaskHandle_t*  )&HTask_Handler);
				
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

void L_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
	static uint32_t i=0;
	
	while(1)
	{
		printf("�����ȼ������ȡ�ź���\r\n");
		xReturn = xSemaphoreTake(BinarySem_Handle, portMAX_DELAY);
		if(xReturn == pdTRUE)
			printf("�����ȼ�������������\r\n");
		
		for (i=0;i<200000;i++)
		{
			taskYIELD();//�����������
		}
		printf("LowPriority_Task �ͷ��ź���!\r\n\r\n");
		xReturn = xSemaphoreGive( BinarySem_Handle );//������ֵ�ź���
		vTaskDelay(500);
	}
}

void M_task(void *pvParameters)
{
	while(1)
	{
		printf("�����ȼ�������������\r\n\r\n");
		vTaskDelay(500);
	}
}

void H_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
	
	while(1)
	{
		printf("�����ȼ������ȡ�ź���\r\n");
		xReturn = xSemaphoreTake(BinarySem_Handle, portMAX_DELAY); 
		if(pdTRUE == xReturn)
			printf("�����ȼ�������������\r\n\r\n");
		LED2=!LED2;
		xReturn = xSemaphoreGive( BinarySem_Handle );//������ֵ�ź���
		vTaskDelay(500);
	}
}
