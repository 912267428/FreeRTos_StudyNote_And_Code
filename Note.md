## 4、FreeRTOS启动流程

视频中关于外设以及中断的初始化函数都是在main函数中的，但是我在实践的时候发现将OLED显示屏的初始化函数放在main函数中时整个程序会卡死，当时的解决方案是将初始化函数放在任务中。后来又遇到了在main函数中调用复写的printf函数向串口发送数据也卡死，查找资料应该是因为芯片的性能或者堆栈空间不够大的问题。为了验证我在main函数开头定义了一个u32的变量用for训练一直加，100、1000都没有问题，但是到了10000就出现了前面提到的卡死问题，证明查找资料得到的结论基本正确。
解决方案有两个：一是在任务中进行外设的初始化，但是我不太喜欢这种方式，后面可能会遇到在多个任务中需要使用同一外设的问题。二是在进入main函数后挂起全部任务，初始化完后在恢复全部任务。分别调用vTaskSuspendAll()和xTaskResumeAll()，外设初始化代码则在这两行中间编写。

#### 任务创建

1：在系统上电是第一个执行的是启动文件里面由汇编语言编写的复位函数
![](image\4.1复位函数.jpg)
		在复位函数中初始化系统时钟，然后再跳转到C语言空函数__main(主要工作是初始化系统的堆和栈)最后调用main函数执行。

2：在main函数中直接进行创建任务的操作，FreeRTOS中会自动完成一些初始化的工作（eg.内存堆的初始化）。 在main函数中创建任务时提前定义好**任务函数**、**任务堆栈大小**、**任务优先级**、**任务句柄**（实际上就是指向任务控制块TCB的指针）然后调用**xTaskCreate**既可。

![image-20230820201254584](image\4.2任务创建_预定于.jpg)

![image-20230820201357342](image\4.3任务创建_传教开始任务.jpg)
创建任务函数中会有申请内存的操作：

```c
pxNewTCB = ( TCB_t * ) pvPortMalloc( sizeof( TCB_t ) );
```

申请内存函数pvPortMalloc中会对内存进行初始化：

```C
if( pxEnd == NULL )  //表示是第一次调用
        {
            prvHeapInit();	//内存初始化
        }
```

3.任务创建好之后需要开启任务调度器（通过函数vTaskStartScheduler();  见图三）
	创建任务后只是将任务添加到系统中还没有真正去调度，并且空闲任务也没有实现、定时器任务也没有实现，而这些任务都是在开启任务调度中实现的。在RTOS中，一旦系统启动就需要保证每时每刻都有一个任务处于运行的状态，所以一定需要有一个空闲任务保证这一点，并且空闲任务不能被挂起和删除，空闲任务的优先级是最低的，为0，这样是方便系统中其他任务能随时抢占空闲任务的CPU使用权。

```c
//task.c vTaskStartScheduler函数中，2032行 ，创建空闲任务
xReturn = xTaskCreate( prvIdleTask,
                                   configIDLE_TASK_NAME,
                                   configMINIMAL_STACK_SIZE,
                                   ( void * ) NULL,
                                   portPRIVILEGE_BIT,
                                   &xIdleTaskHandle );
//2043行， 创建定时器任务
if( xReturn == pdPASS )
 {
	xReturn = xTimerCreateTimerTask();
 }
```

#### 任务调度

在Cortex-M3架构中，FreeRTOS为了任务启动和任务切换使用了三个异常：**SVC**、**PendSV**、**SysTick**

- SVC：（系统服务调用，简称系统调用）用于任务启动
- PendSV：（可挂起系统调用）用于完成任务切换
- SysTick：用于产生系统节拍时钟，提供一个时间片

PendSV和SysTick的异常优先级设置为最低，这样任务切换的时候就不会打断某个中断服务程序，中断服务程序也不会延迟。

#### 主函数

1. 首先进行硬件初始化
2. 调用函数xTaskCreate创建开始任务
3. 调用函数vTaskStartScheduler开启任务调度器
4. 在开始任务函数中创建应用任务（逻辑代码都在应用任务中）**在主任务函数中会首先进入临界区，在临界区中的代码不会被调度器打断，直到退出临界区。**
5. 删除开始任务

## 5、任务管理

对于实时操作系统来说，核心就是管理各个任务与各个任务间的通信。

#### 简介

FreeRTOS四种任务状态之间的转换图：(**仅就绪态可转变成运行态**  	**其他状态的任务想运行，必须先转变成就绪态**)

![image-20230820205009130](image\5.1任务状态.jpg)

#### 常用API函数

##### void vTaskSuspend(TaskHandle_t xTaskToSuspend)

