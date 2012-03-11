/*
	Copyright (C) 2009 William Davy - william.davy@wittenstein.co.uk
	Contributed to FreeRTOS.org V5.3.0.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License (version 2) as published
	by the Free Software Foundation and modified by the FreeRTOS exception.

	FreeRTOS.org is distributed in the hope that it will be useful,	but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
	more details.

	You should have received a copy of the GNU General Public License along
	with FreeRTOS.org; if not, write to the Free Software Foundation, Inc., 59
	Temple Place, Suite 330, Boston, MA  02111-1307  USA.

	A special exception to the GPL is included to allow you to distribute a
	combined work that includes FreeRTOS.org without being obliged to provide
	the source code for any proprietary components.  See the licensing section
	of http://www.FreeRTOS.org for full details.


	***************************************************************************
	*                                                                         *
	* Get the FreeRTOS eBook!  See http://www.FreeRTOS.org/Documentation      *
	*                                                                         *
	* This is a concise, step by step, 'hands on' guide that describes both   *
	* general multitasking concepts and FreeRTOS specifics. It presents and   *
	* explains numerous examples that are written using the FreeRTOS API.     *
	* Full source code for all the examples is provided in an accompanying    *
	* .zip file.                                                              *
	*                                                                         *
	***************************************************************************

	1 tab == 4 spaces!

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting,
	licensing and training services.
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the Posix port.
 *----------------------------------------------------------*/

#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
/*-----------------------------------------------------------*/

#define MAX_NUMBER_OF_TASKS 		( _POSIX_THREAD_THREADS_MAX )
/*-----------------------------------------------------------*/

#ifndef __CYGWIN__ 
//	#define COND_SIGNALING 
//	#define CHECK_TASK_RESUMES
//	#define RUNNING_THREAD_MUTEX
#endif

/* Parameters to pass to the newly created pthread. */
typedef struct XPARAMS
{
	pdTASK_CODE pxCode;
	void *pvParams;
} xParams;

/* Each task maintains its own interrupt status in the critical nesting variable. */
typedef struct THREAD_SUSPENSIONS
{
	pthread_t hThread;
	pthread_cond_t * hCond;
	pthread_mutex_t * hMutex;
	xTaskHandle hTask;
	portBASE_TYPE xThreadState;
	unsigned portBASE_TYPE uxCriticalNesting;
} xThreadState;
/*-----------------------------------------------------------*/

static xThreadState *pxThreads;
static pthread_once_t hSigSetupThread = PTHREAD_ONCE_INIT;
static pthread_attr_t xThreadAttributes;
#ifdef RUNNING_THREAD_MUTEX
static pthread_mutex_t xRunningThread = PTHREAD_MUTEX_INITIALIZER;
#endif
static pthread_mutex_t xSuspendResumeThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xSwappingThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xIrqMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t hMainThread = ( pthread_t )NULL;
/*-----------------------------------------------------------*/

static volatile portBASE_TYPE xSentinel = 0;
static volatile portBASE_TYPE xRunning = pdFALSE;
static volatile portBASE_TYPE xSuspended = pdFALSE;
static volatile portBASE_TYPE xStarted = pdFALSE;
static volatile portBASE_TYPE xHandover = 0;
static volatile portBASE_TYPE xSchedulerEnd = pdFALSE;
static volatile portBASE_TYPE xInterruptsEnabled = pdTRUE;
static volatile portBASE_TYPE xServicingTick = pdFALSE;
static volatile portBASE_TYPE xPendYield = pdFALSE;
static volatile portLONG lIndexOfLastAddedTask = 0;
static volatile unsigned portBASE_TYPE uxCriticalNesting;
/*-----------------------------------------------------------*/

/*
 * Setup the timer to generate the tick interrupts.
 */
static void *prvWaitForStart( void * pvParams );
static void prvSuspendSignalHandler(int sig);
static void prvSetupSignalsAndSchedulerPolicy( void );
static void pauseThread( portBASE_TYPE pauseMode );
static pthread_t prvGetThreadHandle( xTaskHandle hTask );
#ifdef COND_SIGNALING
static pthread_cond_t * prvGetConditionHandle( xTaskHandle hTask );
static pthread_mutex_t * prvGetMutexHandle( xTaskHandle hTask );
#endif
static xTaskHandle prvGetTaskHandle( pthread_t hThread );
static portLONG prvGetFreeThreadState( void );
static void prvSetTaskCriticalNesting( pthread_t xThreadId, unsigned portBASE_TYPE uxNesting );
static unsigned portBASE_TYPE prvGetTaskCriticalNesting( pthread_t xThreadId );
static void prvDeleteThread( void *xThreadId );
/*-----------------------------------------------------------*/

/*
 * Exception handlers.
 */
void vPortYield( void );
void vPortSystemTickHandler( int sig );

