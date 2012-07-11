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

Suspension of a thread is done by setting the threads state to
"YIELDING/PREEMPTING", then signaling the thread until the signal handler
changes that state to "SLEEPING", thus acknowledging the suspend.

The thread will then wait within the signal handler for a thread specific
condition to be set, which allows it to resume operation, setting its state to
"RUNNING"

The running thread also always holds a mutex (xRunningThreadMutex) which is
given up only when the thread suspends.

On thread creation the new thread will acquire this mutex, then yield.

Both preemption and yielding is done using the same mechanism, sending a
SIG_SUSPEND to the preempted thread respectively to itself, however different
synchronization safeguards apply depending if a thread suspends itself or is
suspended remotely

Preemption is done by the main scheduler thread which attempts to run a tick
handler at accurate intervals using nanosleep and gettimeofday, which allows
more accurate high frequency ticks than a timer signal handler.

All public functions in this port are protected by a safeguard mutex which
assures priority access on all data objects

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
#define PORT_ASSERT(assertion)    if ( !(assertion) ) { PORT_PRINT("Assertion failed in %s:%i  " #assertion "\n",__FILE__,__LINE__); int volatile assfail=0; assfail=assfail/assfail; }


#define PORT_LOCK(mutex) PORT_ASSERT( 0 == pthread_mutex_lock(&(mutex)) )
#define PORT_TRYLOCK(mutex) pthread_mutex_trylock(&(mutex))
#define PORT_UNLOCK(mutex) PORT_ASSERT( 0 == pthread_mutex_unlock(&(mutex)) )


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
    volatile enum {THREAD_SLEEPING,THREAD_RUNNING,THREAD_STARTING,THREAD_YIELDING,THREAD_PREEMPTING,THREAD_WAKING} threadStatus;
} xThreadState;
/*-----------------------------------------------------------*/

/* Needed to keep track of critical section depth before scheduler got started */
static xThreadState xDummyThread = { .uxCriticalNesting=0, .threadStatus=THREAD_RUNNING };

static xThreadState *pxThreads = NULL;
static pthread_once_t hSigSetupThread = PTHREAD_ONCE_INIT;
static pthread_attr_t xThreadAttributes;
static pthread_mutex_t xRunningThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xYieldingThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xResumingThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xGuardMutex = PTHREAD_MUTEX_INITIALIZER;
/*-----------------------------------------------------------*/

static volatile portBASE_TYPE xSchedulerEnd = pdFALSE;
static volatile portBASE_TYPE xSchedulerStarted = pdFALSE;
static volatile portBASE_TYPE xInterruptsEnabled = pdFALSE;
static volatile portBASE_TYPE xSchedulerNesting = 0;
static volatile portBASE_TYPE xPendYield = pdFALSE;
static volatile portLONG lIndexOfLastAddedTask = 0;
/*-----------------------------------------------------------*/

/*
 * Setup the timer to generate the tick interrupts.
 */
static void *prvWaitForStart( void * pvParams );
static void prvSuspendSignalHandler(int sig);
static void prvSetupSignalsAndSchedulerPolicy( void );
static void prvResumeThread( xThreadState* xThreadId );
static xThreadState* prvGetThreadHandle( xTaskHandle hTask );
static xThreadState* prvGetThreadHandleByThread( pthread_t hThread );
static portLONG prvGetFreeThreadState( void );
static void prvDeleteThread( void *xThreadId );
static void prvPortYield();
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
 * inline macro functions
 * (easierto debug than macros)
 */
static inline void PORT_ENTER() {
	while( prvGetThreadHandleByThread(pthread_self())->threadStatus!=THREAD_RUNNING) sched_yield();
	PORT_LOCK( xGuardMutex );
	PORT_ASSERT( xSchedulerStarted?( prvGetThreadHandleByThread(pthread_self())==prvGetThreadHandle(xTaskGetCurrentTaskHandle()) ):pdTRUE )
}

