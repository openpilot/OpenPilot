/*
    FreeRTOS V7.0.2 - Copyright (C) 2011 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM Cortex-M4F port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* For backward compatibility, ensure configKERNEL_INTERRUPT_PRIORITY is
defined.  The value should also ensure backward compatibility.
FreeRTOS.org versions prior to V4.4.0 did not include this definition. */
#ifndef configKERNEL_INTERRUPT_PRIORITY
	#define configKERNEL_INTERRUPT_PRIORITY 255
#endif

/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL		( ( volatile unsigned long *) 0xe000e010 )
#define portNVIC_SYSTICK_LOAD		( ( volatile unsigned long *) 0xe000e014 )
#define portNVIC_INT_CTRL			( ( volatile unsigned long *) 0xe000ed04 )
#define portNVIC_SYSPRI2			( ( volatile unsigned long *) 0xe000ed20 )
#define portNVIC_SYSTICK_CLK		0x00000004
#define portNVIC_SYSTICK_INT		0x00000002
#define portNVIC_SYSTICK_ENABLE		0x00000001
#define portNVIC_PENDSVSET			0x10000000
#define portNVIC_PENDSV_PRI			( ( ( unsigned long ) configKERNEL_INTERRUPT_PRIORITY ) << 16 )
#define portNVIC_SYSTICK_PRI		( ( ( unsigned long ) configKERNEL_INTERRUPT_PRIORITY ) << 24 )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR			( 0x01000000 )
#define portINITIAL_EXC_RETURN		( 0xfffffffd )	/* return to thread mode, basic stack frame, FPU not used */

/* The priority used by the kernel is assigned to a variable to make access
from inline assembler easier. */
const unsigned long ulKernelPriority = configKERNEL_INTERRUPT_PRIORITY;

/* Each task maintains its own interrupt status in the critical nesting
variable. */
static unsigned portBASE_TYPE uxCriticalNesting = 0xaaaaaaaa;

/*
 * Setup the timer to generate the tick interrupts.
 */
static void prvSetupTimerInterrupt( void );

/*
 * Exception handlers.
 */
void xPortPendSVHandler( void ) __attribute__ (( naked )) __attribute__((no_instrument_function));
void xPortSysTickHandler( void ) __attribute__((no_instrument_function));
void vPortSVCHandler( void ) __attribute__ (( naked )) __attribute__((no_instrument_function));

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
void vPortStartFirstTask( void ) __attribute__ (( naked )) __attribute__((no_instrument_function));

/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, portSTACK_TYPE *pxStartOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
	/*
	 * Create a basic stack frame plus manual save area as would be
	 * saved by xPortPendSVHandler on entry, for a task that has not
	 * used the FPU.
	 *
	 * XXX if pxTopOfStack is not 8-byte aligned and the CPU is configured
	 * to adjust the stack to 8-byte boundaries, is this code safe?
	 */

	/* automatically stacked state */
	*--pxTopOfStack = portINITIAL_XPSR;					/* xPSR */
	*--pxTopOfStack = ( portSTACK_TYPE ) pxCode;		/* pc (thread entrypoint) */
	*--pxTopOfStack = 0;								/* lr */
	*--pxTopOfStack = 0;								/* r12 */
	*--pxTopOfStack = 0;								/* r3 */
	*--pxTopOfStack = 0;								/* r2 */
	*--pxTopOfStack = 0;								/* r1 */
	*--pxTopOfStack = ( portSTACK_TYPE ) pvParameters;	/* r0 (void * passed to thread) */

	/* manually stacked state */
	*--pxTopOfStack = 0;								/* r11 */
	*--pxTopOfStack = ( portSTACK_TYPE ) pxStartOfStack;/* r10 (stack limit) */
	*--pxTopOfStack = 0;								/* r9 */
	*--pxTopOfStack = 0;								/* r8 */
	*--pxTopOfStack = 0;								/* r7 */
	*--pxTopOfStack = 0;								/* r6 */
	*--pxTopOfStack = 0;								/* r5 */
	*--pxTopOfStack = 0;								/* r4 */

	/* exception return code */
	*--pxTopOfStack = portINITIAL_EXC_RETURN;

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

void vPortSVCHandler( void )
{
	/*
	 * Start the first task.
	 */
	__asm volatile (
			"	b context_restore		\n"	/* jump directly to the context restore path in xPortPendSVHandler */
	);
}
/*-----------------------------------------------------------*/

void vPortStartFirstTask( void )
{
	__asm volatile(
			"	ldr r0, =0xE000ED08 	\n" /* Use the NVIC offset register to locate the stack. */
			"	ldr r0, [r0] 			\n"
			"	ldr r0, [r0] 			\n"
			"	msr msp, r0				\n" /* Set the msp back to the start of the stack. */
			"	cpsie i					\n" /* Globally enable interrupts. */
			"	svc 0					\n" /* System call to start first task. */
			"	nop						\n"
	);
}
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
portBASE_TYPE xPortStartScheduler( void )
{
	/* Make PendSV, CallSV and SysTick the same priroity as the kernel. */
	*(portNVIC_SYSPRI2) |= portNVIC_PENDSV_PRI;
	*(portNVIC_SYSPRI2) |= portNVIC_SYSTICK_PRI;

	/* Start the timer that generates the tick ISR.  Interrupts are disabled
	here already. */
	prvSetupTimerInterrupt();

	/* Initialise the critical nesting count ready for the first task. */
	uxCriticalNesting = 0;

	/* Start the first task. */
	vPortStartFirstTask();

	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* It is unlikely that the CM3 port will require this function as there
	is nothing to return to.  */
}
/*-----------------------------------------------------------*/