#define THREAD_PAUSE_CREATED	0
#define THREAD_PAUSE_YIELD		1
#define THREAD_PAUSE_INTERRUPT	2

FILE * fid;
 
//#define DEBUG_OUTPUT
#define ERROR_OUTPUT
#ifdef DEBUG_OUTPUT

	static pthread_mutex_t xPrintfMutex = PTHREAD_MUTEX_INITIALIZER;

	#define debug_printf(...) ( (real_pthread_mutex_lock( &xPrintfMutex )|1)?( \
	(  \
	(NULL != (debug_task_handle = prvGetTaskHandle(pthread_self())) )? \
	(fprintf( fid, "%20s(%li)\t%20s\t%i: ",debug_task_handle->pcTaskName,(long)pthread_self(),__func__,__LINE__)): \
	(fprintf( fid, "%20s(%li)\t%20s\t%i: ","__unknown__",(long)pthread_self(),__func__,__LINE__)) \
	|1)?( \
	((fprintf( fid, __VA_ARGS__ )|1)?real_pthread_mutex_unlock( &xPrintfMutex ):0) \
	):0 ):0 )

	#define debug_error debug_printf

	int real_pthread_mutex_lock(pthread_mutex_t* mutex) {
		return pthread_mutex_lock(mutex);
	}
	int real_pthread_mutex_unlock(pthread_mutex_t* mutex) {
		return pthread_mutex_unlock(mutex);
	}
	#define pthread_mutex_lock(...) ( (debug_printf(" -!- pthread_mutex_lock(%s)\n",#__VA_ARGS__)|1)?pthread_mutex_lock(__VA_ARGS__):0 )
	#define pthread_mutex_unlock(...) ( (debug_printf(" -=- pthread_mutex_unlock(%s)\n",#__VA_ARGS__)|1)?pthread_mutex_unlock(__VA_ARGS__):0 )
	#define pthread_kill(thread,signal) ( (debug_printf("Sending signal %i to thread %li!\n",(int)signal,(long)thread)|1)?pthread_kill(thread,signal):0 )
	#define pthread_cond_signal( hCond ) (debug_printf( "pthread_cond_signals(%li)\r\n", *((long int *) hCond) ) ? 1 : pthread_cond_signal( hCond ) )
	#define pthread_cond_timedwait( hCond, hMutex, it ) (debug_printf( "pthread_cond_timedwait(%li,%li)\r\n", *((long int *) hCond), *((long int *) hMutex )) ? 1 : pthread_cond_timedwait( hCond, hMutex, it ) )
	#define pthread_sigmask( how, set, out ) (debug_printf( "pthread_sigmask( %i, %li )\r\n", how, *((long int*) set) ) ? 1 : pthread_sigmask( how, set, out ) )

#else
	#ifdef ERROR_OUTPUT
		static pthread_mutex_t xPrintfMutex = PTHREAD_MUTEX_INITIALIZER;
		#define debug_error(...) ( (pthread_mutex_lock( &xPrintfMutex )|1)?( \
		(  \
		(NULL != (debug_task_handle = prvGetTaskHandle(pthread_self())) )? \
		(fprintf( fid, "%20s(%li)\t%20s\t%i: ",debug_task_handle->pcTaskName,(long)pthread_self(),__func__,__LINE__)): \
		(fprintf( fid, "%20s(%li)\t%20s\t%i: ","__unknown__",(long)pthread_self(),__func__,__LINE__)) \
		|1)?( \
		((fprintf( fid, __VA_ARGS__ )|1)?pthread_mutex_unlock( &xPrintfMutex ):0) \
		):0 ):0 )

		#define debug_printf(...) 
	#else
		#define debug_printf(...)
		#define debug_error(...)
	#endif
#endif


/*
 * Start first task is a separate function so it can be tested in isolation.
 */
void vPortStartFirstTask( void );
/*-----------------------------------------------------------*/


typedef struct tskTaskControlBlock
{
	volatile portSTACK_TYPE	*pxTopOfStack;		/*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE STRUCT. */
	
#if ( portUSING_MPU_WRAPPERS == 1 )
	xMPU_SETTINGS xMPUSettings;				/*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE STRUCT. */
#endif	
	
	xListItem				xGenericListItem;	/*< List item used to place the TCB in ready and blocked queues. */
	xListItem				xEventListItem;		/*< List item used to place the TCB in event lists. */
	unsigned portBASE_TYPE	uxPriority;			/*< The priority of the task where 0 is the lowest priority. */
	portSTACK_TYPE			*pxStack;			/*< Points to the start of the stack. */
	signed char				pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */
	
#if ( portSTACK_GROWTH > 0 )
	portSTACK_TYPE *pxEndOfStack;			/*< Used for stack overflow checking on architectures where the stack grows up from low memory. */
#endif
	
#if ( portCRITICAL_NESTING_IN_TCB == 1 )
	unsigned portBASE_TYPE uxCriticalNesting;
#endif
	
#if ( configUSE_TRACE_FACILITY == 1 )
	unsigned portBASE_TYPE	uxTCBNumber;	/*< This is used for tracing the scheduler and making debugging easier only. */
#endif
	
#if ( configUSE_MUTEXES == 1 )
	unsigned portBASE_TYPE uxBasePriority;	/*< The priority last assigned to the task - used by the priority inheritance mechanism. */
#endif
	
#if ( configUSE_APPLICATION_TASK_TAG == 1 )
	pdTASK_HOOK_CODE pxTaskTag;
#endif
	
#if ( configGENERATE_RUN_TIME_STATS == 1 )
	unsigned long ulRunTimeCounter;		/*< Used for calculating how much CPU time each task is utilising. */
#endif
	
} tskTCB;