static inline void PORT_LEAVE() {
	PORT_ASSERT( xSchedulerStarted?( prvGetThreadHandleByThread(pthread_self())==prvGetThreadHandle(xTaskGetCurrentTaskHandle()) ):pdTRUE );
	PORT_ASSERT( prvGetThreadHandleByThread(pthread_self())->threadStatus==THREAD_RUNNING );
	PORT_UNLOCK( xGuardMutex );
}

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

	/**
	 * port enter needs to be delayed since pvPortMalloc() and
	 * SetupSignalsAndSchedulerPolicy() both call critical section code
	 */
	PORT_ENTER();

	/* No need to join the threads. */
	pthread_attr_init( &xThreadAttributes );
	pthread_attr_setdetachstate( &xThreadAttributes, PTHREAD_CREATE_DETACHED );

	/* Add the task parameters. */
	pxThisThreadParams->pxCode = pxCode;
	pxThisThreadParams->pvParams = pvParameters;

	lIndexOfLastAddedTask = prvGetFreeThreadState();
	
	PORT_LOCK( xYieldingThreadMutex );

	pxThreads[ lIndexOfLastAddedTask ].threadStatus = THREAD_STARTING;
	pxThreads[ lIndexOfLastAddedTask ].uxCriticalNesting = 0;

	/* create the thead */
	PORT_ASSERT( 0 == pthread_create( &( pxThreads[ lIndexOfLastAddedTask ].hThread ), &xThreadAttributes, prvWaitForStart, (void *)pxThisThreadParams ) );

	/* Let the task run a bit and wait until it suspends. */
	while ( pxThreads[ lIndexOfLastAddedTask ].threadStatus == THREAD_STARTING ) sched_yield();

	/* this ensures the sleeping thread reached deep sleep (and not more) */
	PORT_UNLOCK( xYieldingThreadMutex );

	PORT_LEAVE();

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

/**
 * Initially the schedulers main thread holds the running thread mutex.
 * it needs to be given up, to allow the first running task to execute
 */
void vPortStartFirstTask( void )
{
	/* Mark scheduler as started */
	xSchedulerStarted = pdTRUE;

	/* Start the first task. */
	prvResumeThread( prvGetThreadHandle( xTaskGetCurrentTaskHandle() ) );

	/* Give up running thread handle */
	PORT_UNLOCK(xRunningThreadMutex);
}
/*-----------------------------------------------------------*/

/**
 * After tasks have been set up the main thread goes into a sleeping loop, but
 * allows to be interrupted by timer ticks.
 */
