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

// vPortYield
// vPortSystemTickHandler -- preempts when necessary (in our case just idle task)
// xPortStartScheduler
// interrupts

/**
 * In addition this method needs to be able to swap out threads via two mechanisms:
 * 1. cooperative where a task calls vPortYield and is told to swap out
 * 2. preemptive where a separate pthread calls vPortSystemTickHandler and swaps it out
 *
 * We want three functions associated with this
 * pauseThread(thread id) -- called from System Tick handler before doing anything. 
 *   returns when that thread confirms it has stopped
 * resumeThread(thread id) -- called from Systme Tick handler and other threads to 
 *   start a different thread.  Doesn't return until it's confirmed that thread started
 * pauseThread(self) -- called from vPortYield after resume thread confirmed to work
 *
 * There will be a single semaphore that is help by the running thread (including the 
 * tick handler) to mimic a single core machine without multithreading.
 * 
 * SystemTick
 * pauseThread(running)
 * claim running Semaphore
 * vTaskSwitch
 * release running semaphore
 * resume new thread
 * assert it resumed
 *
 * vPortYield
 * release running semaphore
 * resume thread
 * assert it resumed
 * pause
 */

// Critical sections: Disable interrupts but can yield
// Suspend scheduler: Makes sure that we don't actually swap tasks
// Interrupts - I don't think they do block the tick handler?  Uses two irqs - the PEND and SVC

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
	//#define CHECK_TASK_RESUMES
#endif

/* Parameters to pass to the newly created pthread. */
typedef struct XPARAMS
{
	pdTASK_CODE pxCode;
	void *pvParams;
} xParams;

enum thread_status {RUNNING, STORED, STOPPED, CREATED, DESTROYED};

/* Each task maintains its own interrupt status in the critical nesting variable. */
typedef struct THREAD_SUSPENSIONS
{
	volatile pthread_t hThread;
	pthread_cond_t * hCond;
	pthread_mutex_t * hMutex;
	xTaskHandle hTask;
	portBASE_TYPE xThreadState;
	volatile enum thread_status status;
	unsigned portBASE_TYPE uxCriticalNesting;
	unsigned portBASE_TYPE xInterruptsEnabled;
	char * name;
} xThreadState;
/*-----------------------------------------------------------*/

static xThreadState pxThreads[MAX_NUMBER_OF_TASKS];
static pthread_once_t hSigSetupThread = PTHREAD_ONCE_INIT;
static pthread_attr_t xThreadAttributes;
static pthread_mutex_t xRunningThread = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xSuspendResumeThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xSwappingThreadMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t xIrqMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t hMainThread = ( pthread_t )NULL;
/*-----------------------------------------------------------*/

static volatile portBASE_TYPE xSentinel = 0;
static volatile portBASE_TYPE xRunning = pdFALSE;
static volatile portBASE_TYPE xSuspended = pdFALSE;
static volatile portBASE_TYPE xStarted = pdFALSE;
static volatile portBASE_TYPE xHandover = 0;
static volatile portBASE_TYPE xSchedulerEnd = pdFALSE;
volatile portBASE_TYPE xInterruptsEnabled = pdTRUE;
static volatile portBASE_TYPE xServicingTick = pdFALSE;
static volatile portBASE_TYPE xPendYield = pdFALSE;
static volatile portLONG lIndexOfLastAddedTask = 0;
static volatile unsigned portBASE_TYPE uxCriticalNesting;
/*-----------------------------------------------------------*/

/**
 * Methods to facilitate the task switching
 */

void maskSuspend();
void unmaskSuspend();

/**
 * Stops another thread that is running (from System Tick handler)
 * returns when that thread has done so.  Sends a signal to that thread
 * when then goes into it's signal handler.  It will changes it's running
 * status to stopped which this thread will wait for.
 */
static int pauseOtherThread(xTaskHandle hTask);

/**
 * Prepares a task to pause (stores critical nesting, releases running semaphore)
 * but doesn't actually sleep
 */
void storeSelf();

/**
 * Pause self, either from the signal handler or from its own thread.  Marks
 * the status to stopped then waits for condition.  When resumes marks condition
 * as true
 */
void pauseSelf();

/**
 * Resumes another thread that is running (from either System Tick handler
 * or another thread) and returns when that thread has done so.  Sends resume
 * condition to that thread and waits for the status flag to change to running.
 */
static void resumeThread(xTaskHandle hTask);

