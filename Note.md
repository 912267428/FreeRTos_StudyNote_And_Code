## 4.1：FreeRTOS启动流程

### 任务创建

1：在系统上电是第一个执行的是启动文件里面由汇编语言编写的复位函数
![](.\image\4.1复位函数.jpg)
		在复位函数中初始化系统时钟，然后再跳转到C语言空函数__main(主要工作是初始化系统的堆和栈)最后调用main函数执行。

2：在main函数中直接进行创建任务的操作，FreeRTOS中会自动完成一些初始化的工作（eg.内存堆的初始化）。 在main函数中创建任务时提前定义好**任务函数**、**任务堆栈大小**、**任务优先级**、**任务句柄**（实际上就是指向任务控制块TCB的指针）然后调用**xTaskCreate**既可。

![image-20230820201254584](.\image\4.2任务创建_预定于.jpg)

![image-20230820201357342](D:\zqrs\FreeRTos_StudyNote_And_Code\image\4.3任务创建_传教开始任务.jpg)
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

### 任务调度

在Cortex-M3架构中，FreeRTOS为了任务启动和任务切换使用了三个异常：**SVC**、**PendSV**、**SysTick**

- SVC：（系统服务调用，简称系统调用）用于任务启动
- PendSV：（可挂起系统调用）用于完成任务切换
- SysTick：用于产生系统节拍时钟，提供一个时间片

PendSV和SysTick的异常优先级设置为最低，这样任务切换的时候就不会打断某个中断服务程序，中断服务程序也不会延迟。

### 主函数

1. 首先进行硬件初始化
2. 调用函数xTaskCreate创建开始任务
3. 调用函数vTaskStartScheduler开启任务调度器
4. 在开始任务函数中创建应用任务（逻辑代码都在应用任务中）**在主任务函数中会首先进入临界区，在临界区中的代码不会被调度器打断，直到退出临界区。**
5. 删除开始任务