portBASE_TYPE xPortStartScheduler( void )
{
	sigset_t xSignalToBlock;

	/**
	 * note: NO PORT_ENTER ! - this is the supervisor thread which runs outside
	 * of the schedulers context
	 */

	/* do not respond to SUSPEND signal (but all others) */
	sigemptyset( &xSignalToBlock );
	(void)pthread_sigmask( SIG_SETMASK, &xSignalToBlock, NULL );
	sigemptyset(&xSignalToBlock);
	sigaddset(&xSignalToBlock,SIG_SUSPEND);
	(void)pthread_sigmask(SIG_BLOCK, &xSignalToBlock, NULL);

	/* Start the first task. This gives up the RunningThreadMutex*/
	vPortStartFirstTask();

	/**
	 * Main scheduling loop. Call the tick handler every
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
	pthread_mutex_destroy( &xRunningThreadMutex );
	pthread_mutex_destroy( &xYieldingThreadMutex );
	pthread_mutex_destroy( &xGuardMutex );
	vPortFree( (void *)pxThreads );

	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

/**
 * quickly clean up all running threads, without asking them first
 */
void vPortEndScheduler( void )
{
portBASE_TYPE xNumberOfThreads;
	for ( xNumberOfThreads = 0; xNumberOfThreads < MAX_NUMBER_OF_TASKS; xNumberOfThreads++ )
	{
		if ( ( pthread_t )NULL != pxThreads[ xNumberOfThreads ].hThread )
		{
			/* Kill all of the threads, they are in the detached state. */
			pthread_cancel( pxThreads[ xNumberOfThreads ].hThread );
		}
	}

	/* Signal the scheduler to exit its loop. */
	xSchedulerEnd = pdTRUE;
}
/*-----------------------------------------------------------*/

/**
 * we must assume this one is called from outside the schedulers context
 * (ISR's, signal handlers, or non-freertos threads)
 * we cannot safely assume mutual exclusion
 */
void vPortYieldFromISR( void )
{
	xPendYield = pdTRUE;
}
/*-----------------------------------------------------------*/

/**
 * enter a critical section (public)
 */
void vPortEnterCritical( void )
{
	PORT_ENTER();
	xInterruptsEnabled = pdFALSE;
	prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting++;
	PORT_LEAVE();
}
/*-----------------------------------------------------------*/

/**
 * leave a critical section (public)
 */
void vPortExitCritical( void )
{
	PORT_ENTER();

	/* Check for unmatched exits. */
	PORT_ASSERT( prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting > 0 );
	if ( prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting > 0 )
	{
		prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting--;
	}

	/* If we have reached 0 then re-enable the interrupts. */
	if( prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting == 0 )
	{
		/* Have we missed ticks? This is the equivalent of pending an interrupt. */
		if ( pdTRUE == xPendYield )
		{
			xPendYield = pdFALSE;
			prvPortYield();
		}

		xInterruptsEnabled = pdTRUE;

	}

	PORT_LEAVE();
}
/*-----------------------------------------------------------*/

/**
 * code to self-yield a task
 * (without the mutex encapsulation)
 * for internal use
 */
void prvPortYield()
{
	xThreadState *xTaskToSuspend, *xTaskToResume;

	/* timer handler should NOT get in our way (just in case) */
	xInterruptsEnabled = pdFALSE;

	/* suspend the current task */
	xTaskToSuspend = prvGetThreadHandleByThread( pthread_self() );

	/**
	 * make sure not to suspend threads that are already trying to do so
	 */
	PORT_ASSERT( xTaskToSuspend->threadStatus == THREAD_RUNNING );

	/**
	 * FreeRTOS switch context
	 */
	vTaskSwitchContext();

	/**
	 * find out which task to resume
	 */
	xTaskToResume = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
	if ( xTaskToSuspend != xTaskToResume )
	{
		/* Resume the other thread first */
		prvResumeThread( xTaskToResume );

		/* prepare the current task for yielding */
		xTaskToSuspend->threadStatus = THREAD_YIELDING;

		/**
		 * Send signals until the signal handler acknowledges.  How long that takes
		 * depends on the systems signal implementation.  During a preemption we
		 * will see the actual THREAD_SLEEPING STATE - but when yielding we
		 * would only see a future THREAD_RUNNING after having woken up - both is
		 * OK
		 */
		while ( xTaskToSuspend->threadStatus == THREAD_YIELDING ) {
			pthread_kill( xTaskToSuspend->hThread, SIG_SUSPEND );
			sched_yield();
		}

		/**
		 * mark: once we reach this point, the task has already slept and awaken anew
		 */

	} else {
		/**
		 * no context switch - keep running
		 */
		if (xTaskToResume->uxCriticalNesting==0) {
			xInterruptsEnabled = pdTRUE;
		}
	}

}
/*-----------------------------------------------------------*/

/**
 * public yield function - secure
 */
void vPortYield( void )
{
	PORT_ENTER();

	prvPortYield();

	PORT_LEAVE();
}
/*-----------------------------------------------------------*/

/**
 * public function to disable interrupts
 */
void vPortDisableInterrupts( void )
{
	PORT_ENTER();

	xInterruptsEnabled = pdFALSE;

	PORT_LEAVE();
}
/*-----------------------------------------------------------*/

/**
 * public function to enable interrupts
 */
void vPortEnableInterrupts( void )
{
	PORT_ENTER();

	/**
	 * It is bad practice to enable interrupts explicitly while in a critical section
	 * most likely this is a bug - better prevent the userspace from being stupid
	 */
	PORT_ASSERT( prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting == 0 );

	xInterruptsEnabled = pdTRUE;

	PORT_LEAVE();
}
/*-----------------------------------------------------------*/

/**
 * set and clear interrupt masks are used by FreeRTOS to enter and leave critical sections
 * with unknown nexting level - but we DO know the nesting level
 */
portBASE_TYPE xPortSetInterruptMask( void )
{
	portBASE_TYPE xReturn;

	PORT_ENTER();

	xReturn = xInterruptsEnabled;

	xInterruptsEnabled = pdFALSE;
	
	prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting++;

	PORT_LEAVE();

	return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * sets the "interrupt mask back to a stored setting
 */
void vPortClearInterruptMask( portBASE_TYPE xMask )
{
	PORT_ENTER();

	/**
	 * we better make sure the calling code behaves
	 * if it doesn't it might indicate something went seriously wrong
	 */
	PORT_ASSERT( xMask == pdTRUE || xMask == pdFALSE );
	PORT_ASSERT(
		( prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting == 1 && xMask==pdTRUE )
		|| 
		( prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting > 1 && xMask==pdFALSE )
	);

	xInterruptsEnabled = xMask;
	if (prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting>0) {
		prvGetThreadHandleByThread(pthread_self())->uxCriticalNesting--;
	}

	PORT_LEAVE();
}

/*-----------------------------------------------------------*/

/**
 * the tick handler is just an ordinary function, called by the supervisor thread periodically
 */
void vPortSystemTickHandler()
{
	/**
	 * the problem with the tick handler is, that it runs outside of the schedulers domain - worse,
	 * on a multi core machine there might be a task running *right now*
	 * - we need to stop it in order to do anything. However we need to make sure we are able to first
	 */
	PORT_LOCK( xGuardMutex );

	/* thread MUST be running */
	if ( prvGetThreadHandle(xTaskGetCurrentTaskHandle())->threadStatus!=THREAD_RUNNING ) {
		xPendYield = pdTRUE;
		PORT_UNLOCK( xGuardMutex );
		return;
	}

	/* interrupts MUST be enabled */
	if ( xInterruptsEnabled != pdTRUE ) {
		xPendYield = pdTRUE;
		PORT_UNLOCK( xGuardMutex );
		return;
	}

	/* this should always be true, but it can't harm to check */
	PORT_ASSERT( prvGetThreadHandle(xTaskGetCurrentTaskHandle())->uxCriticalNesting==0 );

	/* acquire switching mutex for synchronization */
	PORT_LOCK(xYieldingThreadMutex);

	xThreadState *xTaskToSuspend = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );


	/**
	 * halt current task - this means NO task is running!
	 * Send signals until the signal handler acknowledges.  how long that takes
	 * depends on the systems signal implementation.  During a preemption we
	 * will see the actual THREAD_SLEEPING STATE when yielding we would only
	 * see a future THREAD_RUNNING after having woken up both is OK
	 * note: we do NOT give up switchingThreadMutex!
	 */
	xTaskToSuspend->threadStatus = THREAD_PREEMPTING;
	while ( xTaskToSuspend->threadStatus != THREAD_SLEEPING ) {
		pthread_kill( xTaskToSuspend->hThread, SIG_SUSPEND );
		sched_yield();
	}

	/**
	 * synchronize and acquire the running thread mutex
	 */
	PORT_UNLOCK( xYieldingThreadMutex );
	PORT_LOCK( xRunningThreadMutex );

	/**
	 * now the tick handler runs INSTEAD of the currently active thread
	 * - even on a multicore system
	 * failure to do so can lead to unexpected results during
	 * vTaskIncrementTick()...
	 */

	/**
	 * call tick handler
	 */
	vTaskIncrementTick();

	
#if ( configUSE_PREEMPTION == 1 )
	/**
	 * while we are here we can as well switch the running thread
	 */
	vTaskSwitchContext();

	xTaskToSuspend = prvGetThreadHandle( xTaskGetCurrentTaskHandle() );
#endif

	/**
	 * wake up the task (again)
	 */
	prvResumeThread( xTaskToSuspend );

	/**
	 * give control to the userspace task
	 */
	PORT_UNLOCK( xRunningThreadMutex );

	/* finish up */
	PORT_UNLOCK( xGuardMutex );
}
/*-----------------------------------------------------------*/

/**
 * thread kill implementation
 */
void vPortForciblyEndThread( void *pxTaskToDelete )
{
xTaskHandle hTaskToDelete = ( xTaskHandle )pxTaskToDelete;
xThreadState* xTaskToDelete;
xThreadState* xTaskToResume;

	PORT_ENTER();

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
		pthread_cancel( xTaskToDelete->hThread );

	}
	else
	{
		/* Resume the other thread. */
		prvResumeThread( xTaskToResume );
		/* Pthread Clean-up function will note the cancellation. */
		/* Release the execution. */

		PORT_UNLOCK( xRunningThreadMutex );

		//PORT_LEAVE();
		PORT_UNLOCK( xGuardMutex );
		/* Commit suicide */
		pthread_exit( (void *)1 );
	}
	PORT_LEAVE();
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
	xThreadState * myself = prvGetThreadHandleByThread( pthread_self() );

	pthread_cleanup_push( prvDeleteThread, (void *)pthread_self() );

	/* do respond to signals */
	sigemptyset( &xSignalToBlock );
	(void)pthread_sigmask( SIG_SETMASK, &xSignalToBlock, NULL );

	/**
	 * Suspend ourselves. It's important to do that first
	 * because until we come back from this we run outside the schedulers scope
	 * and can't call functions like vPortFree() safely
	 */
	while ( myself->threadStatus == THREAD_STARTING ) {
		pthread_kill( myself->hThread, SIG_SUSPEND );
		sched_yield();
	}

	/**
	 * now we have returned from the dead - reborn as a real thread inside the
	 * schedulers scope.
	 */
	vPortFree( pvParams );

	/* run the actual payload */
	pvCode( pParams );

	pthread_cleanup_pop( 1 );
	return (void *)NULL;
}
/*-----------------------------------------------------------*/