tskTCB *debug_task_handle;

/*
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
/* Should actually keep this struct on the stack. */
xParams *pxThisThreadParams = pvPortMalloc( sizeof( xParams ) );

	debug_printf("pxPortInitialiseStack\r\n");

	(void)pthread_once( &hSigSetupThread, prvSetupSignalsAndSchedulerPolicy );

	if ( (pthread_t)NULL == hMainThread )
	{
		hMainThread = pthread_self();
	} 

	/* No need to join the threads. */
	pthread_attr_init( &xThreadAttributes );
	pthread_attr_setdetachstate( &xThreadAttributes, PTHREAD_CREATE_DETACHED );

	/* Add the task parameters. */
	pxThisThreadParams->pxCode = pxCode;
	pxThisThreadParams->pvParams = pvParameters;

	vPortEnterCritical();

	lIndexOfLastAddedTask = prvGetFreeThreadState();

	debug_printf( "Got index for new task %i\r\n", lIndexOfLastAddedTask );
	
#ifdef COND_SIGNALING 
	/* Create a condition signal for this thread */
	pxThreads[ lIndexOfLastAddedTask ].hCond = ( pthread_cond_t *) malloc( sizeof( pthread_cond_t ) );
	assert( 0 == pthread_cond_init(  pxThreads[ lIndexOfLastAddedTask ].hCond , NULL ) ); //&condAttr ) ); 
	debug_printf("Cond: %li\r\n", *( (long int *) &pxThreads[ lIndexOfLastAddedTask ].hCond) );
	
	/* Create a condition mutex for this thread */
	pxThreads[ lIndexOfLastAddedTask ].hMutex = ( pthread_mutex_t *) malloc( sizeof( pthread_mutex_t ) );	
	assert( 0 == pthread_mutex_init( pxThreads[ lIndexOfLastAddedTask ].hMutex, NULL ) ); //&mutexAttr ) );
	debug_printf("Mutex: %li\r\n", *( (long int *) &pxThreads[ lIndexOfLastAddedTask ].hMutex) );
#endif
		
	/* Create a thread and store it's handle number */
	xSentinel = 0;
	assert( 0 == pthread_create( &( pxThreads[ lIndexOfLastAddedTask ].hThread ), &xThreadAttributes, prvWaitForStart, (void *)pxThisThreadParams ) );		

	/* Wait until the task suspends. */
	while ( xSentinel == 0 );
	vPortExitCritical();

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

void vPortStartFirstTask( void )
{
	/* Initialise the critical nesting count ready for the first task. */
	uxCriticalNesting = 0;

	debug_printf("vPortStartFirstTask\r\n");
	
	/* Start the first task. */
	vPortEnableInterrupts();
	xRunning = 1;
	
	/* Start the first task. */
#ifdef COND_SIGNALING
	pthread_cond_t * hCond = prvGetConditionHandle( xTaskGetCurrentTaskHandle() );
	assert( pthread_cond_signal( hCond ) == 0 ); 		
#endif
}
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
portBASE_TYPE xPortStartScheduler( void )
{
portBASE_TYPE xResult;
sigset_t xSignalToBlock;
portLONG lIndex;

	debug_printf( "xPortStartScheduler\r\n" );

	/* Establish the signals to block before they are needed. */
	sigfillset( &xSignalToBlock );
	sigaddset( &xSignalToBlock, SIG_SUSPEND );

	/* Block until the end */
	(void)pthread_sigmask( SIG_SETMASK, &xSignalToBlock, NULL );

	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		pxThreads[ lIndex ].uxCriticalNesting = 0;
	}

	fid = fopen("log.txt", "w");

	/* Start the first task. Will not return unless all threads are killed. */
	vPortStartFirstTask();

	int i = 0;
	usleep(1000000);
	while( pdTRUE != xSchedulerEnd ) {
		usleep(portTICK_RATE_MICROSECONDS);
		i++;
		if(i % 1000 == 0) {
			printf("."); fflush(stdout);
		}
		while(xInterruptsEnabled == pdFALSE){ // Don't call while in interrupt
			printf("."); fflush(stdout);
		}
		vPortSystemTickHandler(SIG_TICK);		
	}
	
	debug_printf( "Cleaning Up, Exiting.\n" );
	/* Cleanup the mutexes */
	xResult = pthread_mutex_destroy( &xSuspendResumeThreadMutex );
	xResult = pthread_mutex_destroy( &xSwappingThreadMutex );
	vPortFree( (void *)pxThreads );

	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
