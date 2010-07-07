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

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
/*-----------------------------------------------------------*/

#define MAX_NUMBER_OF_TASKS 		( _POSIX_THREAD_THREADS_MAX )
/*-----------------------------------------------------------*/

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
	xTaskHandle hTask;
	unsigned portBASE_TYPE uxCriticalNesting;
} xThreadState;
/*-----------------------------------------------------------*/

static xThreadState *pxThreads;
static pthread_once_t hSigSetupThread = PTHREAD_ONCE_INIT;
static pthread_attr_t xThreadAttributes;
static pthread_mutex_t xSuspendResumeThreadMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t xSingleThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xRunningThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xPrintfMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t hMainThread = ( pthread_t )NULL;
static pthread_t hActiveThread = ( pthread_t )NULL;
static pthread_t hRequestedThread = ( pthread_t )NULL;
/*-----------------------------------------------------------*/

static volatile portBASE_TYPE xSentinel = 0;
static volatile portBASE_TYPE xGeneralFuckedUpIndicator = 0;
static volatile portBASE_TYPE xVeryFirstTask = 1;
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
static void prvSetupTimerInterrupt( void );
static void *prvWaitForStart( void * pvParams );
static void prvSuspendSignalHandler(int sig);
//static void prvResumeSignalHandler(int sig);
static void prvSetupSignalsAndSchedulerPolicy( void );
static void prvSuspendThread( pthread_t xThreadId );
static void prvResumeThread( pthread_t xThreadId );
static pthread_t prvGetThreadHandle( xTaskHandle hTask );
static portLONG prvGetFreeThreadState( void );
static void prvSetTaskCriticalNesting( pthread_t xThreadId, unsigned portBASE_TYPE uxNesting );
static unsigned portBASE_TYPE prvGetTaskCriticalNesting( pthread_t xThreadId );
static void prvDeleteThread( void *xThreadId );
static void prvResolveFuckup( void );
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

tskTCB* debug_task_handle;
tskTCB* prvGetTaskHandle( pthread_t hThread )
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


#define debug_printf(...) ( (real_pthread_mutex_lock( &xPrintfMutex )|1)?( \
	(  \
		(NULL != (debug_task_handle = prvGetTaskHandle(pthread_self())) )? \
			(fprintf( stderr, "%s(%li)\t%s\t%i:",debug_task_handle->pcTaskName,(long)pthread_self(),__func__,__LINE__)): \
			(fprintf( stderr, "__unknown__(%li)\t%s\t%i:",(long)pthread_self(),__func__,__LINE__)) \
	|1)?( \
			((fprintf( stderr, __VA_ARGS__ )|1)?real_pthread_mutex_unlock( &xPrintfMutex ):0) \
	):0 ):0 )

//int xbla;
//#define debug_printf(...) xbla=0
int real_pthread_mutex_lock(pthread_mutex_t* mutex) {
	return pthread_mutex_lock(mutex);
}
int real_pthread_mutex_unlock(pthread_mutex_t* mutex) {
	return pthread_mutex_unlock(mutex);
}
#define pthread_mutex_lock(...) ( (debug_printf(" -!- pthread_mutex_lock(%s)\n",#__VA_ARGS__)|1)?pthread_mutex_lock(__VA_ARGS__):0 )
#define pthread_mutex_unlock(...) ( (debug_printf(" -=- pthread_mutex_unlock(%s)\n",#__VA_ARGS__)|1)?pthread_mutex_unlock(__VA_ARGS__):0 )
#define pthread_kill(thread,signal) ( (debug_printf(" sending signal %i to thread %li!\n",(int)signal,(long)thread)|1)?pthread_kill(thread,signal):0 )
#define vTaskSwitchContext() ( (debug_printf("SWITCHCONTEXT!\n")|1)?vTaskSwitchContext():vTaskSwitchContext() )
/*-----------------------------------------------------------*/