/**
 * The suspend signal handler is called when a thread gets a SIGSUSPEND
 * signal, which is supposed to send it to sleep
 */
void prvSuspendSignalHandler(int sig)
{

	portBASE_TYPE hangover;

	/* make sure who we are */
	xThreadState* myself = prvGetThreadHandleByThread(pthread_self());
	PORT_ASSERT( myself );

	/* make sure we are actually supposed to sleep */
	if (myself->threadStatus != THREAD_YIELDING && myself->threadStatus != THREAD_STARTING && myself->threadStatus != THREAD_PREEMPTING ) {
		/* Spurious signal has arrived, we are not really supposed to halt.
		 * Not a real problem, we can safely ignore that. */
		return;
	}

	/* we need that to wake up later (cond_wait needs a mutex locked) */
	PORT_LOCK(myself->threadSleepMutex);

	/* even waking up is a bit different depending on how we went to sleep */
	hangover = myself->threadStatus;

	myself->threadStatus = THREAD_SLEEPING;

	if ( hangover == THREAD_STARTING ) {
		/**
		 * Synchronization with spawning thread through YieldingMutex
		 * This thread does NOT have the running thread mutex
		 * because it never officially ran before.
		 * It will get that mutex on wakeup though.
		 */
		PORT_LOCK(xYieldingThreadMutex);
		PORT_UNLOCK(xYieldingThreadMutex);
	} else if ( hangover == THREAD_YIELDING) {
		/**
		 * The caller is the same thread as the signal handler.
		 * No synchronization possible or needed.
		 * But we need to unlock the mutexes it holds, so
		 * other threads can run.
		 */
		PORT_UNLOCK(xRunningThreadMutex );
		PORT_UNLOCK(xGuardMutex );
	} else if ( hangover == THREAD_PREEMPTING) {
		/**
		 * The caller is the tick handler.
		 * Use YieldingMutex for synchronization
		 * Give up RunningThreadMutex, so the tick handler
		 * can take it and start another thread.
		 */
		PORT_LOCK(xYieldingThreadMutex);
		PORT_UNLOCK(xRunningThreadMutex );
		PORT_UNLOCK(xYieldingThreadMutex);
	}

	/* deep sleep until wake condition is met*/
	pthread_cond_wait( &myself->threadSleepCond, &myself->threadSleepMutex );

	/* waking */
	myself->threadStatus = THREAD_WAKING;
	
	/* synchronize with waker - quick assertion if the right thread got the condition sent to*/
	PORT_LOCK(xResumingThreadMutex);
	PORT_ASSERT(prvGetThreadHandle( xTaskGetCurrentTaskHandle())==myself);
	PORT_UNLOCK(xResumingThreadMutex);

	/* we don't need that condition mutex anymore */
	PORT_UNLOCK(myself->threadSleepMutex);

	/* we ARE the running thread now (the one and only) */
	PORT_LOCK(xRunningThreadMutex);

	/**
	 * and we have important stuff to do, nobody should interfere with
	 * ( GuardMutex is usually set by PORT_ENTER() )
	 * */
	PORT_LOCK( xGuardMutex );
	if ( myself->uxCriticalNesting == 0 )
	{
		xInterruptsEnabled = pdTRUE;
	}
	else
	{
		xInterruptsEnabled = pdFALSE;
	}

	myself->threadStatus = THREAD_RUNNING;

	/**
	 * if we jump back to user code, we are done with important stuff,
	 * but if we had yielded we are still in protected code after returning.
	 **/
	if (hangover!=THREAD_YIELDING) {
		PORT_UNLOCK( xGuardMutex );
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
	PORT_ASSERT( xThreadId );

	PORT_LOCK( xResumingThreadMutex );

	PORT_ASSERT(xThreadId->threadStatus == THREAD_SLEEPING);

	/**
	 * Unfortunately "is supposed to" does not hold on all Posix-ish systems
	 * but sending the cond_signal again doesn't hurt anyone.
	 */
	while ( xThreadId->threadStatus != THREAD_WAKING ) {
		pthread_cond_signal(& xThreadId->threadSleepCond);
		sched_yield();
	}
	
	PORT_UNLOCK( xResumingThreadMutex );

}
/*-----------------------------------------------------------*/

/**
 * this is init code executed the first time a thread is created
 */
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
	PORT_LOCK( xRunningThreadMutex );

}
/*-----------------------------------------------------------*/