portBASE_TYPE xNumberOfThreads;
portBASE_TYPE xResult;


	for ( xNumberOfThreads = 0; xNumberOfThreads < MAX_NUMBER_OF_TASKS; xNumberOfThreads++ )
	{
		if ( ( pthread_t )NULL != pxThreads[ xNumberOfThreads ].hThread )
		{
			/* Kill all of the threads, they are in the detached state. */
			xResult = pthread_cancel( pxThreads[ xNumberOfThreads ].hThread );
		}
	}

	/* Signal the scheduler to exit its loop. */
	xSchedulerEnd = pdTRUE;
	(void)pthread_kill( hMainThread, SIG_RESUME );
}
/*-----------------------------------------------------------*/

void vPortYieldFromISR( void )
{
	/* Calling Yield from a Interrupt/Signal handler often doesn't work because the
	 * xSwappingThreadMutex is already owned by an original call to Yield. Therefore,
	 * simply indicate that a yield is required soon.
	 */

	xPendYield = pdTRUE;
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
	vPortDisableInterrupts();
	uxCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
	/* Check for unmatched exits. */
	if ( uxCriticalNesting > 0 )
	{
		uxCriticalNesting--;
	}

	/* If we have reached 0 then re-enable the interrupts. */
	if( uxCriticalNesting == 0 )
	{
		/* Have we missed ticks? This is the equivalent of pending an interrupt. */
		if ( pdTRUE == xPendYield )
		{
			xPendYield = pdFALSE;
			vPortYield();
		}
		vPortEnableInterrupts();
	}
}
/*-----------------------------------------------------------*/

void vPortYield( void )
{
pthread_t xTaskToSuspend;
pthread_t xTaskToResume;
sigset_t xSignals;
tskTCB * oldTask, * newTask;
	
	/* We must mask the suspend signal here, because otherwise there can be an */
	/* interrupt while in pthread_mutex_lock and that will cause the next thread */
	/* to deadlock when it tries to get this mutex */	

	debug_printf( "Entering\r\n" );
	
	sigemptyset( &xSignals );
	sigaddset( &xSignals, SIG_SUSPEND );
	pthread_sigmask( SIG_SETMASK, &xSignals, NULL );

	assert( pthread_mutex_lock( &xSwappingThreadMutex ) == 0);	

	oldTask = xTaskGetCurrentTaskHandle();
	xTaskToSuspend = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
	if(xTaskToSuspend != pthread_self() ) {
		/* This means between masking the interrupt and getting the lock, there was an interrupt */
		/* and this task should suspend.  Release the lock, then unmask interrupts to go ahead and */ 
		/* service the signal */

		assert( 0 == pthread_mutex_unlock( &xSwappingThreadMutex ) );
		debug_printf( "The current task isn't even us, letting interrupt happen.  Watch for swap.\r\n" );
		/* Now we are resuming, want to be able to catch this interrupt again */
		sigemptyset( &xSignals );
		pthread_sigmask( SIG_SETMASK, &xSignals, NULL);
		return;
	}

	xStarted = pdFALSE;
	
	/* Get new task then release the task switching mutex */
	vTaskSwitchContext();
	newTask = xTaskGetCurrentTaskHandle();
	xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );

	if ( pthread_self() != xTaskToResume )
	{
		/* Remember and switch the critical nesting. */
		prvSetTaskCriticalNesting( xTaskToSuspend, uxCriticalNesting );
		uxCriticalNesting = prvGetTaskCriticalNesting( xTaskToResume );

		debug_error( "Swapping From %li(%s) to %li(%s)\r\n", (long int) xTaskToSuspend, oldTask->pcTaskName, (long int) xTaskToResume, newTask->pcTaskName);				
		
#ifdef COND_SIGNALING		
		/* Set resume condition for specific thread */
		pthread_cond_t * hCond = prvGetConditionHandle( xTaskGetCurrentTaskHandle() );
		assert( pthread_cond_signal( hCond ) == 0 ); 
#endif
#ifdef CHECK_TASK_RESUMES
		while( xStarted == pdFALSE )
			debug_printf( "Waiting for task to resume\r\n" );
#endif

		debug_printf( "Detected task resuming.  Pausing this task\r\n" );
		
		/* Release swapping thread mutex and pause self */
		assert( pthread_mutex_unlock( &xSwappingThreadMutex ) == 0);
		pauseThread( THREAD_PAUSE_YIELD );				
	}
	else {
		assert( pthread_mutex_unlock( &xSwappingThreadMutex ) == 0);
	}

	/* Now we are resuming, want to be able to catch this interrupt again */
	sigemptyset( &xSignals );
	pthread_sigmask( SIG_SETMASK, &xSignals, NULL);
}
/*-----------------------------------------------------------*/