**参数**：TaskHandle_t xTaskToSuspend	要挂起任务的任务句柄，如果使用函数 xTaskCreate() 创建任务的话那么函数的参数pxCreatedTask 就是此任务的任务句柄。也可以通过函数 xTaskGetHandle()来根据任务名字来获取某个任务的任务句柄。**注意!如果参数为 NULL 的话表示挂起任务自己。**

被挂起的任务在解除挂起态之前绝对不会得到CPU的使用权。



##### void vTaskSuspendAll(void)   如名字，挂起所有任务。

函数实现其实是挂起任务的调度器，不能进行上下文切换。但是中断还是使能的。



##### void vTaskResume(TaskHandle_t xTaskToResume)

任务恢复函数，使得任务恢复到就绪态
**参数**：TaskHandle_t xTaskToResume	要恢复任务的句柄



##### BaseType_t xTaskResumeFromISR(TaskHandle_t xTaskToResume)

任务恢复函数，与前面一个的恢复函数不同，此函数是在中断中恢复函数。
**参数**：同上
**返回值**：pdTRUE：恢复运行的任务的优先级等于或者高于正在运行的任务(被中断打断的任务)，这意味着在退出中断服务函数以后必须进行一次上下文切换。pdFALSE：恢复运行的任务的任务优先级低于当前正在运行的任务(被中断打断的任务)，这意味着在退出中断服务函数的以后不需要进行上下文切换。



##### void vTaskDelete(TaskHandle_t xTaskToDelete)

任务删除函数
**参数**：TaskHandle_t xTaskToDelete		要删除函数的句柄，**删除自己传入NULL**



##### void vTaskDelay(const TickType_t xTicksToDelay)

任务延时函数，可以将任务变为指定时间片的阻塞态。
**参数**：const TickType_t xTicksToDelay	指定延时的时间片数



##### void vTaskDelayUntil(TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement)

绝地延时函数，用于需要绝对精确时间周期的任务。
**参数**：TickType_t * const pxPreviousWakeTime	上一次唤醒的时间
			const TickType_t xTimeIncrement				延时的节拍



#### 任务的设计要求

FreeRTOS中任务运行的上下文包括：

1. 中断服务函数。**需要特别注意**
   运行在非任务的执行环境下，不能使用**挂起当前任务**的操作。
   不允许调用任何会**阻塞**任务的API接口
   最好保持精简、短小。一般只做标记事件发送的操作，然后通知对应任务进行相关处理。
2. 普通任务
   每个任务都有明确的优先级，而在实时操作系统中，任务不能返回在末尾会有一个死循环，而高优先级的任务会抢占低优先级的任务，所以这个死循环一定不能是真正的死循环，否则就会影响到其他低优先级的任务。所以需要在合适的时机将任务阻塞。
3. 空闲任务
   保证CPU在任何时刻都有任务在执行，在空闲任务里面不允许挂起。
   可以在空闲任务中释放一些资源
4. 任务执行时间
   分为任务开始到结束的时间，和任务的周期。
   要正确设计优先级，保证各个任务都能在要求的响应时间内完成。



## 6、消息队列

队列又称消息队列，是一种常用于**任务间**通信的数据结构，队列可以在任务与任务间、中断和任务间传递信息，实现了任务接收来自其他任务或中断的不固定长度的消息。

FreeRTOS 中使用队列数据结构实现任务异步通信工作，具有如下特性：

- 消息支持先进先出方式排队，支持异步读写工作方式。
- 读写队列均支持超时机制。
- 消息支持后进先出方式排队，往队首发送消息（LIFO）。
- 可以允许不同长度（不超过队列节点最大值）的任意类型消息。
- 一个任务能够从任意一个消息队列接收和发送消息。
- 多个任务能够从同一个消息队列接收和发送消息。
- 当队列使用结束后，可以通过删除队列函数进行删除。

##### 消息队列的数据存储

通常队列采用先进先出(FIFO)的存储缓冲机制，当然也可以使用 LIFO。
数据发送到队列中会导致数据拷贝，也就是将要发送的数据拷贝到队列中，这就意味着在队列中存储的是数据的原始值，而不是原数据的引用(即只传递数据的指针)，这个也叫做**值传递**。当特殊情况下，比如需要发送的数据很大时也可以使用地址传递，将数据的指针放入消息队列。

##### 出队阻塞

当任务尝试从一个队列中读取消息的时候可以指定一个阻塞时间，这个阻塞时间就是当任务从队列中读取消息无效的时候任务阻塞的时间（时间节拍数）。可以通过设置阻塞时间控制任务读取队列遇到消息无效时的等待模式（0：不等待   0~Max：等待指定的阻塞时间节拍    MAX：一直等待，直到接受数据为止）

##### 入队阻塞

