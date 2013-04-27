;/******************** (C) COPYRIGHT 2009  STMicroelectronics ********************
;* File Name          : fir_stm32.s
;* Author             : MCD Application Team
;* Version            : V2.0.0
;* Date               : 04/27/2009
;* Description        : FIR filter optimized assembly code for Cortex-M3
;********************************************************************************
;* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
;* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
;* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
;* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
;* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
;* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
;*******************************************************************************/
;/*;-----------------------------------------------------------------------------
; This software module is adopted from the book entitled,
; "ARM System Developers Guide" published by Elsevier Inc in 2004
; and the module is presented "as is with no warranty".
; for Evaluation purposes to support Cortex-M3 (STM32)
; Code compatible with IAR/ARM assembly.
;---------------------------------------------------------------------------- */

.cpu cortex-m3
.fpu softvfp   
.syntax unified
.thumb
.text	

.global  fir_16by16_stm32


/*;******************************************************************************
;* Function Name  : fir_16by16_stm32
;* Description    : FIR 16-bit filter
;* Input          : - a: Output array .
;*                  - x: Input array 
;*                  - c: pointer to the coefficients structure
;*                  - N: the number of output samples
;* Output         : None
;* Return         : None
;*******************************************************************************/
/* void fir_16by16_stm32(void *a, void *x, COEFS *c, u32 N) */
.thumb_func
fir_16by16_stm32:
        
        STMDB   sp!, {r4-r11, lr}   	/* Save context */
        
        /* Get base address and number of coefficients */
        /* LDMIA   r2, {r2, r4} */          	
        LDR     r4, [r2, #4]
        LDR     r2, [r2]

next_sample:
        STMFD   sp!, {r3, r4}
        LDRSH   r5, [r1], #2
        LDRSH   r6, [r1], #2
        LDRSH   r7, [r1], #2
        LDRSH   r14, [r1], #2
        MOV     r8 , #0
        MOV     r9, #0
        MOV     r10, #0
        MOV     r11, #0

next_tap:
        /* perform next block of 4x4=16 taps */
        LDRSH   r3, [r2], #2     	/* Load c_0 */
        LDRSH   r12, [r2], #2    	/* Load c_1 */
        SUBS    r4, r4, #4
        
        MLA     r8, r5, r3, r8      /* a_0 += c_0 * x_0 */
        LDRSH   r5, [r1], #2         	/* Load x_4 (x_0 no more used) */
        MLA     r9, r6, r3, r9   	/* a_1 += c_0 * x_1 */
        MLA     r10, r7, r3, r10   	/* a_2 += c_0 * x_2 */
        MLA     r11, r14, r3, r11   	/* a_3 += c_0 * x_3 */
        
        LDRSH   r3, [r2], #2         	/* Load c_2 */
        MLA     r8, r6, r12, r8    	/* a_0 += c_1 * x_1 */
        LDRSH   r6, [r1], #2        	/* Load x_5 (x_1 no more used) */
        MLA     r9, r7, r12, r9  	/* a_1 += c_1 * x_2 */
        MLA     r10, r14, r12, r10  	/* a_2 += c_1 * x_3 */
        MLA     r11, r5, r12, r11   	/* a_3 += c_1 * x_4 */
        
        LDRSH   r12, [r2], #2        	/* Load c_3 (c_1 no more used) */
        MLA     r8, r7, r3, r8     	/* a_0 += c_2 * x_2 */
        LDRSH   r7, [r1], #2        	/* Load x_6 */
        MLA     r9, r14, r3, r9   	/* a_1 += c_2 * x_3 */
        MLA     r10, r5, r3, r10    	/* a_2 += c_2 * x_4 */
        MLA     r11, r6, r3, r11   	/* a_3 += c_2 * x_5 */ 
        
        MLA     r8, r14, r12, r8    	/* a_0 += c_3 * x_3 */
        LDRSH   r14, [r1], #2        	/* Load x_7 */
        MLA     r9, r5, r12, r9   	/* a_1 += c_3 * x_4 */
        MLA     r10, r6, r12, r10  	/* a_2 += c_3 * x_5 */
        MLA     r11, r7, r12, r11  	/* a_3 += c_3 * x_6 */
        
        BGT     next_tap
        
        LDMFD   sp!, {r3, r4}
        STMIA   r0!, {r8, r9, r10, r11}
        SUB     r2, r2, r4, LSL#1  	/* restore coefficient pointer */
        SUB     r1, r1, r4, LSL#1  	/* advance data pointer */ 
        SUBS    r3, r3, #4        	/* filtered four samples */
        BGT     next_sample

        LDMIA   sp!, {r4-r11, pc}
.end

/*;****************** (C) COPYRIGHT 2009  STMicroelectronics *****END OF FILE****/