/**
 * Claims the running semaphore or fails
 */
static void claimRunningSemaphore(int source);

/**
 * Claims the running semaphore or fails
 */
static void releaseRunningSemaphore();


/*
 * Setup the timer to generate the tick interrupts.
 */
static void *prvWaitForStart( void * pvParams );
static void prvSuspendSignalHandler(int sig);
static void prvSetupSignalsAndSchedulerPolicy( void );
static pthread_t prvGetThreadHandle( xTaskHandle hTask );
static pthread_cond_t * prvGetConditionHandle( xTaskHandle hTask );
static pthread_mutex_t * prvGetMutexHandle( xTaskHandle hTask );
static xTaskHandle prvGetTaskHandle( pthread_t hThread );
static void prvSetThreadStatus( pthread_t hThread, enum thread_status status );
static enum thread_status prvGetThreadStatus( pthread_t hThread );
static portLONG prvGetFreeThreadState( void );
static void prvSetTaskCriticalNesting( pthread_t xThreadId, unsigned portBASE_TYPE uxNesting );
static unsigned portBASE_TYPE prvGetTaskCriticalNesting( pthread_t xThreadId );
static void prvDeleteThread( void *xThreadId );
void prvSetTaskInterrupt( pthread_t xThreadId, unsigned portBASE_TYPE xInterruptsEnabled );
unsigned portBASE_TYPE prvGetTaskInterrupt( pthread_t xThreadId );
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
//#define ERROR_OUTPUT
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

char * threadToName(pthread_t t)
{
	char * sys = "System Tick";
	
	xTaskHandle hTask =  prvGetTaskHandle(t);
	if(hTask == NULL)
		return sys;
	return (char *) ((tskTCB*)hTask)->pcTaskName;
}
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

	//printf( "Got index for new task %i\r\n", lIndexOfLastAddedTask );
	
	/* Create a condition signal for this thread */
	pxThreads[ lIndexOfLastAddedTask ].hCond = ( pthread_cond_t *) malloc( sizeof( pthread_cond_t ) );
	assert( 0 == pthread_cond_init(  pxThreads[ lIndexOfLastAddedTask ].hCond , NULL ) ); //&condAttr ) ); 
	debug_printf("Cond: %li\r\n", *( (long int *) &pxThreads[ lIndexOfLastAddedTask ].hCond) );
	
	/* Create a condition mutex for this thread */
	pxThreads[ lIndexOfLastAddedTask ].hMutex = ( pthread_mutex_t *) malloc( sizeof( pthread_mutex_t ) );	
	assert( 0 == pthread_mutex_init( pxThreads[ lIndexOfLastAddedTask ].hMutex, NULL ) ); //&mutexAttr ) );
	debug_printf("Mutex: %li\r\n", *( (long int *) &pxThreads[ lIndexOfLastAddedTask ].hMutex) );
		
	/* Create a thread and store it's handle number */
	xSentinel = 0;
	assert( 0 == pthread_create( (pthread_t *) &( pxThreads[ lIndexOfLastAddedTask ].hThread ), &xThreadAttributes, prvWaitForStart, (void *)pxThisThreadParams ) );		

	/* Wait until the task suspends. */
	while ( xSentinel == 0 );

	//printf( "Task suspended detected for %i\r\n", lIndexOfLastAddedTask);

	vPortExitCritical();

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