/**
 * get a thread handle based on a task handle
 */
xThreadState* prvGetThreadHandle( xTaskHandle hTask )
{
portLONG lIndex;
if (!pxThreads) return NULL;
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

/**
 * get a thread handle based on a posix thread handle
 */
xThreadState* prvGetThreadHandleByThread( pthread_t hThread )
{
portLONG lIndex;
	/**
	 * if the scheduler is NOT yet started, we can give back a dummy thread handle
	 * to allow keeping track of interrupt nesting.
	 * However once the scheduler is started we return a NULL,
	 * so any misbehaving code can nicely segfault.
	 */
	if (!xSchedulerStarted && !pxThreads) return &xDummyThread;
	if (!pxThreads) return NULL;
	for ( lIndex = 0; lIndex < MAX_NUMBER_OF_TASKS; lIndex++ )
	{
		if ( pxThreads[ lIndex ].hThread == hThread )
		{
			return &pxThreads[ lIndex ];
		}
	}
	if (!xSchedulerStarted) return &xDummyThread;
	return NULL;
}
/*-----------------------------------------------------------*/

/* next free task handle */
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

/**
 * delete a thread from the list
 */
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
				//vPortEnableInterrupts();
				xInterruptsEnabled = pdTRUE;
			}
			pxThreads[ lIndex ].uxCriticalNesting = 0;
			break;
		}
	}
}
/*-----------------------------------------------------------*/

/**
 * add a thread to the list
 */
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

/**
 * find out system speed
 */
void vPortFindTicksPerSecond( void )
{
	/* Needs to be reasonably high for accuracy. */
	unsigned long ulTicksPerSecond = sysconf(_SC_CLK_TCK);
	PORT_PRINT( "Timer Resolution for Run TimeStats is %ld ticks per second.\n", ulTicksPerSecond );
}
/*-----------------------------------------------------------*/

/**
 * timer stuff
 */
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