void vPortDisableInterrupts( void )
{
	//debug_printf("\r\n");
	assert( pthread_mutex_lock( &xIrqMutex ) == 0);
	xInterruptsEnabled = pdFALSE;
	assert( pthread_mutex_unlock( &xIrqMutex) == 0);
}
/*-----------------------------------------------------------*/

void vPortEnableInterrupts( void )
{
	//debug_printf("\r\n");
	assert( pthread_mutex_lock( &xIrqMutex ) == 0);
	xInterruptsEnabled = pdTRUE;
	assert( pthread_mutex_unlock( &xIrqMutex) == 0);
}
/*-----------------------------------------------------------*/

portBASE_TYPE xPortSetInterruptMask( void )
{
portBASE_TYPE xReturn = xInterruptsEnabled;
	debug_printf("\r\n");
	xInterruptsEnabled = pdFALSE;
	return xReturn;
}
/*-----------------------------------------------------------*/

void vPortClearInterruptMask( portBASE_TYPE xMask )
{
	debug_printf("\r\n");
	xInterruptsEnabled = xMask;
}
/*-----------------------------------------------------------*/


void vPortSystemTickHandler( int sig )
{
pthread_t xTaskToSuspend;
pthread_t xTaskToResume;
tskTCB * oldTask, * newTask;
	
	debug_printf( "\r\n\r\n" );
	debug_printf( "(xInterruptsEnabled = %i, xServicingTick = %i)\r\n", (int) xInterruptsEnabled != 0, (int) xServicingTick != 0);
	if ( ( pdTRUE == xInterruptsEnabled ) && ( pdTRUE != xServicingTick ) )
	{
//		debug_printf( "Checking for lock ...\r\n" );
		if ( 0 ==  pthread_mutex_trylock( &xSwappingThreadMutex ) )
		{
			debug_printf( "Handling\r\n");
			xServicingTick = pdTRUE;
			
			oldTask = xTaskGetCurrentTaskHandle();
			xTaskToSuspend = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
			
			/* Tick Increment. */
			vTaskIncrementTick();

			/* Select Next Task. */
#if ( configUSE_PREEMPTION == 1 )
			vTaskSwitchContext();
#endif

			newTask = xTaskGetCurrentTaskHandle();			
			xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );

			debug_printf( "Want %s running\r\n", newTask->pcTaskName );
			/* The only thread that can process this tick is the running thread. */
			if ( xTaskToSuspend != xTaskToResume )
			{
				xSuspended = pdFALSE;
				xStarted = pdFALSE;
				
				/* Remember and switch the critical nesting. */
				prvSetTaskCriticalNesting( xTaskToSuspend, uxCriticalNesting );
				uxCriticalNesting = prvGetTaskCriticalNesting( xTaskToResume );

				debug_printf( "Swapping From %li(%s) to %li(%s)\r\n", (long int) xTaskToSuspend, oldTask->pcTaskName, (long int) xTaskToResume, newTask->pcTaskName);		

#ifdef CHECK_TASK_RESUMES
				/* It shouldn't be possible for a second task swap to happen while waiting for this because */
				/* they can't get the xSwappingThreadMutex */
				while( xSuspended == pdFALSE ) 
#endif
				{
					assert( pthread_kill( xTaskToSuspend, SIG_SUSPEND ) == 0);
					sched_yield();
				}
				
#ifdef CHECK_TASK_RESUMES				
				while( xStarted == pdFALSE) 					
#endif					
				{
					
#ifdef COND_SIGNALING
					// Set resume condition for specific thread
					pthread_cond_t * hCond = prvGetConditionHandle( xTaskGetCurrentTaskHandle() );
					assert( pthread_cond_signal( hCond ) == 0 ); 
#endif
					assert( pthread_kill( xTaskToSuspend, SIG_SUSPEND ) == 0);

					sched_yield();
				}

				debug_printf( "Swapped From %li(%s) to %li(%s)\r\n", (long int) xTaskToSuspend, oldTask->pcTaskName, (long int) xTaskToResume, newTask->pcTaskName);		}
			else
			{
			//	debug_error ("Want %s running \r\n", newTask->pcTaskName );
			}
			xServicingTick = pdFALSE;
			(void)pthread_mutex_unlock( &xSwappingThreadMutex );
		}
		else
		{
			debug_error( "Pending yield here (portYield has lock - hopefully)\r\n" );
			xPendYield = pdTRUE;
		}

	}
	else
	{
		debug_printf( "Pending yield or here\r\n");
		xPendYield = pdTRUE;
	}
}
/*-----------------------------------------------------------*/