void vPortStartFirstTask( void )
{
	/* Initialise the critical nesting count ready for the first task. */
	uxCriticalNesting = 0;

	fprintf(stdout, "vPortStartFirstTask\r\n");
		
	/* Start the first task. */
	xInterruptsEnabled = pdTRUE;
	xRunning = 1;
	unmaskSuspend();
	
	resumeThread( xTaskGetCurrentTaskHandle() );
}
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
portBASE_TYPE xPortStartScheduler( void )
{
portBASE_TYPE xResult;
sigset_t xSignalToBlock;
//portLONG lIndex;

	fid = fopen("log.txt", "w");
	

	debug_printf( "xPortStartScheduler\r\n" );

	/* Establish the signals to block before they are needed. */
	sigfillset( &xSignalToBlock );
	sigaddset( &xSignalToBlock, SIG_SUSPEND );

	/* Block until the end */
	(void)pthread_sigmask( SIG_SETMASK, &xSignalToBlock, NULL );
/*
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		pxThreads[ lIndex ].uxCriticalNesting = 0;
		pxThreads[ lIndex ].status = CREATED;
	}
*/
	/* Start the first task. Will not return unless all threads are killed. */
	vPortStartFirstTask();

	usleep(500000);
	int i = 0;
	while( pdTRUE != xSchedulerEnd ) {
		usleep(portTICK_RATE_MICROSECONDS * 0.8);
		vPortSystemTickHandler(SIG_TICK);	
		i++;
		if (i % 1000 == 0)
			fprintf(stderr,".");
		//if (i % 5000 == 0)
		//	printTasks();
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
	uxCriticalNesting--;
	assert(uxCriticalNesting >= 0);

	if( uxCriticalNesting == 0)
	{
		portENABLE_INTERRUPTS();
	}
}
/*-----------------------------------------------------------*/

int yield_locking = 0;
tskTCB *lastYield;
int yield_count = 0;
void vPortYield( void )
{
	if(pthread_mutex_trylock( &xSwappingThreadMutex )== EBUSY) {
		// The tick handler will just pause this thread
		return;
	} else {
		lastYield =  xTaskGetCurrentTaskHandle();
		vTaskSwitchContext();
		if(prvGetThreadHandle( xTaskGetCurrentTaskHandle()) != pthread_self() ) {
			// Time to sleep
			//fprintf(stdout, "Yielding from %s\r\n", threadToName(pthread_self()));
			yield_count++;
			storeSelf();
			resumeThread(xTaskGetCurrentTaskHandle());
			pthread_mutex_unlock( &xSwappingThreadMutex );
			pauseSelf();
			claimRunningSemaphore(3);
			unmaskSuspend();
		} else
			pthread_mutex_unlock( &xSwappingThreadMutex );

	}
	
}
/*-----------------------------------------------------------*/

void maskSuspend()
{
	sigset_t xSignals;
	sigemptyset( &xSignals );
	sigaddset( &xSignals, SIG_SUSPEND );
	pthread_sigmask( SIG_SETMASK, &xSignals, NULL );
}
void unmaskSuspend()
{
	sigset_t xSignals;
	sigemptyset( &xSignals );
	pthread_sigmask( SIG_SETMASK, &xSignals, NULL );
}

int irq_lock;
tskTCB * lastInterruptDisable;
void vPortDisableInterrupts( void )
{

	if (prvGetTaskHandle(pthread_self()) != NULL)
		assert(prvGetThreadStatus(pthread_self()) == RUNNING);

	lastInterruptDisable = (tskTCB *)xTaskGetCurrentTaskHandle();
	//debug_printf("\r\n");
	maskSuspend();
	irq_lock = 2;
//	assert( pthread_mutex_trylock( &xIrqMutex ) == 0);
	while( pthread_mutex_trylock( &xSwappingThreadMutex ) != 0)
	{
		// Give task schedule a chance to suspend this
		unmaskSuspend();
		//usleep(1);
		maskSuspend();
	}
	irq_lock = 1;
	xInterruptsEnabled = pdFALSE;
//	assert( pthread_mutex_unlock( &xIrqMutex) == 0);

	assert( pthread_mutex_unlock( &xSwappingThreadMutex) == 0);

	irq_lock = 0;
	unmaskSuspend();
}
/*-----------------------------------------------------------*/

/**
 * Obvious the simulated interrupts status must be thread safe.
 * This means we also must mask these threads ever being suspended
 * while they have the irqMutex.
 */
void vPortEnableInterrupts( void )
{
	//debug_printf("\r\n");
	if (prvGetTaskHandle(pthread_self()) != NULL)
		assert(prvGetThreadStatus(pthread_self()) == RUNNING);
	
	maskSuspend();
	irq_lock = 4;
	//	assert( pthread_mutex_trylock( &xIrqMutex ) == 0);
	while( pthread_mutex_trylock( &xSwappingThreadMutex ) != 0)
	{
		// Give task schedule a chance to suspend this
		unmaskSuspend();
		//usleep(1);
		maskSuspend();
	}

	irq_lock = 3;
//	assert( pthread_mutex_trylock( &xIrqMutex ) == 0);
	xInterruptsEnabled = pdTRUE;
	assert( pthread_mutex_unlock( &xSwappingThreadMutex) == 0);
//	assert( pthread_mutex_unlock( &xIrqMutex) == 0);
	irq_lock = 0;
	unmaskSuspend();	
}
/*-----------------------------------------------------------*/

portBASE_TYPE xPortSetInterruptMask( void )
{
portBASE_TYPE xReturn = xInterruptsEnabled;
	debug_printf("\r\n");
//	xInterruptsEnabled = pdFALSE;
	vPortDisableInterrupts();
	return xReturn;
}
/*-----------------------------------------------------------*/

void vPortClearInterruptMask( portBASE_TYPE xMask )
{
	debug_printf("\r\n");
	if(xMask == pdTRUE)
		vPortEnableInterrupts();
	else
		vPortDisableInterrupts();
//	xInterruptsEnabled = xMask;
}
/*-----------------------------------------------------------*/


tskTCB * oldTask, * newTask;
int preemptive_count = 0;
int noirq_count = 0;
void vPortSystemTickHandler( int sig )
{
	/* In this real hardware this is what happens
	 * 0. if preemption set pending sys call flag (happens when all isr's finished)
	 * 1. set interrupt mask
	 * 2. increment tick count
	 * 3. restore interrupt mask
	 * 4. exits which triggers the pending sys call (?)
	 *
	 * Pending sys call:
	 * 1. store the current tcb
	 * 2. something with "msr basepri, r0" which changes the interrupt priority mask
	 * 3. switches context
	 * 4. restores the interrupt priority
	 * 5. loads the new tcb
	 * 
	 */
	 
	debug_printf( "\r\n\r\n" );
	debug_printf( "(xInterruptsEnabled = %i, xServicingTick = %i)\r\n", (int) xInterruptsEnabled != 0, (int) xServicingTick != 0);

/*	if (pthread_mutex_trylock( &xIrqMutex ) == EBUSY) {
		fprintf(stdout, "Systick could not block interrupts\r\n");
		return;
	} */

	if(pthread_mutex_trylock( &xSwappingThreadMutex )== EBUSY) {
		//fprintf(stdout,"Can't get swapping lock for tick handler\r\n");
		return;
	}

#if ( configUSE_PREEMPTION == 1 )			
	if (pdTRUE == xInterruptsEnabled )
	{
		//fprintf(stdout, "System tick executing.  Suspending %s\r\n", ((tskTCB*)xTaskGetCurrentTaskHandle())->pcTaskName);
		
		//printTasks();
		
		// If that thread won't pause something is going on
		if (pauseOtherThread(xTaskGetCurrentTaskHandle()) == -1) {
			tskTCB *non_stopping = (tskTCB *) xTaskGetCurrentTaskHandle();
			fprintf(stdout, "Can't pause thread %s\r\n", non_stopping->pcTaskName);
		}
		
		vTaskIncrementTick();
		
		oldTask = xTaskGetCurrentTaskHandle();
		claimRunningSemaphore(2);
		vTaskSwitchContext();
			preemptive_count++;
		releaseRunningSemaphore();
		resumeThread(xTaskGetCurrentTaskHandle());
		newTask = xTaskGetCurrentTaskHandle();

	}
	else
		noirq_count++;
#endif

//	assert( pthread_mutex_unlock( &xIrqMutex) == 0);	
	assert( pthread_mutex_unlock( &xSwappingThreadMutex ) == 0 );

}
/*-----------------------------------------------------------*/

int init_destroyed = 0;
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
				fprintf(stderr, "HEREEREREE\r\n");
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
			(void)pthread_mutex_unlock( &xSwappingThreadMutex );
			prvSetThreadStatus( xTaskToDelete, DESTROYED );
			init_destroyed = 1;
			releaseRunningSemaphore();
			//vPortEnableInterrupts();
			xInterruptsEnabled = pdTRUE;
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

	//fprintf(stdout,"Thread started, waiting till handle is added\r\n");

	xSentinel = 1;

	while( prvGetTaskHandle( pthread_self() ) == NULL ){
		sched_yield();
		usleep(1);
	}

	//fprintf(stdout,"Handle added%s\r\n",threadToName(pthread_self()));
	
	/* Want to delay briefly until we have explicit resume signal as otherwise the */
	/* current task variable might be in the wrong state */	
	/* Block further suspend signals.  They need to go to their thread */

	xTaskHandle hTask = prvGetTaskHandle( pthread_self() );
	portLONG lIndex;	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS && pxThreads[ lIndex ].hTask != hTask; lIndex++ );
	assert(pxThreads[ lIndex ].hTask == hTask);
	
	pxThreads[lIndex].name = (char *) ((tskTCB *) hTask)->pcTaskName;

	xInterruptsEnabled = pdTRUE;
	if(1 || prvGetTaskHandle(pthread_self()) != xTaskGetCurrentTaskHandle()) {
		pauseSelf();
		claimRunningSemaphore(7);
		unmaskSuspend();
	}
	else {
		fprintf(stdout, "New thread wants to run right away %s\r\n", threadToName(pthread_self()));

		assert(pthread_mutex_trylock( &xSwappingThreadMutex )!= EBUSY);
		
		// Find the previously running thread.  xTaskGetCurrentHandle no longer reflects current state
		
		portLONG lIndex;	
		for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS && pxThreads[ lIndex ].hTask != hTask; lIndex++ )
			if(pxThreads[ lIndex ].status == RUNNING)
				break;
				
		pthread_mutex_unlock( &xSwappingThreadMutex );
		
		assert(pxThreads[ lIndex ].hTask != NULL); // Should find a thread that is running
		
		if( pxThreads[lIndex].hThread != pthread_self())
			assert(pauseOtherThread(pxThreads[ lIndex ].hTask) == 0);
		
		prvSetThreadStatus(pthread_self(), RUNNING);
		claimRunningSemaphore(6);
	}
	
	vPortEnableInterrupts();
	
	sigemptyset( &xSignals );
	assert( pthread_sigmask( SIG_SETMASK, &xSignals, NULL ) == 0);

