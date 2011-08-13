/**
 ******************************************************************************
 * @file      startup_stm32f2xx.s
 * @author    MCD Application Team / Michael Smith: Vector table for FreeRTOS
 * @version   V0.1
 * @date      03/24/2011
 * @brief     STM32F2xx High Density Devices vector table for GCC toolchain.
 *            Derived from startup_stm32f10x_HD_OP.s
 *
 *            This module performs:
 *                - Set the initial SP
 *                - Set the initial PC == Reset_Handler,
 *                - Set the vector table entries with the exceptions ISR address
 *                - Configure the clock system and the external SRAM mounted on 
 *                  STM3210E-EVAL board to be used as data memory (optional, 
 *                  to be enabled by user)
 *                - Branches to main in the C library (which eventually
 *                  calls main()).
 *            After Reset the Cortex-M3 processor is in Thread mode,
 *            priority is Privileged, and the Stack is set to Main.
 *******************************************************************************
 * @copy
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
 */
    
#include "pios_config.h"

	.syntax		unified
	.cpu		cortex-m3
	.fpu		softvfp
	.thumb

.global  g_pfnVectors
.global  Default_Handler
.global  xPortIncreaseHeapSize
.global  Stack_Change

/* start address for the initialization values of the .data section. 
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */  
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called. 
 * @param  None
 * @retval : None
*/

    .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:

/*
 * From Cortex-M3 reference manual:
 * - Handler IRQ always use SP_main
 * - Process use SP_main or SP_process
 * Here, we will use beginning of SRAM for IRQ (SP_main)
 * and end of heap for initialization (SP_process).
 * Once the schedule starts, all threads will use their own stack
 * from heap and NOBOBY should use SP_process again.
 */
 /* Do set/reset the stack pointers */
  LDR r0, =_irq_stack_top
  MSR msp, r0
  LDR r2, =_init_stack_top
  MSR psp, r2
  /* check if irq and init stack are the same */
  /* if they are, we don't do stack swap */
  /* and lets bypass the monitoring as well for now */
  cmp  r0, r2
  beq SectionBssInit
/* DO
 * - stay in thread process mode
 * - stay in privilege level
 * - use process stack
 */
  movs r0, #2
  MSR control, r0
  ISB
/* Fill IRQ stack for watermark monitoring */
  ldr  r2, =_irq_stack_end
  b  LoopFillIRQStack

FillIRQStack:
  movw  r3, #0xA5A5
  str  r3, [r2], #4

LoopFillIRQStack:
  ldr  r3, = _irq_stack_top
  cmp  r2, r3
  bcc  FillIRQStack

SectionBssInit:
/* Copy the data segment initializers from flash to SRAM */  
  movs  r1, #0
  b  LoopCopyDataInit

CopyDataInit:
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4
    
LoopCopyDataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds  r2, r0, r1
  cmp  r2, r3
  bcc  CopyDataInit
  ldr  r2, =_sbss
  b  LoopFillZerobss
/* Zero fill the bss segment. */  
FillZerobss:
  movs  r3, #0
  str  r3, [r2], #4
    
LoopFillZerobss:
  ldr  r3, = _ebss
  cmp  r2, r3
  bcc  FillZerobss
/* Call the application's entry point.*/
  bl  main
/* will never return here */
  bx  lr
.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that swaps stack (from end of heap to irq_stack).
 * @param  None
 * @retval : None
*/
  .section  .text.Stack_Change
  .weak  Stack_Change
  .type  Stack_Change, %function
Stack_Change:
  mov r4, lr
/* Switches stack back momentarily to MSP */
  movs r0, #0
  msr control, r0
  bx r4
  .size  Stack_Change, .-Stack_Change

/**
 * @brief  Dummy SystemInit_ExtMemCtl function
 * @param  None
 * @retval : None
*/
  .section  .text.SystemInit_ExtMemCtl_Dummy,"ax",%progbits
SystemInit_ExtMemCtl_Dummy:
  bx  lr
  .size  SystemInit_ExtMemCtl_Dummy, .-SystemInit_ExtMemCtl_Dummy

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*/
    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler

/* grab the vectors from the standard file */
#define INITIAL_STACK_TOP _irq_stack_top
#include "vectors_stm32f2xx.S"