void vPortYieldFromISR( void )
{
	/* Set a PendSV to request a context switch. */
	*(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
	portDISABLE_INTERRUPTS();
	uxCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
	uxCriticalNesting--;
	if( uxCriticalNesting == 0 )
	{
		portENABLE_INTERRUPTS();
	}
}
/*-----------------------------------------------------------*/

void xPortPendSVHandler( void )
{
	/* This is a naked function. */
	/*
	 * XXX todo - check r0 against r10 for stack overflow detection?
	 */

	/*
	 * Handle a PendSV exception, triggered by vPortYieldFromISR (etc).
	 *
	 * FP state saving is conditional on bit 4 of the EXC_RETURN value (lr).
	 * If the bit is 1, the frame saved by entry is a basic frame and FP
	 * state should not be saved.  If the bit is 0, the frame saved is
	 * an extended frame and the high FP registers also need to be saved.
	 *
	 * Since the EXC_RETURN value is specific to the task, it is the last
	 * item stacked and the first to be popped when restoring a task's
	 * context.
	 */
	__asm volatile
	(
			"	mrs r0, psp							\n"	/* get the program stack pointer (base of automatic frame) in r0 */
			"										\n"
			"	tst lr, 0x10						\n"	/* check for extended stack frame */
			"	bne 1f								\n"	/* skip FP register save if only a basic frame */
			"	vstmdb r0!, {s16-s31}				\n" /* stack the remaining FP registers */
			"1:										\n"
			"	stmdb r0!, {r4-r11}					\n" /* stack the remaining GP registers */
			"	stmdb r0!, {lr}						\n"	/* stack the exception return code for use when resuming */
			"										\n"
			"	ldr	r3, 3f							\n" /* pointer to pointer to current TCB in r3 */
			"	ldr	r2, [r3]						\n"	/* pointer to current TCB in r2 */
			"	str r0, [r2]						\n" /* save stack (context) pointer in the first member of the TCB */
			"										\n"
			"	mov r0, %0							\n"	/* switch to syscall interrupt priority */
			"	msr basepri, r0						\n"
			"										\n"
			"	bl vTaskSwitchContext				\n"	/* handle the context switch call */
			"										\n"
			"context_restore:						\n"	/* branch target for vPortSVCHandler */
			"	mov r0, #0							\n"	/* reset to priority 0 */
			"	msr basepri, r0						\n"
			"										\n"
			"	ldr	r3, 3f							\n" /* pointer to pointer to current TCB in r3 */
			"	ldr r2, [r3]						\n"	/* pointer to current TCB in r2 */
			"	ldr r0, [r2]						\n" /* restore stack (context) pointer from the first member of the TCB */
			"										\n"
			"	ldmia r0!, {lr}						\n"	/* pop the exception return code */
			"	ldmia r0!, {r4-r11}					\n" /* pop the manually-stacked GP registers */
			"	tst lr, 0x10						\n"	/* check for extended stack frame */
			"	bne 2f								\n"	/* skip FP register pop if only a basic frame */
			"	vldmia r0!, {s16-s31}				\n" /* pop the manually-stacked FP registers */
			"2:										\n"
			"	msr psp, r0							\n"	/* reload the program stack pointer */
			"	bx lr								\n"	/* perform an exception return as per the encoding in lr */
			"										\n"
			"	.align 2							\n"
			"3: .word pxCurrentTCB					\n"
			::"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY)
	);
}
/*-----------------------------------------------------------*/

void xPortSysTickHandler( void )
{
unsigned long ulDummy;

	/* If using preemption, also force a context switch. */
	#if configUSE_PREEMPTION == 1
		*(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
	#endif

	ulDummy = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		vTaskIncrementTick();
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( ulDummy );
}
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void prvSetupTimerInterrupt( void )
{
	/* Configure SysTick to interrupt at the requested rate. */
	*(portNVIC_SYSTICK_LOAD) = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
	*(portNVIC_SYSTICK_CTRL) = portNVIC_SYSTICK_CLK | portNVIC_SYSTICK_INT | portNVIC_SYSTICK_ENABLE;
}
/*-----------------------------------------------------------*/

void	__cyg_profile_func_enter(void *func, void *caller) __attribute__((naked, no_instrument_function));
void	__cyg_profile_func_exit(void *func, void *caller)  __attribute__((naked, no_instrument_function));
void	__stack_overflow_trap(void) __attribute__((naked, no_instrument_function));

void
__stack_trap(void)
{
	/* if we get here, the stack has overflowed */
	asm ( "b .");
}

void
__cyg_profile_func_enter(void *func, void *caller)
{
	asm volatile (
			"	mrs r2, ipsr		\n"	/* Check whether we are in interrupt mode */
			"	cmp r2, #0			\n" /* since we don't switch r10 on interrupt entry, we */
			"	bne 2f				\n" /* can't detect overflow of the interrupt stack. */
			"						\n"
			"	sub	r2, sp, #68		\n"	/* compute stack pointer as though we just stacked a full frame */
			"	mrs	r1, control		\n"	/* Test CONTROL.FPCA to see whether we also need room for the FP */
			"	tst r1, #4			\n"	/* context. */
			"	beq 1f				\n"
			"	sub r2, r2, #136	\n"	/* subtract FP context frame size */
			"1:						\n"
			"	cmp r2, r10			\n"	/* compare stack with limit */
			"	bgt	2f				\n"	/* stack is above limit and thus OK */
			"	b __stack_trap		\n"
			"2:						\n"
			"	bx lr				\n"
	);
}

void
__cyg_profile_func_exit(void *func, void *caller)
{
	asm volatile("bx lr");
}