入队说的是向队列中发送消息，将消息加入到队列中。和出队阻塞一样，当一个任务向队列发送消息的话也可以设置阻塞时间（队列是满的时候发生）。

##### 消息队列的使用

在创建队列的时候需要指定**队列的长度**以及**每条消息的长度**。在以值传递的方式向消息队列发送消息时，一旦发送完成那么原变量是可以再次使用的。

#### <a name="消息队列控制块">消息队列控制块</a>

```CC
typedef struct QueueDefinition /* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
    int8_t * pcHead;		//指向队列消息存储区的起始位置
    int8_t * pcWriteTo;		//指向队列消息存储区下一个可用消息空间
    
    //下面结构与视频教程版本不一样
    union		//用枚举类型确保只有一个存在
    {
        QueuePointers_t xQueue;		//用作队列时选择，
        ///////////////////////////////////////
        //typedef struct QueuePointers
		//{
    	//	int8_t * pcTail;     //指向队列消息存储区的结束位置
    	//	int8_t * pcReadFrom; //指向出队消息空间的最后一个
		//} QueuePointers_t;
        //////////////////////////////////////
        SemaphoreData_t xSemaphore; //用作信号量时选择
        ////////////////////////////////////////
        //typedef struct SemaphoreData
		//{
    	//	TaskHandle_t xMutexHolder;        //持有互斥对象的任务的句柄。
    	//	UBaseType_t uxRecursiveCallCount; //用于计数，记录递归互斥量被调用的次数
		//} SemaphoreData_t;
        ////////////////////////////////////////
    } u;

    List_t xTasksWaitingToSend;		//发送消息阻塞的列表，用于保存阻塞在队列的任务
    List_t xTasksWaitingToReceive;  //获取消息阻塞的列表，用于保存阻塞在队列的任务
    
    volatile UBaseType_t uxMessagesWaiting;  //记录当前消息队列的消息个数，当消息队列用于信号量时用于记录信号量的个数
    UBaseType_t uxLength;		//消息队列的长度
    UBaseType_t uxItemSize;      //保存单个消息的大小   单位是字节        
    volatile int8_t cRxLock;    //用于当队列上锁之后的接收列表项数目（出队的数目）  ，没用上锁会设置为一个宏定义的值 
    volatile int8_t cTxLock;    //用于当队列上锁之后发送的列表项数目（入队的数目）  ，没用上锁会设置为一个宏定义的值
    
//下面是通过条件编译定义的一些队列的其他功能
    #if ( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
        uint8_t ucStaticallyAllocated; 
    #endif

    #if ( configUSE_QUEUE_SETS == 1 )
        struct QueueDefinition * pxQueueSetContainer;
    #endif

    #if ( configUSE_TRACE_FACILITY == 1 )
        UBaseType_t uxQueueNumber;
        uint8_t ucQueueType;
    #endif
} xQUEUE;
```

#### 常用消息队列的API函数

1. ##### ![创建消息队列函数](image\6.1 创建消息队列函数.jpg)

2. ##### ![image-20230821163718411](image\6.2 消息队列静态创建函数.jpg)

3. ##### 消息队列删除函数 vQueueDelete()

   void vQueueDelete( QueueHandle_t xQueue )
   删除消息队列
   **参数：** QueueHandle_t xQueue  要删除消息队列的句柄。

4. ##### 向队列发送消息的函数

   1. ###### ![image-20230821170309796](image\6.3 向消息队列发送消息函数(队尾).jpg)

   2. ###### xQueueSendToBack()  返回值与参数均同函数1。

      函数1，2实际都是xQueueGenericSend函数的重定义
      ![image-20230821171047939](image\6.4 向消息队列发送消息函数(队尾)的重定义.jpg)

   3. ###### ![image-20230821171405636](image\6.5 向消息队列发送消息函数(中断中使用).jpg)

   4. ###### xQueueSendToBackFromISR() 返回值与参数均同函数3。![image-20230821171621479](image\6.4 向消息队列发送消息函数(中断使用)(队尾)的重定义.jpg)

   5. ###### ![image-20230821171744714](image\6.3 向消息队列发送消息函数(队头).jpg)![image-20230821171827422](image\6.3 向消息队列发送消息函数(队头)的定义.jpg)

   6. ###### ![image-20230821172033492](image\6.5 向消息队列发送消息函数(中断中使用)(队头).jpg)![image-20230821172136195](image\6.4 向消息队列发送消息函数(中断使用)(队头)的重定义.jpg)

   7. ###### xQueueGenericSend()![image-20230821172252582](image\6.6 xQueueGenericSend.jpg)

   8. xQueueGenericSendFromISR()![image-20230821172321074](image\6.7 xQueueGenericSendFromISR.jpg)

