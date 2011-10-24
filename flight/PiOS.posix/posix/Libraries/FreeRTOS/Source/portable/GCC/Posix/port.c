/*
	Copyright (C) 2011 Corvus Corax from OpenPilot.org
	based on linux port from William Davy

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


/** Description of scheduler:

This scheduler is based on posix signals to halt or preempt tasks, and on
pthread conditions to resume them.

Each FreeRTOS thread is created as a posix thread, with a signal handler to
SIGUSR1 (SIG_SUSPEND) signals.

Suspension of a thread is done by setting the threads state to "PAUSING",
then signaling the thread until the signal handler changes that state to "SLEEPING",
thus acknowledging the suspend.

The thread will then wait within the signal handler for a thread specific
condition to be set, which allows it to resume operation, setting its state to "RUNNING"

The running thread also always holds a mutex (xRunningThreadMutex) which is
given up only when the thread suspends.

On thread creation the new thread will acquire this mutex, then yield.

Both preemption and yielding is done using the same mechanism,
sending a SIG_SUSPEND to the preempted thread respectively to itself. 

Preemption is done by the main scheduler thread which attempts to run a tick
handler at accurate intervals using nanosleep and gettimeofday, which allows
accurate high frequency ticks.

Task switching is protected by xSwitchingThreadMutex which prevents the tick
handler and regular yields from interfering with each other.

This approach is tested and works both on Linux and BSD style Unix (MAC OS X)

*/

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

#define PORT_PRINT(...) fprintf(stderr,__VA_ARGS__)
#define PORT_ASSERT(assertion)    if ( !(assertion) ) { PORT_PRINT("Assertion failed in %s:%i  " #assertion "\n",__FILE__,__LINE__); int assfail=0; assfail=assfail/assfail; }


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
	pthread_mutex_t threadSleepMutex;
	pthread_cond_t threadSleepCond;
    enum {THREAD_SLEEPING,THREAD_RUNNING,THREAD_STARTING,THREAD_PAUSING} threadStatus;
} xThreadState;
/*-----------------------------------------------------------*/

static xThreadState *pxThreads;
static pthread_once_t hSigSetupThread = PTHREAD_ONCE_INIT;
static pthread_attr_t xThreadAttributes;
static pthread_mutex_t xRunningThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xSwitchingThreadMutex = PTHREAD_MUTEX_INITIALIZER;
/*-----------------------------------------------------------*/

static volatile portBASE_TYPE xSchedulerEnd = pdFALSE;
static volatile portBASE_TYPE xInterruptsEnabled = pdTRUE;
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
static void prvSuspendThread( xThreadState* xThreadId );
static void prvResumeThread( xThreadState* xThreadId );
static xThreadState* prvGetThreadHandle( xTaskHandle hTask );
static xThreadState* prvGetThreadHandleByThread( pthread_t hThread );
static portLONG prvGetFreeThreadState( void );
static void prvSetTaskCriticalNesting( xThreadState*  xThreadId, unsigned portBASE_TYPE uxNesting );
static unsigned portBASE_TYPE prvGetTaskCriticalNesting( xThreadState*  xThreadId );
static void prvDeleteThread( void *xThreadId );
static void prvSwitchTasks(void);
/*-----------------------------------------------------------*/

/*
 * Exception handlers.
 */
void vPortYield( void );
void vPortSystemTickHandler( void );

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
void vPortStartFirstTask( void );
/*-----------------------------------------------------------*/

/**
 * Creates a new thread.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
	/* Should actually keep this struct on the stack. */
	xParams *pxThisThreadParams = pvPortMalloc( sizeof( xParams ) );

	/* Initialize scheduler during first call */
	(void)pthread_once( &hSigSetupThread, prvSetupSignalsAndSchedulerPolicy );

	/* No need to join the threads. */
	pthread_attr_init( &xThreadAttributes );
	pthread_attr_setdetachstate( &xThreadAttributes, PTHREAD_CREATE_DETACHED );

	/* Add the task parameters. */
	pxThisThreadParams->pxCode = pxCode;
	pxThisThreadParams->pvParams = pvParameters;

	/* Prevent preemption during task creation */
	vPortEnterCritical();

	lIndexOfLastAddedTask = prvGetFreeThreadState();

	pxThreads[ lIndexOfLastAddedTask ].threadStatus = THREAD_STARTING;
	if ( 0 != pthread_create( &( pxThreads[ lIndexOfLastAddedTask ].hThread ), &xThreadAttributes, prvWaitForStart, (void *)pxThisThreadParams ) )
	{
		/* Thread create failed, signal the failure */
		return 0;
	}

	/* Let the task run a bit and wait until it suspends. */
	PORT_ASSERT( 0 == pthread_mutex_unlock( &xRunningThreadMutex ) );
	while ( pxThreads[ lIndexOfLastAddedTask ].threadStatus == THREAD_STARTING ) sched_yield();
	PORT_ASSERT( 0 == pthread_mutex_lock( &xRunningThreadMutex ) );

	vPortExitCritical();

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

