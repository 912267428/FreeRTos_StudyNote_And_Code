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
#define Take_TASK_PRIO		3
//�����ջ��С	
#define Take_STK_SIZE 		512  
//������
TaskHandle_t TakeTask_Handler;
//������
void Take_task(void *pvParameters);

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
	printf("FreeRTOS����֪ͨ��������ź���ʵ��\r\n");
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
	
	//����Take����
    xTaskCreate((TaskFunction_t )Take_task,     
                (const char*    )"Take_task",   
                (uint16_t       )Take_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Take_TASK_PRIO,
                (TaskHandle_t*  )&TakeTask_Handler);
	
				
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
			xReturn = xTaskNotifyGive(TakeTask_Handler);
			if ( pdPASS == xReturn ) 
				printf( "KEY0�����£��ͷ�1��ͣ��λ��\r\n" );
		}
		vTaskDelay(20);
	}
}

void Take_task(void *pvParameters)
{
	uint32_t take_num = pdTRUE;
	u8 KeyNum = 0;
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum == KEY1_PRESS)
		{
			take_num = ulTaskNotifyTake(pdFAIL,0);//û�л�ȡ����ֱ�Ӳ��ȴ�
			if ( take_num > 0 ) 
				printf( "KEY1�����£��ɹ����뵽ͣ��λ����ǰ��λΪ %d \r\n", take_num - 1);
			else
				printf( "KEY1�����£���λ�Ѿ�û���ˣ��밴KEY0�ͷų�λ\r\n" );  
		}
		vTaskDelay(20);
	}
}