void vPortForciblyEndThread( void *pxTaskToDelete )
{
xTaskHandle hTaskToDelete = ( xTaskHandle )pxTaskToDelete;
pthread_t xTaskToDelete;
pthread_t xTaskToResume;
portBASE_TYPE xResult;

	printf("vPortForciblyEndThread\r\n");

	if ( 0 == pthread_mutex_lock( &xSwappingThreadMutex ) )
	{
		xTaskToDelete = prvGetThreadHandle( hTaskToDelete );
		xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );

		if ( xTaskToResume == xTaskToDelete )
		{
			/* This is a suicidal thread, need to select a different task to run. */
			vTaskSwitchContext();
			xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
		}

		if ( pthread_self() != xTaskToDelete )
		{
			/* Cancelling a thread that is not me. */
			if ( xTaskToDelete != ( pthread_t )NULL )
			{
				/* Send a signal to wake the task so that it definitely cancels. */
				pthread_testcancel();
				xResult = pthread_cancel( xTaskToDelete );
				/* Pthread Clean-up function will note the cancellation. */
			}
			(void)pthread_mutex_unlock( &xSwappingThreadMutex );
		}
		else
		{
			/* Resume the other thread. */
			/* Assert zero - I never fixed this functionality */
			//assert( 0 );
			
			/* Pthread Clean-up function will note the cancellation. */
			/* Release the execution. */
			uxCriticalNesting = 0;
			vPortEnableInterrupts();
			(void)pthread_mutex_unlock( &xSwappingThreadMutex );
			/* Commit suicide */
			pthread_exit( (void *)1 );
		}
	}
}
/*-----------------------------------------------------------*/

void *prvWaitForStart( void * pvParams )
{
xParams * pxParams = ( xParams * )pvParams;
pdTASK_CODE pvCode = pxParams->pxCode;
void * pParams = pxParams->pvParams;
	vPortFree( pvParams );

	pthread_cleanup_push( prvDeleteThread, (void *)pthread_self() );
		
	/* want to block suspend when not the active thread */
	sigset_t xSignals;
	sigemptyset( &xSignals );
	sigaddset( &xSignals, SIG_SUSPEND );
	assert( pthread_sigmask( SIG_BLOCK, &xSignals, NULL ) == 0);

	/* Because the FreeRTOS creates the TCB stack, which in this implementation   */
	/* creates a thread, we need to wait until the task handle is added before    */
	/* trying to pause.  Must set xSentinel high so the creating task knows we're */
	/* here */

	debug_printf("Thread started, waiting till handle is added\r\n");

	xSentinel = 1;

	while( prvGetTaskHandle( pthread_self() ) == NULL ){
		sched_yield();
	}

	debug_printf("Handle added, pausing\r\n");
	
	/* Want to delay briefly until we have explicit resume signal as otherwise the */
	/* current task variable might be in the wrong state */	
	pauseThread( THREAD_PAUSE_CREATED );	
	debug_printf("Starting first run\r\n");

	sigemptyset( &xSignals );
	assert( pthread_sigmask( SIG_SETMASK, &xSignals, NULL ) == 0);

	pvCode( pParams );

	pthread_cleanup_pop( 1 );
	return (void *)NULL;
}
/*-----------------------------------------------------------*/

void pauseThread( portBASE_TYPE pauseMode ) 
{
	debug_printf( "Pausing thread %li.  Set xSuspended false\r\n", (long int) pthread_self() );
	
	//assert( pthread_self() != prvGetThreadHandle(xTaskGetCurrentTaskHandle() ) ); 
	
#ifdef RUNNING_THREAD_MUTEX
	if( pauseMode != THREAD_PAUSE_CREATED ) 
		assert( 0 == pthread_mutex_unlock( &xRunningThread ) ); 
#endif	

#ifdef COND_SIGNALING
	int xResult;
	xTaskHandle hTask = prvGetTaskHandle( pthread_self() );
	pthread_cond_t * hCond = prvGetConditionHandle( hTask );
	pthread_mutex_t * hMutex = prvGetMutexHandle( hTask );		
	debug_printf("Cond: %li\r\n", *( (long int *) hCond) );
	debug_printf("Mutex: %li\r\n", *( (long int *) hMutex) );
	
	struct timeval tv;
	struct timespec ts;
	gettimeofday( &tv, NULL );
	ts.tv_sec = tv.tv_sec + 0;
#endif	

	xSuspended = pdTRUE;

	while (1) {
		if( pthread_self() == prvGetThreadHandle(xTaskGetCurrentTaskHandle() ) && xRunning )
		{
			
			xStarted = pdTRUE;
			
#ifdef RUNNING_THREAD_MUTEX
			assert( 0 == pthread_mutex_lock( &xRunningThread ) );
#endif
			debug_error("Resuming\r\n");
			return;
		}
		else {
#ifdef COND_SIGNALING
			gettimeofday( &tv, NULL );
			ts.tv_sec = ts.tv_sec + 1;
			ts.tv_nsec = 0; 
			xResult = pthread_cond_timedwait( hCond, hMutex, &ts );
			assert( xResult != EINVAL );
#else
			/* For windows where conditional signaling is buggy */
			/* It would be wonderful to put a nanosleep here, but since its not reentrant safe */
			/* and there may be a sleep in the main code (this can be called from an ISR) we must */
			/* check this */
			if( pauseMode != THREAD_PAUSE_INTERRUPT )
				usleep(100);
			sched_yield();
			
#endif
//			debug_error( "Checked my status\r\n" );
		}

	}
}