/**
 * Initially the schedulers main thread holds the running thread mutex.
 * it needs to be given up, to allow the first running task to execute
 */
void vPortStartFirstTask( void )
{
	/* Initialise the critical nesting count ready for the first task. */
	uxCriticalNesting = 0;

	/* Start the first task. */
	vPortEnableInterrupts();

	/* Start the first task. */
	prvResumeThread( prvGetThreadHandle( xTaskGetCurrentTaskHandle() ) );

	/* Give up running thread handle */
	PORT_ASSERT( 0 == pthread_mutex_unlock(&xRunningThreadMutex));
}
/*-----------------------------------------------------------*/

/**
 * After tasks have been set up the main thread goes into a sleeping loop, but
 * allows to be interrupted by timer ticks.
 */
portBASE_TYPE xPortStartScheduler( void )
{
portBASE_TYPE xResult;
sigset_t xSignalToBlock;
portLONG lIndex;

	/* just in case*/
	vPortDisableInterrupts();

	/* do not respond to SUSPEND signal (but all others) */
	sigemptyset( &xSignalToBlock );
	(void)pthread_sigmask( SIG_SETMASK, &xSignalToBlock, NULL );
	sigemptyset(&xSignalToBlock);
	sigaddset(&xSignalToBlock,SIG_SUSPEND);
	(void)pthread_sigmask(SIG_BLOCK, &xSignalToBlock, NULL);

	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		pxThreads[ lIndex ].uxCriticalNesting = 0;
	}

	/* Start the first task. This gives up the RunningThreadMutex*/
	vPortStartFirstTask();

	/** Main scheduling loop. Call the tick handler every
	 * portTICK_RATE_MICROSECONDS
	 */
	portLONG sleepTimeUS = portTICK_RATE_MICROSECONDS;
	portLONG actualSleepTime;
	struct timeval lastTime,currentTime;
	gettimeofday( &lastTime, NULL );
	struct timespec wait;
	
	while ( pdTRUE != xSchedulerEnd )
	{
		/* wait for the specified wait time */
		wait.tv_sec = sleepTimeUS / 1000000;
		wait.tv_nsec = 1000 * ( sleepTimeUS % 1000000 );
		nanosleep( &wait, NULL );

		/* check the time */
		gettimeofday( &currentTime, NULL);
		actualSleepTime = 1000000 * ( currentTime.tv_sec - lastTime.tv_sec ) + ( currentTime.tv_usec - lastTime.tv_usec );

		/* only hit the tick if we slept at least half the period */
		if ( actualSleepTime >= sleepTimeUS/2 ) {

			vPortSystemTickHandler();

			/* check the time again */
			gettimeofday( &currentTime, NULL);
			actualSleepTime = 1000000 * ( currentTime.tv_sec - lastTime.tv_sec ) + ( currentTime.tv_usec - lastTime.tv_usec );

			/* sleep until the next tick is due */
			sleepTimeUS += portTICK_RATE_MICROSECONDS;
		}

		/* reduce remaining sleep time by the slept time */
		sleepTimeUS -= actualSleepTime;
		lastTime = currentTime;

		/* safety checks */
		if (sleepTimeUS <=0 || sleepTimeUS >= 3 * portTICK_RATE_MICROSECONDS) sleepTimeUS = portTICK_RATE_MICROSECONDS;

	}

	PORT_PRINT( "Cleaning Up, Exiting.\n" );
	/* Cleanup the mutexes */
	xResult = pthread_mutex_destroy( &xRunningThreadMutex );
	xResult = pthread_mutex_destroy( &xSwitchingThreadMutex );
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
}
/*-----------------------------------------------------------*/

void vPortYieldFromISR( void )
{
	/* yielding from an ISR must return immediately. We cannot wait for the
	 * mutex */
	if( 0 == pthread_mutex_trylock( &xSwitchingThreadMutex ) ) {
		prvSwitchTasks();
	} else {
		xPendYield = pdTRUE;
	}
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
/**
 * The task switching works identically for both preemption and
 * yielding, therefore it sits in this shared function
 */
static void prvSwitchTasks(void)
{
	/**
	 * needs to be called with xSwitchingThreadMutex locked!!!
	 */

	xThreadState *xTaskToSuspend, *xTaskToResume;
	xTaskToSuspend = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );

	vTaskSwitchContext();

	xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
	if ( xTaskToSuspend != xTaskToResume )
	{
		/* Remember and switch the critical nesting. */
		prvSetTaskCriticalNesting( xTaskToSuspend, uxCriticalNesting );
		uxCriticalNesting = prvGetTaskCriticalNesting( xTaskToResume );
		/* Switch tasks. */
		prvResumeThread( xTaskToResume );
		prvSuspendThread( xTaskToSuspend );
	} else {
		(void)pthread_mutex_unlock( &xSwitchingThreadMutex );
	}
}
/*-----------------------------------------------------------*/