//	fprintf(stderr,"Starting first run\r\n");
//	claimRunningSemaphore(4);

	pvCode( pParams );

	pthread_cleanup_pop( 1 );
	return (void *)NULL;
}
/*-----------------------------------------------------------*/

extern volatile unsigned portBASE_TYPE uxSchedulerSuspended;

void prvSuspendSignalHandler(int sig)
{
	if (prvGetThreadStatus(pthread_self()) != RUNNING) {
		fprintf(stderr, "Caught erroneous suspend signal (%d): %s\r\n", sig, threadToName(pthread_self()));
		return;
	}
	else {
		//fprintf(stderr, "Caught suspend signal (%d): %s\r\n", sig, threadToName(pthread_self()));
		storeSelf();
		pauseSelf();
		assert(prvGetTaskHandle(pthread_self()) == xTaskGetCurrentTaskHandle());
		claimRunningSemaphore(1);
		unmaskSuspend();
	}
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
	
	//pxThreads = ( xThreadState *)pvPortMalloc( sizeof( xThreadState ) * MAX_NUMBER_OF_TASKS );
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
static void prvSetThreadStatus( pthread_t hThread, enum thread_status status ) 
{
	portLONG lIndex;
	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == hThread )
		{
			pxThreads[ lIndex ].status = status;
			return;
		}
	}
	fprintf(stderr, "Could not find thread %lx\r\n", (long int) hThread);
}

