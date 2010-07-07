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

#define DB_P(x) // x


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
	pthread_cond_t hCond;
	pthread_mutex_t hMutex;
	xTaskHandle hTask;
	unsigned portBASE_TYPE uxCriticalNesting;
} xThreadState;
/*-----------------------------------------------------------*/

static xThreadState *pxThreads;
static pthread_once_t hSigSetupThread = PTHREAD_ONCE_INIT;
static pthread_attr_t xThreadAttributes;
static pthread_mutex_t xSuspendResumeThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xSingleThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t hMainThread = ( pthread_t )NULL;
/*-----------------------------------------------------------*/

static volatile portBASE_TYPE xSentinel = 0;
static volatile portBASE_TYPE xHandover = 0;
static volatile portBASE_TYPE xSchedulerEnd = pdFALSE;
static volatile portBASE_TYPE xInterruptsEnabled = pdTRUE;
static volatile portBASE_TYPE xInterruptsCurrent = pdTRUE;
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
//static void prvResumeSignalHandler(int sig);
static void prvSetupSignalsAndSchedulerPolicy( void );
static void prvSuspendThread( pthread_t xThreadId );
static void pauseThread();
//static void prvResumeThread( pthread_t xThreadId );
static pthread_t prvGetThreadHandle( xTaskHandle hTask );
static pthread_cond_t prvGetThreadCondition( xTaskHandle hTask );
static pthread_mutex_t prvGetThreadMutex( xTaskHandle hTask );
static xTaskHandle prvGetThreadTask( pthread_t hThread );
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


 
//#define DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT

	static pthread_mutex_t xPrintfMutex = PTHREAD_MUTEX_INITIALIZER;

	#define debug_printf(...) ( (real_pthread_mutex_lock( &xPrintfMutex )|1)?( \
	(  \
	(NULL != (debug_task_handle = prvGetTaskHandle(pthread_self())) )? \
	(fprintf( stderr, "%20s(%li)\t%20s\t%i: ",debug_task_handle->pcTaskName,(long)pthread_self(),__func__,__LINE__)): \
	(fprintf( stderr, "%20s(%li)\t%20s\t%i: ","__unknown__",(long)pthread_self(),__func__,__LINE__)) \
	|1)?( \
	((fprintf( stderr, __VA_ARGS__ )|1)?real_pthread_mutex_unlock( &xPrintfMutex ):0) \
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
#else
	#define debug_error(...) ( (pthread_mutex_lock( &xPrintfMutex )|1)?( \
	(  \
	(NULL != (debug_task_handle = prvGetTaskHandle(pthread_self())) )? \
	(fprintf( stderr, "%20s(%li)\t%20s\t%i: ",debug_task_handle->pcTaskName,(long)pthread_self(),__func__,__LINE__)): \
	(fprintf( stderr, "%20s(%li)\t%20s\t%i: ","__unknown__",(long)pthread_self(),__func__,__LINE__)) \
	|1)?( \
	((fprintf( stderr, __VA_ARGS__ )|1)?pthread_mutex_unlock( &xPrintfMutex ):0) \
	):0 ):0 )

	#define debug_printf(...) 
#endif





//#define debug_printf(...) fprintf( stderr, __VA_ARGS__ );
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
tskTCB * prvGetTaskHandle( pthread_t hThread )
{
	portLONG lIndex;
	
	if (pxThreads==NULL) return NULL;
	
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == hThread )
		{
			return pxThreads[ lIndex ].hTask;
		}
	}
	return NULL;
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
	} /*else {
		sigset_t xSignals;
		sigemptyset( &xSignals );
		sigaddset( &xSignals, SIG_TICK );
		pthread_sigmask( SIG_BLOCK, &xSignals, NULL );
	} */


	/* No need to join the threads. */
	pthread_attr_init( &xThreadAttributes );
	pthread_attr_setdetachstate( &xThreadAttributes, PTHREAD_CREATE_DETACHED );

	/* Add the task parameters. */
	pxThisThreadParams->pxCode = pxCode;
	pxThisThreadParams->pvParams = pvParameters;

	vPortEnterCritical();

	lIndexOfLastAddedTask = prvGetFreeThreadState();

	/* Create a condition signal for this thread */
