/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 *
 * @file       startup.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      C based startup of F4
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <string.h>
#include <stm32f4xx.h>

/* prototype for main() that tells us not to worry about it possibly returning */
extern int		main(void) __attribute__((noreturn));

/* prototype our _main() to avoid prolog/epilog insertion and other related junk */
void 			_main(void) __attribute__((noreturn, naked, no_instrument_function));

/** default handler for CPU exceptions */
static void		default_cpu_handler(void) __attribute__((noreturn, naked, no_instrument_function));

/** BSS symbols XXX should have a header that defines all of these */
extern char		_sbss, _ebss;

/** DATA symbols XXX should have a header that defines all of these */
extern char		_sidata, _sdata, _edata, _sfast, _efast;

/** The bootstrap/IRQ stack XXX should define size somewhere else */
char			irq_stack[1024] __attribute__((section(".irqstack")));

/** exception handler */
typedef const void	(vector)(void);

/** CortexM3 CPU vectors */
struct cm3_vectors {
	void	*initial_stack;
	vector	*entry;
	vector	*vectors[14];
};

/**
 * Initial startup code.
 */
void
_main(void)
{
	// load the stack base for the current stack before we attempt to branch to any function
	// that might bounds-check the stack
	asm volatile ("mov r10, %0" : : "r" (&irq_stack[0]) : );

	/* enable usage, bus and memory faults */
	SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk;

	/* configure FP state save behaviour - automatic, lazy save */
	FPU->FPCCR |= FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk;

	/* configure default FPU state */
	FPU->FPDSCR |= FPU_FPDSCR_DN_Msk;	/* enable Default NaN */

	/* enable the FPU */
	SCB->CPACR |= (0xf << 20);	// turn on CP10/11 for FP support on cores that implement it

	/* copy initialised data from flash to RAM */
	memcpy(&_sdata, &_sidata, &_edata - &_sdata);

	/* zero the BSS */
	memset(&_sbss, 0, &_ebss - &_sbss);

	/* zero any 'fast' RAM that's been used */
	memset(&_sfast, 0, &_efast - &_sfast);

	/* fill most of the IRQ/bootstrap stack with a watermark pattern so we can measure how much is used */
	/* leave a little space at the top in case memset() isn't a leaf with no locals */
	memset(&irq_stack, 0xa5, sizeof(irq_stack) - 64);

	/* call main */
	(void)main();
}

/**
 * Default handler for CPU exceptions.
 */
static void
default_cpu_handler(void)
{
	for (;;) ;
}

/** Prototype for optional exception vector handlers */
#define HANDLER(_name)	extern vector _name __attribute__((weak, alias("default_cpu_handler")))

/* standard CMSIS vector names */
HANDLER(NMI_Handler);
HANDLER(HardFault_Handler);
HANDLER(MemManage_Handler);
HANDLER(BusFault_Handler);
HANDLER(UsageFault_Handler);
HANDLER(DebugMon_Handler);

/* these vectors point directly to the relevant FreeRTOS functions if they are defined */
HANDLER(vPortSVCHandler);
HANDLER(xPortPendSVHandler);
HANDLER(xPortSysTickHandler);

/** CortexM3 vector table */
struct cm3_vectors cpu_vectors __attribute((section(".cpu_vectors"))) = {
		.initial_stack = &irq_stack[sizeof(irq_stack)],
		.entry = (vector *)_main,
		.vectors = {
				NMI_Handler,
				HardFault_Handler,
				MemManage_Handler,
				BusFault_Handler,
				UsageFault_Handler,
				0,
				0,
				0,
				0,
				vPortSVCHandler,
				DebugMon_Handler,
				0,
				xPortPendSVHandler,
				xPortSysTickHandler,
		}
};

/**
 * @}
 */