/*-----------------------------------------------------------*/
static enum thread_status prvGetThreadStatus( pthread_t hThread )
{
	portLONG lIndex;
	
	/* If not initialized yet */
	if( pxThreads  == NULL ) return DESTROYED;
	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == hThread )
		{
			return pxThreads[ lIndex ].status;				}
	}
	return DESTROYED;
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

void prvSetTaskInterrupt( pthread_t xThreadId, unsigned portBASE_TYPE xInterruptsEnabled )
{
	portLONG lIndex;
	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == xThreadId )
		{
			pxThreads[ lIndex ].xInterruptsEnabled = xInterruptsEnabled;
			break;
		}
	}
}
/*-----------------------------------------------------------*/

unsigned portBASE_TYPE prvGetTaskInterrupt( pthread_t xThreadId )
{
	unsigned portBASE_TYPE xInterruptsEnabled = 0;
	portLONG lIndex;
	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == xThreadId )
		{
			xInterruptsEnabled = pxThreads[ lIndex ].uxCriticalNesting;
			break;
		}
	}
	return xInterruptsEnabled;
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


/**
 * Prepares a task to pause (stores critical nesting, releases running semaphore)
 * but doesn't actually sleep.  Must be follows by a pauseSelf as soon as another
 * thread is running.
 */
