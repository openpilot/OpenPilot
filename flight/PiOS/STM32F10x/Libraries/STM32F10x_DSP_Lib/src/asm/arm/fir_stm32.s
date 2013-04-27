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

  THUMB
  REQUIRE8
  PRESERVE8

  AREA |.text|, CODE, READONLY, ALIGN=2

  EXPORT  fir_16by16_stm32


a      RN R0  ; Assigned to output samples a[]     
x      RN R1  ; Assigned to input samples x[]      
			   
c      RN R2  ; Assigned to coefficients c[]       
			   
N      RN R3  ; Assigned to number of outputs (a multiple of 4)    
M      RN R4  ; Assigned number of coefficients (a multiple of 4)  

c_0    RN R3  ; coefficient registers
c_1    RN R12
c_2    RN R3
c_3    RN R12

x_0    RN R5  ; data registers
x_1    RN R6
x_2    RN R7
x_3    RN R14
x_4    RN R5
x_5	   RN R6
x_6    RN R7
x_7    RN R14

a_0    RN R8  ; output accumulators
a_1    RN R9
a_2    RN R10
a_3    RN R11

;/*******************************************************************************
;* Function Name  : fir_16by16_stm32
;* Description    : FIR 16-bit filter
;* Input          : - a: Output array .
;*                  - x: Input array 
;*                  - c: pointer to the coefficients structure
;*                  - N: the number of output samples
;* Output         : None
;* Return         : None
;*******************************************************************************/
;void fir_16by16_stm32(void *a, void *x, COEFS *c, u32 N) 

fir_16by16_stm32
        STMDB   sp!, {r4-r11, lr}   	; Save context
        LDMIA   c, {c, M}          		; Get base address and number of coefficients
next_sample
        STMFD   sp!, {N, M}
        LDRSH   x_0, [x], #2
        LDRSH   x_1, [x], #2
        LDRSH   x_2, [x], #2
        LDRSH   x_3, [x], #2
        MOV     a_0 , #0
        MOV     a_1, #0
        MOV     a_2, #0
        MOV     a_3, #0
next_tap
        ; perform next block of 4x4=16 taps
        LDRSH   c_0, [c], #2     		; Load c_0
        LDRSH   c_1, [c], #2    		; Load c_1
        SUBS    M, M, #4
        
		MLA     a_0, x_0, c_0, a_0      ; a_0 += c_0 * x_0
        LDRSH   x_4, [x], #2         	; Load x_4 (x_0 no more used)
        MLA     a_1, x_1, c_0, a_1   	; a_1 += c_0 * x_1
        MLA     a_2, x_2, c_0, a_2   	; a_2 += c_0 * x_2
        MLA     a_3, x_3, c_0, a_3   	; a_3 += c_0 * x_3
        
		LDRSH   c_2, [c], #2         	; Load c_2
        MLA     a_0, x_1, c_1, a_0    	; a_0 += c_1 * x_1
        LDRSH   x_5, [x], #2        	; Load x_5 (x_1 no more used)
        MLA     a_1, x_2, c_1, a_1  	; a_1 += c_1 * x_2
        MLA     a_2, x_3, c_1, a_2  	; a_2 += c_1 * x_3
        MLA     a_3, x_4, c_1, a_3   	; a_3 += c_1 * x_4
        
		LDRSH   c_3, [c], #2        	; Load c_3 (c_1 no more used)
        MLA     a_0, x_2, c_2, a_0     	; a_0 += c_2 * x_2
        LDRSH   x_6, [x], #2        	; Load x_6
        MLA     a_1, x_3, c_2, a_1   	; a_1 += c_2 * x_3
        MLA     a_2, x_4, c_2, a_2    	; a_2 += c_2 * x_4
        MLA     a_3, x_5, c_2, a_3   	; a_3 += c_2 * x_5
        
		MLA     a_0, x_3, c_3, a_0    	; a_0 += c_3 * x_3
        LDRSH   x_7, [x], #2        	; Load x_7
        MLA     a_1, x_4, c_3, a_1   	; a_1 += c_3 * x_4
        MLA     a_2, x_5, c_3, a_2  	; a_2 += c_3 * x_5
        MLA     a_3, x_6, c_3, a_3  	; a_3 += c_3 * x_6
        
		BGT     next_tap
        
		LDMFD   sp!, {N, M}
        STMIA   a!, {a_0, a_1, a_2, a_3}
        SUB     c, c, M, LSL#1  		; restore coefficient pointer
        SUB     x, x, M, LSL#1  		; advance data pointer
        SUBS    N, N, #4        		; filtered four samples
        BGT     next_sample

        LDMIA   sp!, {r4-r11, pc}
        END

;/******************* (C) COPYRIGHT 2009  STMicroelectronics *****END OF FILE****/