void vPortYield( void )
{

	PORT_ASSERT( 0 == pthread_mutex_lock( &xSwitchingThreadMutex ) );

	prvSwitchTasks();

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

void vPortSystemTickHandler()
{
	if ( ( pdTRUE == xInterruptsEnabled ) )
	{
		/* Tick Increment. */
		vTaskIncrementTick();

#if ( configUSE_PREEMPTION == 1 )
		if ( 0 == pthread_mutex_trylock( &xSwitchingThreadMutex ) )
		{
			prvSwitchTasks();
		}
		else
		{
			xPendYield = pdTRUE;
		}
#else
		xPendYield = pdTRUE;
#endif
	}
	else
	{
		xPendYield = pdTRUE;
	}
}
/*-----------------------------------------------------------*/

void vPortForciblyEndThread( void *pxTaskToDelete )
{
xTaskHandle hTaskToDelete = ( xTaskHandle )pxTaskToDelete;
xThreadState* xTaskToDelete;
xThreadState* xTaskToResume;
portBASE_TYPE xResult;

	PORT_ASSERT( 0 == pthread_mutex_lock( &xSwitchingThreadMutex ));

	xTaskToDelete = prvGetThreadHandle( hTaskToDelete );
	xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );

	PORT_ASSERT(  xTaskToDelete );
	PORT_ASSERT(  xTaskToResume );

	if ( xTaskToResume == xTaskToDelete )
	{
		/* This is a suicidal thread, need to select a different task to run. */
		vTaskSwitchContext();
		xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
	}

	if ( pthread_self() != xTaskToDelete->hThread )
	{
		/* Cancelling a thread that is not me. */

		/* Send a signal to wake the task so that it definitely cancels. */
		pthread_testcancel();
		xResult = pthread_cancel( xTaskToDelete->hThread );

		/* Pthread Clean-up function will note the cancellation. */
		(void)pthread_mutex_unlock( &xSwitchingThreadMutex );
	}
	else
	{
		/* Resume the other thread. */
		prvResumeThread( xTaskToResume );
		/* Pthread Clean-up function will note the cancellation. */
		/* Release the execution. */
		uxCriticalNesting = 0;
		vPortEnableInterrupts();
		(void)pthread_mutex_unlock( &xRunningThreadMutex );
		(void)pthread_mutex_unlock( &xSwitchingThreadMutex );
		/* Commit suicide */
		pthread_exit( (void *)1 );
	}
}
/*-----------------------------------------------------------*/

/**
 * any new thread first acquires the runningThreadMutex, but then suspends
 * immediately, giving control back to the thread starting the new one
 */
void *prvWaitForStart( void * pvParams )
{
xParams * pxParams = ( xParams * )pvParams;
pdTASK_CODE pvCode = pxParams->pxCode;
void * pParams = pxParams->pvParams;
sigset_t xSignalToBlock;
	vPortFree( pvParams );

	pthread_cleanup_push( prvDeleteThread, (void *)pthread_self() );

	/* do respond to signals */
	sigemptyset( &xSignalToBlock );
	(void)pthread_sigmask( SIG_SETMASK, &xSignalToBlock, NULL );

	/* acquire mutexes */
	PORT_ASSERT( 0 == pthread_mutex_lock( &xRunningThreadMutex ) );
	PORT_ASSERT( 0 == pthread_mutex_lock( &xSwitchingThreadMutex ) );
		
	/* suspend until further notice */
	prvSuspendThread( prvGetThreadHandleByThread(pthread_self()) );

	/* run the actual task */
	pvCode( pParams );

	pthread_cleanup_pop( 1 );
	return (void *)NULL;
}
/*-----------------------------------------------------------*/

void prvSuspendSignalHandler(int sig)
{
	xThreadState* myself = prvGetThreadHandleByThread(pthread_self());

	PORT_ASSERT( myself );

	if (myself->threadStatus != THREAD_PAUSING) {
		/* Spurious signal has arrived, we are not really supposed to halt.
		 * Not a real problem, we just ignore that. */
		return;
	}

	/* go to sleep */
	PORT_ASSERT( 0 == pthread_mutex_lock( &myself->threadSleepMutex ) );
	myself->threadStatus = THREAD_SLEEPING;
	PORT_ASSERT( 0 == pthread_mutex_unlock( &xRunningThreadMutex ) );

	/* sleep */
	do {
		pthread_cond_wait( &myself->threadSleepCond, &myself->threadSleepMutex );
	} while (prvGetThreadHandle( xTaskGetCurrentTaskHandle())!=myself);

	/* wake up */
	PORT_ASSERT( 0 == pthread_mutex_lock( &xRunningThreadMutex ) );
	myself->threadStatus = THREAD_RUNNING;
	PORT_ASSERT( 0 == pthread_mutex_unlock( &myself->threadSleepMutex ) );
	/* wake up */

	/* Will resume here when the RESUME signal is received. */
	/* Need to set the interrupts based on the task's critical nesting. */
	if ( uxCriticalNesting == 0 )
	{
		vPortEnableInterrupts();
	}
	else
	{
		vPortDisableInterrupts();
	}
}
/*-----------------------------------------------------------*/