void storeSelf()
{
	//assert( xInterruptsEnabled );

	xTaskHandle hTask = prvGetTaskHandle( pthread_self() );

	/* Block further suspend signals.  They need to go to their thread */
	maskSuspend();
	
	prvSetTaskCriticalNesting( hTask, uxCriticalNesting );
	prvSetTaskInterrupt( hTask, xInterruptsEnabled );

	releaseRunningSemaphore();
	
	prvSetThreadStatus( pthread_self(), STORED );

}

/**
 * Pause self, either from the signal handler or from its own thread.  Marks
 * the status to stopped then waits for condition.  When resumes marks condition
 * as true
 */
void pauseSelf()
{
	//fprintf(stdout,  "Pausing self: %s\r\n", threadToName( pthread_self() ) );

	int xResult;
	xTaskHandle hTask = prvGetTaskHandle( pthread_self() );
	pthread_cond_t * hCond = prvGetConditionHandle( hTask );
	pthread_mutex_t * hMutex = prvGetMutexHandle( hTask );		
	
	assert(hCond);
	assert(hMutex);

	prvSetThreadStatus( pthread_self(), STOPPED );
	
	xResult = ETIMEDOUT;
	while( xResult != 0 || hTask != xTaskGetCurrentTaskHandle() ) {
		struct timeval tv;
		struct timespec ts;
		gettimeofday( &tv, NULL );
		ts.tv_sec = tv.tv_sec + 1;
		ts.tv_nsec = tv.tv_usec * 1000; 
		xResult = pthread_cond_timedwait( hCond, hMutex, &ts );
	
		assert( xResult != EINVAL );
	
		if (xResult == ETIMEDOUT && pthread_self() == prvGetThreadHandle(xTaskGetCurrentTaskHandle()) && 
			init_destroyed) {
			fprintf(stdout,"Timed out should be running %s\r\n", threadToName( pthread_self() ));
			break;
		}
	}
	
	/* Restore the critical nesting */
	uxCriticalNesting = prvGetTaskCriticalNesting( hTask );
	//xInterruptsEnabled = prvGetTaskInterrupt( hTask );
	if(uxCriticalNesting == 0)
		xInterruptsEnabled = pdTRUE;
	
	prvSetThreadStatus( pthread_self(), RUNNING );
//	if (xResult == 0)
//		fprintf(stdout, "Thread resumed from signal %s\r\n", threadToName( pthread_self() ));
//	else 
//		fprintf(stdout, "Thread resumed from timeout %s\r\n", threadToName( pthread_self() ));

}

/**
 * Stops another thread that is running (from System Tick handler)
 * returns when that thread has done so.  Sends a signal to that thread
 * when then goes into it's signal handler.  It will changes it's running
 * status to stopped which this thread will wait for.
 */
static int pauseOtherThread(xTaskHandle hTask)
{
	const int MAX_TIME = 10000; // us
	const int MAX_ATTEMPTS = 100;
	
	assert(xInterruptsEnabled == pdTRUE);

	portLONG lIndex;	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS && pxThreads[ lIndex ].hTask != hTask; lIndex++ );
	assert(pxThreads[ lIndex ].hTask == hTask);

	if(pxThreads[ lIndex ].status != RUNNING) {
		// If this thread doesn't indicate it is running make sure no others are
		for( int i = 0; i < MAX_NUMBER_OF_TASKS; i++) {
			assert( pxThreads[i].hTask == 0 || pxThreads[i].status != RUNNING);
		}
		//fprintf(stdout, "Attempted to stop thread that is not running %s by %s\r\n", threadToName(pxThreads[lIndex].hThread), threadToName(pthread_self()));
		return -1;
	}
	

	for (int i = 0; i < MAX_ATTEMPTS; i++) {
		// Trigger signal handler which will call pauseSelf
		pthread_t thread_to_supend = prvGetThreadHandle( hTask );
		//fprintf(stdout,  "Requesting pause of thread %s from %s.\r\n", threadToName(thread_to_supend), threadToName( pthread_self() ) );
		
		assert( pthread_kill( thread_to_supend, SIG_SUSPEND ) == 0);		
		
		int start_time = clock();
		while( (clock() - start_time) < MAX_TIME ) {
			if(pxThreads[ lIndex ].status == STOPPED || pxThreads[ lIndex ].status == STORED) {
				//fprintf(stdout, "Pause detected of %s by %s\r\n", threadToName(pxThreads[lIndex].hThread), threadToName(pthread_self()));
				return 0;
			}
		}

		//fprintf(stdout, "Sending pause signal from %s to %s: Try %d\r\n", threadToName(pthread_self()), threadToName(prvGetThreadHandle( hTask )), i);
	}
	assert(0);
	
	return -2;
}