/*	pthread_condattr_t   condAttr;
	assert( 0 == pthread_condattr_init(&condAttr) );
	assert( 0 == pthread_cond_init( &( pxThreads[ lIndexOfLastAddedTask ].hCond ), &condAttr ) ); */
	
	/* Create a condition mutex for this thread */
/*	pthread_mutexattr_t mutexAttr;
	assert( 0 == pthread_mutexattr_init( &mutexAttr ) );
	assert( 0 == pthread_mutexattr_settype( &mutexAttr, PTHREAD_MUTEX_ERRORCHECK ) );
	assert( 0 == pthread_mutex_init( &( pxThreads[ lIndexOfLastAddedTask ].hMutex ), &mutexAttr ) );*/ 	
	
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
	
	/* Start the first task. */
	//prvResumeThread( prvGetThreadHandle( xTaskGetCurrentTaskHandle() ) );
	
	debug_printf( "Sending resume signal to %li\r\n", (long int) prvGetThreadHandle( xTaskGetCurrentTaskHandle() ) );
//	pthread_kill( prvGetThreadHandle( xTaskGetCurrentTaskHandle() ), SIG_RESUME );
	// Set resume condition for specific thread
	//pthread_cond_t hCond = prvGetThreadCondition( xTaskGetCurrentTaskHandle() );
	//assert( pthread_cond_signal( &hCond ) == 0 );
	
}
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
portBASE_TYPE xPortStartScheduler( void )
{
portBASE_TYPE xResult;
sigset_t xSignals;
sigset_t xSignalToBlock;
sigset_t xSignalsBlocked;
portLONG lIndex;

	debug_printf( "xPortStartScheduler\r\n" );

	/* Establish the signals to block before they are needed. */
	sigfillset( &xSignalToBlock );
	sigaddset( &xSignalToBlock, SIG_SUSPEND );
/*	sigaddset( &xSignalToBlock, SIG_RESUME );
	sigaddset( &xSignalToBlock, SIG_TICK ); */
	

	/* Block until the end */
	(void)pthread_sigmask( SIG_SETMASK, &xSignalToBlock, &xSignalsBlocked );

	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		pxThreads[ lIndex ].uxCriticalNesting = 0;
	}

	/* Start the timer that generates the tick ISR.  Interrupts are disabled
	here already. */
//	prvSetupTimerInterrupt();

	/* Start the first task. Will not return unless all threads are killed. */
	vPortStartFirstTask();

	/* This is the end signal we are looking for. */
	sigemptyset( &xSignals );
	sigaddset( &xSignals, SIG_RESUME );

	while( pdTRUE != xSchedulerEnd ) {
		struct timespec x;
		x.tv_sec=0;
		x.tv_nsec=1000000;
		nanosleep(&x,NULL);
		vPortSystemTickHandler(SIG_TICK);		
	}
	
	debug_printf( "Cleaning Up, Exiting.\n" );
	/* Cleanup the mutexes */
	xResult = pthread_mutex_destroy( &xSuspendResumeThreadMutex );
	xResult = pthread_mutex_destroy( &xSingleThreadMutex );
	vPortFree( (void *)pxThreads );

	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