void prvSuspendSignalHandler(int sig)
{
sigset_t xBlockSignals;

	/* This signal is set here instead of pauseThread because it is checked by the tick handler */
	/* which means if there were a swap it should result in a suspend interrupt */
	
	debug_error( "Caught signal %i\r\n", sig );
	/* Check that we aren't suspending when we should be running.  This bug would need tracking down */
	//assert( pthread_self() != prvGetThreadHandle(xTaskGetCurrentTaskHandle() ) ); 
	if( pthread_self() == prvGetThreadHandle( xTaskGetCurrentTaskHandle() ) ) 
	{
		debug_printf( "Suspend ISR called while this thread still marked active.  Reflects buggy behavior in scheduler\r\n" );
		return;
	}

	/* Block further suspend signals.  They need to go to their thread */
	sigemptyset( &xBlockSignals );
	sigaddset( &xBlockSignals, SIG_SUSPEND );
	assert( pthread_sigmask( SIG_BLOCK, &xBlockSignals, NULL ) == 0);
	
	pauseThread( THREAD_PAUSE_INTERRUPT ); 
	
//	assert( pthread_self() == prvGetThreadHandle( xTaskGetCurrentTaskHandle() ) );
	while( pthread_self() != prvGetThreadHandle( xTaskGetCurrentTaskHandle() ) )
	{
		debug_printf( "Incorrectly woke up.  Repausing\r\n" );
		pauseThread( THREAD_PAUSE_INTERRUPT );
	}

	/* Make sure the right thread is resuming */
	assert( pthread_self() == prvGetThreadHandle(xTaskGetCurrentTaskHandle() ) ); 
	
	/* Old synchronization code, may still be required 
	while( !xHandover );
	assert( 0 == pthread_mutex_lock( &xSingleThreadMutex ) ); */

	/* Respond to signals again */
	sigemptyset( &xBlockSignals );
	pthread_sigmask( SIG_SETMASK, &xBlockSignals, NULL );
	
	debug_printf( "Resuming %li from signal %i\r\n", (long int) pthread_self(), sig );	

	/* Will resume here when the SIG_RESUME signal is received. */
	/* Need to set the interrupts based on the task's critical nesting. */
	if ( uxCriticalNesting == 0 )
	{
		vPortEnableInterrupts();
	}
	else
	{
		vPortDisableInterrupts();
	}
	debug_printf("Exit\r\n");
}

/*-----------------------------------------------------------*/

void prvSetupSignalsAndSchedulerPolicy( void )
{
/* The following code would allow for configuring the scheduling of this task as a Real-time task.
 * The process would then need to be run with higher privileges for it to take affect.
int iPolicy;
int iResult;
int iSchedulerPriority;
	iResult = pthread_getschedparam( pthread_self(), &iPolicy, &iSchedulerPriority );
	iResult = pthread_attr_setschedpolicy( &xThreadAttributes, SCHED_FIFO );
	iPolicy = SCHED_FIFO;
	iResult = pthread_setschedparam( pthread_self(), iPolicy, &iSchedulerPriority );		*/

struct sigaction sigsuspendself;
portLONG lIndex;
	
	debug_printf("prvSetupSignalAndSchedulerPolicy\r\n");
	
	pxThreads = ( xThreadState *)pvPortMalloc( sizeof( xThreadState ) * MAX_NUMBER_OF_TASKS );
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		pxThreads[ lIndex ].hThread = ( pthread_t )NULL;
		pxThreads[ lIndex ].hTask = ( xTaskHandle )NULL;
		pxThreads[ lIndex ].uxCriticalNesting = 0;
	}
	
	sigsuspendself.sa_flags = 0;
	sigsuspendself.sa_handler = prvSuspendSignalHandler;
	sigfillset( &sigsuspendself.sa_mask );

	assert ( 0 == sigaction( SIG_SUSPEND, &sigsuspendself, NULL ) );
}

