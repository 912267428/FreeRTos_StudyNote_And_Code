#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "YCKey.h"
#include "led.h"
#include "limits.h"


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

uint8_t *Test_Ptr = NULL;		//�����ڴ������ָ��

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
	printf("FreeRTOS�ڴ����ʵ��\r\n");
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
	uint32_t g_memsize;
	
	while(1)
	{
		KeyNum = YCKey_GetNum(0);
		if (KeyNum == KEY0_PRESS)
		{
			if(NULL == Test_Ptr)
			{
				//��ȡ��ǰ�ڴ��С
				g_memsize = xPortGetFreeHeapSize();
				printf("��ǰ�ڴ���СΪ��%d����ʼ�����ڴ�\r\n", g_memsize);
				Test_Ptr = pvPortMalloc(1024);
				if(NULL != Test_Ptr)
				{
					printf("�ڴ�����ɹ�\r\n");
					printf("���뵽�ĵ�ַΪ��%#x\r\n",(int)Test_Ptr);
					
					//��ȡ��ǰ�ڴ��С
					g_memsize = xPortGetFreeHeapSize();
					printf("��ǰ�ڴ���СΪ��%d����ʼ�����ڴ�\r\n", g_memsize);
					
					//�����뵽���ڴ���д������
					sprintf((char*)Test_Ptr,"��ǰϵͳTickCount = %d \r\n",xTaskGetTickCount());
					printf("д��������� %s \r\n",(char*)Test_Ptr);
				}
			}
			else
					printf("���Ȱ���KEY1�ͷ��ڴ�������\r\n");
		}
		else if(KeyNum == KEY1_PRESS)
		{
			if(NULL != Test_Ptr)
			{
				printf("�ͷ��ڴ�\n");
				vPortFree(Test_Ptr);//�ͷ��ڴ�
				Test_Ptr=NULL;
				/* ��ȡ��ǰ��ʣ����С */
				g_memsize = xPortGetFreeHeapSize();
				printf("ϵͳ��ǰ�ڴ��СΪ %d �ֽڣ��ڴ��ͷ����\r\n",g_memsize);
			}
			else
				printf("���Ȱ���KEY0�����ڴ����ͷ�\r\n");
		}
		vTaskDelay(20);
	}
}