portBASE_TYPE xNumberOfThreads;
portBASE_TYPE xResult;

	DB_P("vPortEndScheduler\r\n");

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
	 * xSingleThreadMutex is already owned by an original call to Yield. Therefore,
	 * simply indicate that a yield is required soon.
	 */

	DB_P("vPortYieldFromISR\r\n");

	xPendYield = pdTRUE;
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
	DB_P("vPortEnterCritical\r\n");

	vPortDisableInterrupts();
	uxCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
	DB_P("vPortExitCritical\r\n");
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
tskTCB * oldTask, * newTask;
	
	oldTask = xTaskGetCurrentTaskHandle();
	xTaskToSuspend = prvGetThreadHandle( oldTask ); //xTaskGetCurrentTaskHandle() );
	if(xTaskToSuspend != pthread_self() ) {
		debug_printf( "The current task isn't even us, suspending and letting that task start\r\n" );
		pauseThread();
		return;
	}

	assert( pthread_mutex_lock( &xSingleThreadMutex ) == 0);	

	vTaskSwitchContext();

	newTask = xTaskGetCurrentTaskHandle();
	xTaskToResume = prvGetThreadHandle( newTask ) ; //xTaskGetCurrentTaskHandle() );

	if ( prvGetThreadTask( pthread_self() ) != xTaskToResume )
	{
		/* Remember and switch the critical nesting. */
		prvSetTaskCriticalNesting( xTaskToSuspend, uxCriticalNesting );
		uxCriticalNesting = prvGetTaskCriticalNesting( xTaskToResume );

		assert( pthread_mutex_unlock( &xSingleThreadMutex ) == 0);

		/* pausing self if switching tasks, can now rely on other thread waking itself up */
		pauseThread();				

		/* Switch tasks. */
/*		debug_printf( "Yielding %li: From %li(%s) to %li(%s)\r\n",(long int) pthread_self(), (long int) xTaskToSuspend, oldTask->pcTaskName, (long int) xTaskToResume, newTask->pcTaskName);
		prvSuspendThread( xTaskToSuspend );
		debug_printf( "Yielded %li: From %li(%s) to %li(%s)\r\n",(long int) pthread_self(), (long int) xTaskToSuspend, oldTask->pcTaskName, (long int) xTaskToResume, newTask->pcTaskName); */
	}
	else 
	{

		assert( pthread_mutex_unlock( &xSingleThreadMutex ) == 0);
	}
}
/*-----------------------------------------------------------*/

void vPortDisableInterrupts( void )
{
	DB_P("vPortDisableInterrupts\r\n");
	xInterruptsEnabled = pdFALSE;
}
/*-----------------------------------------------------------*/

void vPortEnableInterrupts( void )
{
	DB_P("vPortEnableInterrupts\r\n");
	xInterruptsEnabled = pdTRUE;
}
/*-----------------------------------------------------------*/

portBASE_TYPE xPortSetInterruptMask( void )
{
portBASE_TYPE xReturn = xInterruptsEnabled;
	DB_P("vPortsetInterruptMask\r\n");
	xInterruptsEnabled = pdFALSE;
	return xReturn;
}
/*-----------------------------------------------------------*/

void vPortClearInterruptMask( portBASE_TYPE xMask )
{
	DB_P("vPortClearInterruptMask\r\n");
	xInterruptsEnabled = xMask;
}
/*-----------------------------------------------------------*/