5. ##### 从消息队列读取消息函数
   1. ###### ![image-20230821172558271](image\6.8 xQueueReceive.jpg)

   2. ###### xQueuePeek() 同上 但是不会吧消息从该队列中移除

   3. ###### ![image-20230821172823009](image\6.9 xQueueReceiveFromISR.jpg)

   4. ###### ![image-20230821172907181](image\6.10 xQueuePeekFromISR.jpg)

6. ##### **消息队列使用注意事项**

   ![image-20230821173010583](image\6.11 消息队列使用注意事项.jpg)

## 

## 7、信号量

#### 简介

信号量（Semaphore）是一种实现**任务间通信**的机制，可以实现任务之间同步或临界资源的互斥访问，常用于协助一组**相互竞争**的任务来访问临界资源。在多任务系统中，各任务之间需要同步或互斥实现临界资源的保护，信号量功能可以为用户提供这方面的支持。

1. ###### 二值信号量

   二值信号量既可以用于临界资源访问也可以用于同步功能。

2. ###### 计数信号量

   二进制信号量可以被认为是长度为 1 的队列，而计数信号量则可以被认为长度大于 1的队列，信号量使用者依然不必关心存储在队列中的消息，只需关心队列是否有消息即可。

3. ###### 互斥信号量

   互斥信号量其实是特殊的二值信号量，由于其特有的**优先级继承机制**从而使它更适用于**简单互锁**，也就是保护临界资源。**不能用在中断中**

4. 递归信号量

#### 二值信号量

![image-20230822163532316](image\7.1 二值信号量的运行机制.jpg)

#### 计数信号量

![image-20230822165102236](D:\Program Files(x86)\qrs\FreeRTos_StudyNote_And_Code\image\7.2 计数信号量的运作机制.jpg)

​	可以允许多个任务获取信号量共享资源，但是回限制任务的最大数目。

#### 信号量控制块

与消息队列的结构体一模一样，只是某些成员变量的含义不同。：[消息队列控制块](#消息队列控制块)

```c
typedef struct QueueDefinition /* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
    int8_t * pcHead;		
    int8_t * pcWriteTo;		
    
    //下面结构与视频教程版本不一样
    union		//用枚举类型确保只有一个存在
    {
        QueuePointers_t xQueue;	
        SemaphoreData_t xSemaphore; //用作信号量时选择
        ////////////////////////////////////////
        //typedef struct SemaphoreData
		//{
    	//	TaskHandle_t xMutexHolder;        //持有互斥对象的任务的句柄。
    	//	UBaseType_t uxRecursiveCallCount; //用于计数，记录递归互斥量被调用的次数
		//} SemaphoreData_t;
        ////////////////////////////////////////
    } u;

    List_t xTasksWaitingToSend;		
    List_t xTasksWaitingToReceive;  
    
    volatile UBaseType_t uxMessagesWaiting;  //队列用于信号量时该成员记录有效信号量的个数，根据信号量的分类选择。  
    UBaseType_t uxLength;		//队列用于信号量时该成员表示最大信号量的可用个数
    UBaseType_t uxItemSize;     //队列用于信号量时该成员表示无存储空间    
    volatile int8_t cRxLock;    
    volatile int8_t cTxLock;    
    
//下面是通过条件编译定义的一些队列的其他功能
///...
} xQUEUE;
```

#### 信号量常用API函数

1. ###### 创建二值信号量 xSemaphoreCreateBinary 返回一个句柄。![](image\7.3 创建二值信号量.jpg)

   必须要在配置文件中使能动态内存分配才能用。

2. ###### 创建计数信号量  xSemaphoreCreateCounting()<img src="image\7.4 创建计数信号量.jpg" alt="image-20230822171150207" style="zoom: 80%;" />

3. ###### 信号量删除函数 vSemaphoreDelete()![image-20230822171304196](image\7.5 信号量删除函数.jpg)

   如果有任务阻塞在一个信号量时不要删除这个信号量

4. ###### 信号量释放函数(普通任务)  xSemaphoreGive()![image-20230822171438226](image\7.3 信号量释放函数(普通任务).jpg)

5. ###### 信号量释放函数(中断中)  xSemaphoreGiveFromISR()![image-20230822171544934](D:\Program Files(x86)\qrs\FreeRTos_StudyNote_And_Code\image\7.6 信号量释放函数(中断中).jpg)

6. ###### 信号量获取函数(普通任务) xSemaphoreTake() ![image-20230822171812711](image\7.7 信号量获取函数.jpg)

7. ###### 信号量获取函数(中断)  xSemaphoreTakeFromISR()![image-20230822171922189](image\7.8 信号量获取函数(中断).jpg)