void prvSuspendThread( pthread_t xThreadId )
{
portBASE_TYPE xResult;
//	xResult = pthread_mutex_lock( &xSuspendResumeThreadMutex );
//	if ( 0 == xResult )
//	{
		/* Set-up for the Suspend Signal handler? */
		//xSentinel = 0;
//portBASE_TYPE aSentinel=xSentinel;
		xResult = pthread_kill( xThreadId, SIG_SUSPEND );
//		xResult = pthread_mutex_unlock( &xSuspendResumeThreadMutex );
//		while ( ( aSentinel == xSentinel ) && ( pdTRUE != xServicingTick ) )
//		{
//			sched_yield();
//		}
//	}
}
/*-----------------------------------------------------------*/

void prvResumeThread( pthread_t xThreadId )
{
portBASE_TYPE xResult;
//	if ( 0 == pthread_mutex_lock( &xSuspendResumeThreadMutex ) )
//	{
		if ( pthread_self() != xThreadId )
		{
			xResult = pthread_kill( xThreadId, SIG_RESUME );
		}
//		xResult = pthread_mutex_unlock( &xSuspendResumeThreadMutex );
//	}
}
#define prvSuspendThread(thread) debug_printf("calling SuspendThread(%li)\n",(long)thread); prvSuspendThread(thread)
#define prvResumeThread(thread) debug_printf("calling ResumeThread(%li)\n",(long)thread); prvResumeThread(thread)

/*
 * Exception handlers.
 */
void vPortYield( void );
void vPortSystemTickHandler( int sig );

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
void vPortStartFirstTask( void );
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
/* Should actually keep this struct on the stack. */
xParams *pxThisThreadParams = pvPortMalloc( sizeof( xParams ) );

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
	if ( 0 == pthread_mutex_lock( &xSuspendResumeThreadMutex ) ) {
		while (hActiveThread!=hRequestedThread && !xVeryFirstTask) {
			(void)pthread_mutex_unlock( &xSuspendResumeThreadMutex );
			sched_yield();
			(void)pthread_mutex_lock( &xSuspendResumeThreadMutex );
		}


		lIndexOfLastAddedTask = prvGetFreeThreadState();

		/* Create the new pThread. */
		/* On creation of the very first thread, RunningThreadMutex is not claimed yet
		 * by the master thread - do that! */
		if (xVeryFirstTask==1) {
			debug_printf("Seting up very first task (main) - MAIN is ACTIVE TASK\n");
			if (0 == pthread_mutex_lock( &xRunningThreadMutex)) {
				xVeryFirstTask=0;
				hActiveThread=pthread_self();
				hRequestedThread=hActiveThread;
			} else {
				printf("Failed to acquire lock for first task");
				exit(1);
			}

		}

		if ( 0 != pthread_create( &( pxThreads[ lIndexOfLastAddedTask ].hThread ), &xThreadAttributes, prvWaitForStart, (void *)pxThisThreadParams ) )
		{
			/* Thread create failed, signal the failure */
			pxTopOfStack = 0;
		}

		/* Wait until the task suspends. */
		xSentinel=0;
		(void)pthread_mutex_unlock( &xRunningThreadMutex );
		while ( xSentinel == 0 ) {
			sched_yield();
		}
		(void)pthread_mutex_lock( &xRunningThreadMutex );
		hActiveThread=pthread_self();
		debug_printf("ACTIVE THREAD RECLAIMED!\n");
		(void)pthread_mutex_unlock( &xSuspendResumeThreadMutex );
	} else {
		debug_printf("mutex locking failed\n");
		exit(1);
	}
	vPortExitCritical();

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