/**
 * Resumes another thread that is running (from either System Tick handler
 * or another thread) and returns when that thread has done so.  Sends resume
 * condition to that thread and waits for the status flag to change to running.
 */
tskTCB *lastResumeAttempt;
tskTCB *lastResume;
static void resumeThread(xTaskHandle hTask)
{
	const unsigned int MAX_TIME = 100; // us
	const int MAX_ATTEMPTS = 2000;

	lastResumeAttempt = (tskTCB *) hTask;

	portLONG lIndex;	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS && pxThreads[ lIndex ].hTask != hTask; lIndex++ );
	assert(pxThreads[ lIndex ].hTask == hTask);


	for(int i = 0; i < MAX_ATTEMPTS; i++) {
		//pthread_t thread_to_resume = prvGetThreadHandle( hTask );
		//fprintf(stdout,  "Requesting resume of thread %s from %s.\r\n", threadToName(thread_to_resume), threadToName( pthread_self() ) );
		/* Set resume condition for specific thread */
		pthread_cond_t * hCond = prvGetConditionHandle( hTask );
		assert( pthread_cond_signal( hCond ) == 0 );
		
		unsigned int start_time = clock();
		while( (clock() - start_time) < MAX_TIME ) {
			if(pxThreads[ lIndex ].status == RUNNING) {
				//fprintf(stdout, "Resume detected of %s by %s\r\n", threadToName(pxThreads[lIndex].hThread), threadToName(pthread_self()));
				lastResume = (tskTCB *) hTask;
				return;
			}
		}
		
		//fprintf(stdout, "Sending resume signal from %s to %s (%d): Try %d\r\n", threadToName(pthread_self()), threadToName(hTask), lIndex, i);

	}
	//fprintf(stdout, "Railed to resume from %s to %s (%d)\r\n", threadToName(pthread_self()), threadToName(hTask), lIndex);
	// Thread resumption timed out
	assert(0);
}

/**
 * Claims the running semaphore or fails.  Does not block as failing to get this
 * lock indicates another failure upstream.
 */
tskTCB *lastClaim;
int lastClaimType;
int claim_count = 0;
static void claimRunningSemaphore(int source)
{
	//fprintf(stderr,"Claimed the semaphore(%d) %s\r\n", source, threadToName(pthread_self()));
	xTaskHandle hTask = prvGetTaskHandle( pthread_self() );

	// And they succeed
	int succeed = 0;
	for (int i = 0; i < 100 && !succeed; i++) {
		
		succeed = (pthread_mutex_trylock( &xRunningThread ) == 0);
		if (!succeed)
			usleep(10);
	}
	
	if(succeed) {
		lastClaim = (tskTCB *) hTask;
		lastClaimType = source;
		claim_count ++;
		
		if(hTask != NULL && hTask !=  xTaskGetCurrentTaskHandle()) {
			fprintf(stdout,"This shouldn't have happened\r\b");
			storeSelf();
			pauseSelf();
			claimRunningSemaphore(10);
			unmaskSuspend();
		}
	} else {
		
		/* Block further suspend signals.  They need to go to their thread */
		maskSuspend();

		/* Store self without releasing semaphore */
		prvSetTaskCriticalNesting( hTask, uxCriticalNesting );
		prvSetTaskInterrupt( hTask, xInterruptsEnabled );
		
		releaseRunningSemaphore();
		
		prvSetThreadStatus( pthread_self(), STORED );
	
		pauseSelf();
		claimRunningSemaphore(11);
		unmaskSuspend();
	}
	

	// Make sure the right task is trying for this (SystemTick doesn't have a handle)
	assert( hTask == NULL || hTask ==  xTaskGetCurrentTaskHandle() );
}

/**
 * Claims the running semaphore or fails
 */
tskTCB *lastRelease;
int release_count = 0;
static void releaseRunningSemaphore()
{
	xTaskHandle hTask = prvGetTaskHandle( pthread_self() );
	assert( lastClaim == hTask );
	assert( 0 == pthread_mutex_unlock( &xRunningThread ) ); 
	//fprintf(stderr,"Released the semaphore %s\r\n", threadToName(pthread_self()));
	
	lastRelease = (tskTCB *) hTask;

	release_count++;
}


