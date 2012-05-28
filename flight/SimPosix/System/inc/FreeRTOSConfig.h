
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

/**
  * @addtogroup PIOS PIOS
  * @{
  * @addtogroup FreeRTOS FreeRTOS
  * @{
  */

/* Notes: We use 5 task priorities */

#define configCPU_CLOCK_HZ						(SYSCLK_FREQ)	// really the NVIC clock ...
#define configTICK_RATE_HZ						((portTickType )1000)
#define configMAX_PRIORITIES					((unsigned portBASE_TYPE)5)
#define configMINIMAL_STACK_SIZE				((unsigned short)512)
#define configTOTAL_HEAP_SIZE					((size_t)(180 * 1024))	// this is minimum, not total
#define configMAX_TASK_NAME_LEN					(16)

#define configUSE_PREEMPTION					1
#define configUSE_IDLE_HOOK						1
#define configUSE_TICK_HOOK						0
#define configUSE_TRACE_FACILITY				0
#define configUSE_16_BIT_TICKS					0
#define configIDLE_SHOULD_YIELD					0
#define configUSE_MUTEXES						1
#define configUSE_RECURSIVE_MUTEXES				1
#define configUSE_COUNTING_SEMAPHORES			0
#define configUSE_ALTERNATIVE_API				0
#define configCHECK_FOR_STACK_OVERFLOW			2
#define configQUEUE_REGISTRY_SIZE				10

#define configUSE_TIMERS						1
#define configTIMER_TASK_PRIORITY				(configMAX_PRIORITIES - 1)	/* run timers at max priority */
#define configTIMER_QUEUE_LENGTH				10
#define configTIMER_TASK_STACK_DEPTH			configMINIMAL_STACK_SIZE

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 					0
//#define configMAX_CO_ROUTINE_PRIORITIES 		( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet				1
#define INCLUDE_uxTaskPriorityGet				1
#define INCLUDE_vTaskDelete						1
#define INCLUDE_vTaskCleanUpResources			1
#define INCLUDE_vTaskSuspend					1
#define INCLUDE_vTaskDelayUntil					1
#define INCLUDE_vTaskDelay						1
#define INCLUDE_xTaskGetSchedulerState			1
#define INCLUDE_xTaskGetCurrentTaskHandle		1
#define INCLUDE_uxTaskGetStackHighWaterMark		1

/* This is the raw value as per the Cortex-M3 NVIC.  Values can be 255
(lowest) to 1 (highest maskable) to 0 (highest non-maskable). */
#define configKERNEL_INTERRUPT_PRIORITY 		15 << 4	/* equivalent to NVIC priority 15 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	 3 << 4	/* equivalent to NVIC priority  3 */

/* This is the value being used as per the ST library which permits 16
priority values, 0 to 15.  This must correspond to the
configKERNEL_INTERRUPT_PRIORITY setting.  Here 15 corresponds to the lowest
NVIC value of 255. */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY	15

/* Enable run time stats collection */
#define configGENERATE_RUN_TIME_STATS 			1
#define INCLUDE_uxTaskGetRunTime 				1

/*
 * Once we move to CMSIS2 we can at least use:
 *
 * CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
 *
 * (still nothing for the DWT registers, surprisingly)
 */
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()									\
	do {																			\
		(*(unsigned long *)0xe000edfc) |= (1<<24);	/* DEMCR |= DEMCR_TRCENA */		\
		(*(unsigned long *)0xe0001000) |= 1; 	/* DWT_CTRL |= DWT_CYCCNT_ENA */	\
	} while(0)
#define portGET_RUN_TIME_COUNTER_VALUE() 			(*(unsigned long *)0xe0001004)	/* DWT_CYCCNT */


/**
  * @}
  */

#endif /* FREERTOS_CONFIG_H */