void vPortSystemTickHandler( int sig )
{
pthread_t xTaskToSuspend;
pthread_t xTaskToResume;
portBASE_TYPE xInterruptValue;
tskTCB * oldTask, * newTask;
int lockResult;
	
	DB_P("vPortSystemTickHandler");
	xInterruptValue = xInterruptsEnabled;
	debug_printf( "\r\n\r\n" );
	debug_printf( "(xInterruptsEnabled = %i, xServicingTick = %i)\r\n", (int) xInterruptValue != 0, (int) xServicingTick != 0);
	if ( ( pdTRUE == xInterruptValue ) && ( pdTRUE != xServicingTick ) )
	{
		debug_printf( "Checking for lock ...\r\n" );
		lockResult = pthread_mutex_trylock( &xSingleThreadMutex );
//		lockResult = 0;
		debug_printf( "Done\r\n" );
		if ( 0 ==  lockResult)
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

			/* The only thread that can process this tick is the running thread. */
			if ( xTaskToSuspend != xTaskToResume )
			{
				/* Remember and switch the critical nesting. */
				prvSetTaskCriticalNesting( xTaskToSuspend, uxCriticalNesting );
				uxCriticalNesting = prvGetTaskCriticalNesting( xTaskToResume );
				
				debug_printf( "Swapping From %li(%s) to %li(%s)\r\n", (long int) xTaskToSuspend, oldTask->pcTaskName, (long int) xTaskToResume, newTask->pcTaskName);				
				prvSuspendThread( xTaskToSuspend );				/* Suspend the current task. */
				debug_printf( "Swapped From %li(%s) to %li(%s)\r\n", (long int) xTaskToSuspend, oldTask->pcTaskName, (long int) xTaskToResume, newTask->pcTaskName);
			}
			xServicingTick = pdFALSE;
			(void)pthread_mutex_unlock( &xSingleThreadMutex );
		}
		else
		{
			debug_printf( "Pending yield here (portYield has lock - hopefully)\r\n" );
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

	if ( 0 == pthread_mutex_lock( &xSingleThreadMutex ) )
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
			(void)pthread_mutex_unlock( &xSingleThreadMutex );
		}
		else
		{
			/* Resume the other thread. */
			//prvResumeThread( xTaskToResume );
			assert( 0 );
			
			/* Pthread Clean-up function will note the cancellation. */
			/* Release the execution. */
			uxCriticalNesting = 0;
			vPortEnableInterrupts();
			(void)pthread_mutex_unlock( &xSingleThreadMutex );
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

	xSentinel = 1; // tell creating block to resume
		
	/* want to block suspend when not the active thread */
	sigset_t xSignals;
	sigemptyset( &xSignals );
	sigaddset( &xSignals, SIG_SUSPEND );
	assert( pthread_sigmask( SIG_BLOCK, &xSignals, NULL ) == 0);
	
	debug_printf( "Delaying before the pause to make sure system knows about task\r\n" );
	struct timespec x;
	x.tv_sec=0;
	x.tv_nsec=10000000;
	nanosleep(&x,NULL);
	
	// wait for resume signal
	debug_printf("Pausing newly created thread\r\n");		
	pauseThread();
		
	// no longer want to block any signals
	sigemptyset( &xSignals );
	assert( pthread_sigmask( SIG_SETMASK, &xSignals, NULL ) == 0);
	debug_printf("Starting first run\r\n");

	//xHandover = 1;
	//assert ( 0 == pthread_mutex_lock( &xSingleThreadMutex ) ) 

	pvCode( pParams );

	pthread_cleanup_pop( 1 );
	return (void *)NULL;
}
/*-----------------------------------------------------------*/

void pauseThread() 
{
	debug_printf("\r\n");
	
	while (1) {
		if( pthread_self() == prvGetThreadHandle(xTaskGetCurrentTaskHandle() ) )
			return;
		else {
			struct timespec x;
			x.tv_sec=0;
			x.tv_nsec=10000;
			nanosleep(&x,NULL);
			sched_yield();
		}

	}
		xTaskHandle hTask = prvGetThreadTask( pthread_self() );
		pthread_cond_t hCond = prvGetThreadCondition( hTask );
		pthread_mutex_t hMutex = prvGetThreadMutex( hTask );
	//	while( !shouldResume ) {
	pthread_cond_wait( &hCond, &hMutex );
	//	}
}

void prvSuspendSignalHandler(int sig)
{
sigset_t xBlockSignals;

	debug_printf( "Caught signal %i\r\n", sig );
	/* Check that we aren't suspending when we should be running.  This bug would need tracking down */
	//assert( pthread_self() != prvGetThreadHandle(xTaskGetCurrentTaskHandle() ) ); 

	/* Block further suspend signals.  They need to go to their thread */
	sigemptyset( &xBlockSignals );
	sigaddset( &xBlockSignals, SIG_SUSPEND );
	assert( pthread_sigmask( SIG_BLOCK, &xBlockSignals, NULL ) == 0);

	/* Unlock the Single thread mutex to allow the resumed task to continue. */
	//assert ( 0 == pthread_mutex_unlock( &xSingleThreadMutex ) );

	// Set resume condition for specific thread
/*	pthread_cond_t hCond = prvGetThreadCondition( xTaskGetCurrentTaskHandle() );
	assert( pthread_cond_signal( &hCond ) == 0 ); */

	pauseThread(); 
	
	while( pthread_self() != prvGetThreadHandle( xTaskGetCurrentTaskHandle() ) )
	{
		debug_printf( "Incorrectly woke up.  Repausing\r\n" );
		pauseThread();
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

void prvSuspendThread( pthread_t xThreadId )
{
	debug_printf( "Suspending %li\r\n", (long int) xThreadId);
	/* Set-up for the Suspend Signal handler? */
	xSentinel = 0;
	
	if( pthread_self() == xThreadId ) {
		debug_printf( "Pausing self, no reason to call signal handler\r\n" );
		pauseThread();
	}
	else {
		debug_printf( "About to kill %li\r\n", (long int) xThreadId );
		assert( pthread_kill( xThreadId, SIG_SUSPEND ) == 0);
		debug_printf( "Killed %li\r\n", (long int) xThreadId );
	}
	

//	while ( ( xSentinel == 0 ) && ( pdTRUE != xServicingTick ) )
	{
//		debug_printf( "sched_yield()\r\n" );
		sched_yield();
	} 
}
/*-----------------------------------------------------------*/

/*void prvResumeSignalHandler(int sig)
{
	
	DB_P("prvResumeSignalHandler\r\n");

	debug_printf( "prvResumeSignalHandler getLock");
	while(1);
	// Yield the Scheduler to ensure that the yielding thread completes. 
	if ( 0 == pthread_mutex_lock( &xSingleThreadMutex ) )
	{
		debug_printf( "prvResumeSignalHandler: unlocking xSingleThreadMutex (%li)\r\n", (long int) pthread_self());
		(void)pthread_mutex_unlock( &xSingleThreadMutex );
	}
}*/
/*-----------------------------------------------------------*/

/*void prvResumeThread( pthread_t xThreadId )
{
portBASE_TYPE xResult;

	DB_P("prvResumeThread\r\n");
	debug_printf( "getLock\r\n" );
	if ( 0 == pthread_mutex_lock( &xSuspendResumeThreadMutex ) )
	{		
		debug_printf( "Resuming %li\r\n", (long int) xThreadId );
		if ( pthread_self() != xThreadId )
		{
			//xResult = pthread_kill( xThreadId, SIG_RESUME );
			debug_printf( "No longer doing anything.  Suspend handler for previous thread should start %li\r\n", (long int) xThreadId );
		}
		else {
			debug_printf( "Thread attempting to resume itself.  This is not expected behavior\r\n" );
			kill( getpid(), SIGKILL );
		}

		xResult = pthread_mutex_unlock( &xSuspendResumeThreadMutex );
	}
	else {
		debug_printf("Error getting lock to resume thread\r\n");
		kill( getpid(), SIGKILL );
	}

} */
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
pthread_mutex_t prvGetThreadMutex( xTaskHandle hTask ) 
{	
pthread_mutex_t hMutex;
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
xTaskHandle prvGetThreadTask( pthread_t hThread ) 
{
xTaskHandle hTask;
portLONG lIndex;
	
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
pthread_cond_t prvGetThreadCondition( xTaskHandle hTask )
{
pthread_cond_t hCond;
portLONG lIndex;

	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hTask == hTask )
		{
			hCond = pxThreads[ lIndex ].hCond;
			break;
		}
	}
	return hCond;
}

/*-----------------------------------------------------------*/
pthread_t prvGetThreadHandle( xTaskHandle hTask )
{
	pthread_t hThread = ( pthread_t )NULL;
	portLONG lIndex;
	
	DB_P("prvGetThreadHandle\r\n");
	
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

	DB_P("prvGetFreeThreadState\r\n");

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
	DB_P("prvSetTaskCriticalNesting\r\n");
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

	DB_P("prvGetTaskCriticalNesting\r\n");

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

	DB_P("prvDeleteThread\r\n");

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
}
/*-----------------------------------------------------------*/

void vPortFindTicksPerSecond( void )
{
	DB_P("vPortFindTicksPerSecond\r\n");

	/* Needs to be reasonably high for accuracy. */
	//unsigned long ulTicksPerSecond = sysconf(_SC_CLK_TCK);
	//printf( "Timer Resolution for Run TimeStats is %ld ticks per second.\n", ulTicksPerSecond );
}
/*-----------------------------------------------------------*/

unsigned long ulPortGetTimerValue( void )
{
struct tms xTimes;

	DB_P("ulPortGetTimerValue\r\n");

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