void vPortStartFirstTask( void )
{
	/* Initialise the critical nesting count ready for the first task. */
	uxCriticalNesting = 0;

	/* Start the first task. */
	vPortEnableInterrupts();

	/* Start the first task. */
	hRequestedThread=prvGetThreadHandle( xTaskGetCurrentTaskHandle());
	prvResumeThread( hRequestedThread );

	sched_yield();
}
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
portBASE_TYPE xPortStartScheduler( void )
{
portBASE_TYPE xResult;
int iSignal;
int fuckedUpCount=0;
sigset_t xSignals;
sigset_t xSignalToBlock;
sigset_t xSignalsBlocked;
portLONG lIndex;

	/* Establish the signals to block before they are needed. */
	sigfillset( &xSignalToBlock );

	/* Block until the end */
	(void)pthread_sigmask( SIG_SETMASK, &xSignalToBlock, &xSignalsBlocked );

	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		pxThreads[ lIndex ].uxCriticalNesting = 0;
	}

	/* Start the timer that generates the tick ISR.  Interrupts are disabled
	here already. */
	prvSetupTimerInterrupt();

	/* Start the first task. Will not return unless all threads are killed. */
	vPortStartFirstTask();

	/* This is the end signal we are looking for. */
	sigemptyset( &xSignals );
	sigaddset( &xSignals, SIG_RESUME );
	sigaddset( &xSignals, SIG_TICK );

	/* Allow other threads to run */
	(void)pthread_mutex_unlock( &xRunningThreadMutex );
	debug_printf( "MAIN thread is entering main signal wait loop!\n");

	while ( pdTRUE != xSchedulerEnd )
	{
		if ( 0 != sigwait( &xSignals, &iSignal ) )
		{
			printf( "Main thread spurious signal: %d\n", iSignal );
		}
		/**
		 * Tick handler is called from here - AND ONLY FROM HERE
		 * (needed for cygwin - but should work on all)
		 */
		if (iSignal==SIG_TICK) {
			if (xGeneralFuckedUpIndicator!=0 && hActiveThread!=hRequestedThread) {
				fuckedUpCount++;
				if (fuckedUpCount>10) {
					fuckedUpCount=0;
					prvResolveFuckup();
				}
			} else {
				fuckedUpCount=0;
			}
			vPortSystemTickHandler(iSignal);
		}
		if (iSignal==SIG_RESUME && pdTRUE != xSchedulerEnd) {
			debug_printf( "ALERT! Main received SIG_RESUME that was supposed to go elsewhere!");
		}
	}

	printf( "Cleaning Up, Exiting.\n" );
	/* Cleanup the mutexes */
	//xResult = pthread_mutex_destroy( &xSuspendResumeThreadMutex );
	//xResult = pthread_mutex_destroy( &xSingleThreadMutex );
	xResult = pthread_mutex_destroy( &xRunningThreadMutex );
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
	 * xSingleThreadMutex is already owned by an original call to Yield. Therefore,
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

	/**
	 * Sentinel - do not change context while the running task is not equal the task supposed to run
	 */
	if ( 0 == pthread_mutex_lock( &xSuspendResumeThreadMutex ) )
	{
		/**
		 * Make sure we don't create outdated resume signals
		 */
		while (hActiveThread!=hRequestedThread) {
			(void)pthread_mutex_unlock( &xSuspendResumeThreadMutex );
			sched_yield();
			(void)pthread_mutex_lock( &xSuspendResumeThreadMutex );
		}
		xTaskToSuspend = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );

		vTaskSwitchContext();

		xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
		if ( xTaskToSuspend != xTaskToResume )
		{
			/* Remember and switch the critical nesting. */
			prvSetTaskCriticalNesting( xTaskToSuspend, uxCriticalNesting );
			uxCriticalNesting = prvGetTaskCriticalNesting( xTaskToResume );
			/* Switch tasks. */
			hRequestedThread = xTaskToResume;	
			prvResumeThread( xTaskToResume );
			//prvSuspendThread( xTaskToSuspend );
if (prvGetThreadHandle(xTaskGetCurrentTaskHandle())!=xTaskToResume) {
	debug_printf("\n     what the fuck???? someone else did a switchcontext?!?\n");
}
			(void)pthread_mutex_unlock( &xSuspendResumeThreadMutex );
			prvSuspendSignalHandler(SIG_SUSPEND);
			return;
		}
		else
		{
			/* Yielding to self */
			(void)pthread_mutex_unlock( &xSuspendResumeThreadMutex );
		}
	}
}
/*-----------------------------------------------------------*/

void vPortDisableInterrupts( void )
{
	xInterruptsEnabled = pdFALSE;
}
/*-----------------------------------------------------------*/

void vPortEnableInterrupts( void )
{
	xInterruptsEnabled = pdTRUE;
}
/*-----------------------------------------------------------*/

portBASE_TYPE xPortSetInterruptMask( void )
{
portBASE_TYPE xReturn = xInterruptsEnabled;
	xInterruptsEnabled = pdFALSE;
	return xReturn;
}
/*-----------------------------------------------------------*/