/*-----------------------------------------------------------*/
pthread_mutex_t * prvGetMutexHandle( xTaskHandle hTask ) 
{	
pthread_mutex_t * hMutex;
portLONG lIndex;
	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hTask == hTask )
		{
			hMutex = pxThreads[ lIndex ].hMutex;
			break;
		}
	}
	return hMutex;
}

/*-----------------------------------------------------------*/
xTaskHandle prvGetTaskHandle( pthread_t hThread ) 
{
xTaskHandle hTask = NULL;
portLONG lIndex;
	
	/* If not initialized yet */
	if( pxThreads  == NULL ) return NULL;
	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == hThread )
		{
			hTask = pxThreads[ lIndex ].hTask;
			break;
		}
	}
	return hTask;
} 

/*-----------------------------------------------------------*/
pthread_cond_t * prvGetConditionHandle( xTaskHandle hTask )
{
pthread_cond_t * hCond;
portLONG lIndex;
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hTask == hTask )
		{
			debug_printf( "Found condition on %i task\r\n", lIndex );
			return pxThreads[ lIndex ].hCond;
			break;
		}
	}
	printf( "Failed to get handle, pausing then recursing\r\n" );
	usleep(1000); 
	return prvGetConditionHandle( hTask );
	assert(0);
	return hCond;
}

/*-----------------------------------------------------------*/
pthread_t prvGetThreadHandle( xTaskHandle hTask )
{
	pthread_t hThread = ( pthread_t )NULL;
	portLONG lIndex;
	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hTask == hTask )
		{
			hThread = pxThreads[ lIndex ].hThread;
			break;
		}
	}
	return hThread;
}
/*-----------------------------------------------------------*/

portLONG prvGetFreeThreadState( void )
{
portLONG lIndex;

	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == ( pthread_t )NULL )
		{
			break;
		}
	}

	if ( MAX_NUMBER_OF_TASKS == lIndex )
	{
		printf( "No more free threads, please increase the maximum.\n" );
		lIndex = 0;
		vPortEndScheduler();
	}

	return lIndex;
}
/*-----------------------------------------------------------*/

void prvSetTaskCriticalNesting( pthread_t xThreadId, unsigned portBASE_TYPE uxNesting )
{
portLONG lIndex;

	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == xThreadId )
		{
			pxThreads[ lIndex ].uxCriticalNesting = uxNesting;
			break;
		}
	}
}
/*-----------------------------------------------------------*/

unsigned portBASE_TYPE prvGetTaskCriticalNesting( pthread_t xThreadId )
{
unsigned portBASE_TYPE uxNesting = 0;
portLONG lIndex;

	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == xThreadId )
		{
			uxNesting = pxThreads[ lIndex ].uxCriticalNesting;
			break;
		}
	}
	return uxNesting;
}
/*-----------------------------------------------------------*/

void prvDeleteThread( void *xThreadId )
{
portLONG lIndex;

	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == ( pthread_t )xThreadId )
		{
			pxThreads[ lIndex ].hThread = (pthread_t)NULL;
			pxThreads[ lIndex ].hTask = (xTaskHandle)NULL;
			if ( pxThreads[ lIndex ].uxCriticalNesting > 0 )
			{
				uxCriticalNesting = 0;
				vPortEnableInterrupts();
			}
			pxThreads[ lIndex ].uxCriticalNesting = 0;
			break;
		}
	}
}
/*-----------------------------------------------------------*/

void vPortAddTaskHandle( void *pxTaskHandle )
{
portLONG lIndex;

	debug_printf("vPortAddTaskHandle\r\n");

	pxThreads[ lIndexOfLastAddedTask ].hTask = ( xTaskHandle )pxTaskHandle;
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == pxThreads[ lIndexOfLastAddedTask ].hThread )
		{
			if ( pxThreads[ lIndex ].hTask != pxThreads[ lIndexOfLastAddedTask ].hTask )
			{
				pxThreads[ lIndex ].hThread = ( pthread_t )NULL;
				pxThreads[ lIndex ].hTask = NULL;
				pxThreads[ lIndex ].uxCriticalNesting = 0;
			}
		}
	}
	usleep(10000);
}
/*-----------------------------------------------------------*/

void vPortFindTicksPerSecond( void )
{

	/* Needs to be reasonably high for accuracy. */
	unsigned long ulTicksPerSecond = sysconf(_SC_CLK_TCK);
	printf( "Timer Resolution for Run TimeStats is %ld ticks per second.\n", ulTicksPerSecond );
}
/*-----------------------------------------------------------*/

unsigned long ulPortGetTimerValue( void )
{
struct tms xTimes;

	unsigned long ulTotalTime = times( &xTimes );
	/* Return the application code times.
	 * The timer only increases when the application code is actually running
	 * which means that the total execution times should add up to 100%.
	 */
	return ( unsigned long ) xTimes.tms_utime;

	/* Should check ulTotalTime for being clock_t max minus 1. */
	(void)ulTotalTime;
}
/*-----------------------------------------------------------*/