void prvSuspendThread( xThreadState* xThreadId )
{
	/**
	 * needs to be called with xSwitchingThreadMutex locked!!!
	 */
	PORT_ASSERT(xThreadId);
	PORT_ASSERT(xThreadId->threadStatus != THREAD_PAUSING);
	xThreadId->threadStatus = THREAD_PAUSING;
	/* It would be optimal if we could keep the mutex until after the suspend
	 * has worked however if we suspend ourselfes (yield) that would cause a
	 * deadlock since we cannot give back the mutex when sleeping
	 */
	(void)pthread_mutex_unlock( &xSwitchingThreadMutex );
	/**
	 * Send signals until the signal handler acknowledges.  how long that takes
	 * depends on the systems signal implementation.  During a preemption we
	 * will see the actual THREAD_SLEEPING STATE when yielding we would only
	 * see a future THREAD_RUNNING after having woken up both is OK
	 */
	while ( xThreadId->threadStatus == THREAD_PAUSING ) {
		pthread_kill( xThreadId->hThread, SIG_SUSPEND );
		sched_yield();
	}

}
/*-----------------------------------------------------------*/

/**
 * Signal the condition.
 * Unlike pthread_kill this actually is supposed to be reliable, so we need no
 * checks on the outcome.
 */
void prvResumeThread( xThreadState* xThreadId )
{
	PORT_ASSERT(xThreadId);
	pthread_cond_signal(& xThreadId->threadSleepCond);
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

	pxThreads = ( xThreadState *)pvPortMalloc( sizeof( xThreadState ) * MAX_NUMBER_OF_TASKS );
	
	const pthread_cond_t cinit = PTHREAD_COND_INITIALIZER;
	const pthread_mutex_t minit = PTHREAD_MUTEX_INITIALIZER;
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		pxThreads[ lIndex ].hThread = ( pthread_t )NULL;
		pxThreads[ lIndex ].hTask = ( xTaskHandle )NULL;
		pxThreads[ lIndex ].uxCriticalNesting = 0;
		pxThreads[ lIndex ].threadSleepMutex = minit;
		pxThreads[ lIndex ].threadSleepCond = cinit;
	}

	sigsuspendself.sa_flags = 0;
	sigsuspendself.sa_handler = prvSuspendSignalHandler;
	sigfillset( &sigsuspendself.sa_mask );

	if ( 0 != sigaction( SIG_SUSPEND, &sigsuspendself, NULL ) )
	{
		PORT_PRINT( "Problem installing SIG_SUSPEND_SELF\n" );
	}
	PORT_PRINT( "Running as PID: %d\n", getpid() );

	/* When scheduler is set up main thread first claims the running thread mutex */
	PORT_ASSERT( 0 == pthread_mutex_lock( &xRunningThreadMutex ) );

}
/*-----------------------------------------------------------*/

xThreadState* prvGetThreadHandle( xTaskHandle hTask )
{
portLONG lIndex;
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hTask == hTask )
		{
			return &pxThreads[ lIndex ];
			break;
		}
	}
	return NULL;
}
/*-----------------------------------------------------------*/

xThreadState* prvGetThreadHandleByThread( pthread_t hThread )
{
portLONG lIndex;
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == hThread )
		{
			return &pxThreads[ lIndex ];
			break;
		}
	}
	return NULL;
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
		PORT_PRINT( "No more free threads, please increase the maximum.\n" );
		lIndex = 0;
		vPortEndScheduler();
	}

	return lIndex;
}
/*-----------------------------------------------------------*/

void prvSetTaskCriticalNesting( xThreadState* xThreadId, unsigned portBASE_TYPE uxNesting )
{
	PORT_ASSERT(xThreadId);
	xThreadId->uxCriticalNesting = uxNesting;
}
/*-----------------------------------------------------------*/

unsigned portBASE_TYPE prvGetTaskCriticalNesting( xThreadState* xThreadId )
{

	PORT_ASSERT(xThreadId);
	return xThreadId->uxCriticalNesting;
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
	PORT_PRINT( "Timer Resolution for Run TimeStats is %ld ticks per second.\n", ulTicksPerSecond );
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