void vPortClearInterruptMask( portBASE_TYPE xMask )
{
	xInterruptsEnabled = xMask;
}
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void prvSetupTimerInterrupt( void )
{
struct itimerval itimer, oitimer;
portTickType xMicroSeconds = portTICK_RATE_MICROSECONDS;

	debug_printf("init %li microseconds\n",(long)xMicroSeconds);
	/* Initialise the structure with the current timer information. */
	if ( 0 == getitimer( TIMER_TYPE, &itimer ) )
	{
		/* Set the interval between timer events. */
		itimer.it_interval.tv_sec = 0;
		itimer.it_interval.tv_usec = xMicroSeconds;

		/* Set the current count-down. */
		itimer.it_value.tv_sec = 0;
		itimer.it_value.tv_usec = xMicroSeconds;

		/* Set-up the timer interrupt. */
		if ( 0 != setitimer( TIMER_TYPE, &itimer, &oitimer ) )
		{
			printf( "Set Timer problem.\n" );
		}
	}
	else
	{
		printf( "Get Timer problem.\n" );
	}
}
/*-----------------------------------------------------------*/

void vPortSystemTickHandler( int sig )
{
pthread_t xTaskToSuspend;
pthread_t xTaskToResume;
struct timespec timeout;

	//debug_printf("received %i\n",sig);
	/**
	 * do not call tick handler if
	 * - interrupts are disabled
	 * - tick handler is still running
	 * - old task switch not yet completed (wrong task running)
	 */
	if ( ( pdTRUE == xInterruptsEnabled ) && ( pdTRUE != xServicingTick ) && ( hRequestedThread == hActiveThread ) )
	{
		xServicingTick = pdTRUE;
		if ( 0 == pthread_mutex_trylock( &xSuspendResumeThreadMutex ) )
		{
			debug_printf("does handle tick\n");
			
			/**
			 * this shouldn't ever happen - but WELL...
			 * Make sure we don't create outdated resume signals
			 */
			if (hActiveThread!=hRequestedThread) {
				xServicingTick = pdFALSE;
				xPendYield = pdTRUE;
				(void)pthread_mutex_unlock( &xSuspendResumeThreadMutex );
				return;
			}

			xTaskToSuspend = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
			/* Tick Increment. */
			vTaskIncrementTick();

			/* Select Next Task. */
#if ( configUSE_PREEMPTION == 1 )
			vTaskSwitchContext();
#endif
			xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
			//hRequestedThread = xTaskToResume;	

			/* The only thread that can process this tick is the running thread. */
			if ( xTaskToSuspend != xTaskToResume )
			{
				/* Suspend the current task. */
				hRequestedThread = 0;	
				prvSuspendThread( xTaskToSuspend );
				timeout.tv_sec=0;
				timeout.tv_nsec=10000;
				while ( 0 != pthread_mutex_timedlock( &xRunningThreadMutex,&timeout ) ) {
					prvSuspendThread( xTaskToSuspend );
					timeout.tv_sec=0;
					timeout.tv_nsec=10000;
				}
				//if ( 0 == pthread_mutex_lock( &xRunningThreadMutex) ) {
				/* Remember and switch the critical nesting. */
				prvSetTaskCriticalNesting( xTaskToSuspend, uxCriticalNesting );
				uxCriticalNesting = prvGetTaskCriticalNesting( xTaskToResume );
				/* Resume next task. */
				hRequestedThread = xTaskToResume;	
				prvResumeThread( xTaskToResume );
if (prvGetThreadHandle(xTaskGetCurrentTaskHandle())!=xTaskToResume) {
debug_printf("\n     what the fuck???? someone else did a switchcontext?!?\n");
}
				(void)pthread_mutex_unlock( &xRunningThreadMutex );
			}
			else
			{
				/* Release the lock as we are Resuming. */
		//		(void)pthread_mutex_unlock( &xSingleThreadMutex );
			}
		}
		else
		{
			xPendYield = pdTRUE;
		}
		(void)pthread_mutex_unlock( &xSuspendResumeThreadMutex );
		xServicingTick = pdFALSE;
	}
	else
	{
		debug_printf("will NOT handle tick\n");
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

//	if ( 0 == pthread_mutex_lock( &xSingleThreadMutex ) )
//	{
		xTaskToDelete = prvGetThreadHandle( hTaskToDelete );
		xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );

		if ( xTaskToResume == xTaskToDelete )
		{
			if ( 0 == pthread_mutex_lock( &xSuspendResumeThreadMutex ) ) {
				/**
				 * Make sure we don't create outdated resume signals
				 */
				while (hActiveThread!=hRequestedThread) {
					(void)pthread_mutex_unlock( &xSuspendResumeThreadMutex );
					sched_yield();
					(void)pthread_mutex_lock( &xSuspendResumeThreadMutex );
				}
				/* This is a suicidal thread, need to select a different task to run. */
				vTaskSwitchContext();
				xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
				hRequestedThread = xTaskToResume;
				(void)pthread_mutex_unlock( &xSuspendResumeThreadMutex );
			} else {
				debug_printf("mutex lock failed!\n");
				exit(1);
			}
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
//			(void)pthread_mutex_unlock( &xSingleThreadMutex );
		}
		else
		{
			/* Resume the other thread. */
			prvResumeThread( xTaskToResume );
			/* Pthread Clean-up function will note the cancellation. */
			/* Release the execution. */
			uxCriticalNesting = 0;
			vPortEnableInterrupts();
//			(void)pthread_mutex_unlock( &xSingleThreadMutex );
			(void)pthread_mutex_unlock( &xRunningThreadMutex );
			/* Commit suicide */
			pthread_exit( (void *)1 );
		}
//	}
}
/*-----------------------------------------------------------*/

void *prvWaitForStart( void * pvParams )
{
xParams * pxParams = ( xParams * )pvParams;
pdTASK_CODE pvCode = pxParams->pxCode;
void * pParams = pxParams->pvParams;
sigset_t xSignals;
	vPortFree( pvParams );

	pthread_cleanup_push( prvDeleteThread, (void *)pthread_self() );

	if ( 0 == pthread_mutex_lock( &xRunningThreadMutex ) )
	{
		xSentinel=1;
		hActiveThread=pthread_self();
		debug_printf("temporarily made ACTIVE THREAD!\n");

		sigemptyset( &xSignals );
		sigaddset( &xSignals, SIG_RESUME );
		/* set up proc mask */
		pthread_sigmask(SIG_SETMASK,&xSignals,NULL);

		prvSuspendSignalHandler(SIG_SUSPEND);
		//prvSuspendThread( pthread_self() );
	} else {
		debug_printf("now this is just WRONG!\n");
		exit(1);
	}

	pvCode( pParams );

	pthread_cleanup_pop( 1 );
	return (void *)NULL;
}
/*-----------------------------------------------------------*/

void prvSuspendSignalHandler(int sig)
{
sigset_t xSignals;
//sigset_t xPendingSignals;

	/* Only interested in the resume signal. */
	sigemptyset( &xSignals );
	sigaddset( &xSignals, SIG_RESUME );
	sigaddset( &xSignals, SIG_SUSPEND );

	/* Unlock the Running thread mutex to allow the resumed task to continue. */
	if ( 0 != pthread_mutex_unlock( &xRunningThreadMutex ) )
	{
		printf( "Releasing someone else's lock.\n" );
	}

	debug_printf("SUSPENDING until SIG_RESUME received\n");
	/* Wait on the resume signal. */
	while (hRequestedThread != pthread_self()) {
		if ( 0 != sigwait( &xSignals, &sig ) )
		{
			printf( "SSH: Sw %d\n", sig );
			/* tricky one - shouldn't ever happen - trying to resolve situation as graceful as possible */
			debug_printf("ALERT AAAAH PANIC! - sigwait failed.....\n\n\n");
			/* Signal main thread something just went HORRIBLY wrong */
			xGeneralFuckedUpIndicator = 2;
			//(void)pthread_mutex_lock( &xRunningThreadMutex );
			//(void)pthread_kill( pthread_self(), SIG_SUSPEND );
			//return;
		} else if (sig == SIG_RESUME) {
			//debug_printf("received signal %i\n",sig);
			/* Make sure the right thread received the signal */
			if (hRequestedThread != pthread_self() ) {
				debug_printf( "ALERT! Received SIG_RESUME which is already outdated!\n     active thread is %li\n",(long)hRequestedThread);
				/* Signal main thread something just went wrong */
				xGeneralFuckedUpIndicator = 1;
				/*
				if (0 == sigpending(&xPendingSignals)) {
					if (sigismember(&xPendingSignals,SIG_SUSPEND)) {
						debug_printf( "reason: we slept too long...\n");
						//(void)sigwait(&xPendingSignals,&sig);
						// we can safely return - signal is already pending
						return;
					}
				}
				debug_printf( "reason: unknown! - whatever ...\n\n");
				*/
				//exit(1);
				//(void)pthread_kill( xTaskToResume, SIG_RESUME );
				//(void)pthread_mutex_lock( &xRunningThreadMutex );
				//(void)pthread_kill( pthread_self(), SIG_SUSPEND );
				//return;
			}
		}
	}
	/* Yield the Scheduler to ensure that the yielding thread completes. */
	if ( 0 != pthread_mutex_lock( &xRunningThreadMutex ) )
	{
		//(void)pthread_mutex_unlock( &xSingleThreadMutex );
		debug_printf("critical - mutex acquiring of active thread failed!\n");
		exit(1);
	}
	hActiveThread = pthread_self();

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
	if (hRequestedThread==pthread_self()) {
		/* doesn't look too bad, does it? */
		xGeneralFuckedUpIndicator = 0;
	}
	debug_printf("ACTIVE THREAD!\n");
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

struct sigaction sigsuspendself, sigresume, sigtick;
portLONG lIndex;

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

	sigresume.sa_flags = 0;
	//sigresume.sa_handler = prvResumeSignalHandler;
	sigresume.sa_handler = SIG_IGN;
	sigfillset( &sigresume.sa_mask );

	sigtick.sa_flags = 0;
	//sigtick.sa_handler = vPortSystemTickHandler;
	sigtick.sa_handler = SIG_IGN;
	sigfillset( &sigtick.sa_mask );

	if ( 0 != sigaction( SIG_SUSPEND, &sigsuspendself, NULL ) )
	{
		printf( "Problem installing SIG_SUSPEND_SELF\n" );
	}
	//if ( 0 != sigaction( SIG_RESUME, &sigresume, NULL ) )
	//{
	//	printf( "Problem installing SIG_RESUME\n" );
	//}
	if ( 0 != sigaction( SIG_TICK, &sigtick, NULL ) )
	{
		printf( "Problem installing SIG_TICK\n" );
	}
	printf( "Running as PID: %d\n", getpid() );
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
 * Scheduler f***d up - we got to fix that
 */
void prvResolveFuckup( void )
{
struct timespec timeout;
pthread_t xTaskToResume;

	(void)pthread_mutex_lock ( &xSuspendResumeThreadMutex);
	if (hActiveThread == hRequestedThread) {
		debug_printf("emergency handler started - but not needed - returning\n");
		return;
	}
	printf("\nScheduler fucked up again - lets try to fix it...\n");
	if (hRequestedThread==0) {
		printf("\nno supposedly active thread - fixing\n");
		xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );

	} else {
		xTaskToResume = hRequestedThread;
	}
	printf("\nsending sig_suspend to thread that is supposed to be dead...\n");
	prvSuspendThread(hActiveThread);
	printf("\nacquire running lock...\n");
	timeout.tv_sec=0;
	timeout.tv_nsec=10000;
	while ( 0 != pthread_mutex_timedlock( &xRunningThreadMutex,&timeout ) ) {
		prvSuspendThread(hActiveThread);
		timeout.tv_sec=0;
		timeout.tv_nsec=10000;
	}
	printf("\nsending sig_resume to thread that is supposed to be running...\n");
	prvResumeThread(xTaskToResume);
	printf("\ngiving up mutex...\n");
	(void)pthread_mutex_unlock(&xRunningThreadMutex); 
	(void)pthread_mutex_unlock ( &xSuspendResumeThreadMutex);

}
/*-----------------------------------------------------------*/
