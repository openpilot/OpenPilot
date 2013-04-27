;/******************** (C) COPYRIGHT 2009  STMicroelectronics ********************
;* File Name          : iirarma_stm32.s
;* Author             : MCD Application Team
;* Version            : V2.0.0
;* Date               : 04/27/2009
;* Description        : This source file contains IIR ARMA filter source code
;********************************************************************************
;* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
;* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
;* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
;* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
;* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
;* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
;*******************************************************************************/

  THUMB
  REQUIRE8
  PRESERVE8

  AREA |.text|, CODE, READONLY, ALIGN=2

  EXPORT  iirarma_stm32

y       RN R0    ; Assigned to output array y[]  
x       RN R1    ; Assigned to input array x[]   
h2      RN R2    ; Assigned to coefficients h2[] 
h1      RN R3    ; Assigned to coefficients h1[] 

ar_a    RN R4
ar_b    RN R5
ar_c    RN R6
ar_d    RN R7

val_ny  RN R12   ;/* number of output samples */

;/*******************************************************************************
;* Function Name  : iirarma_stm32
;* Description    : IIR order ARMA 16-bit filter
;* Input          : - y: Output array .
;*                  - x: Input array 
;*                  - h2: AutoRegressive part Filter Coefficients
;*                  - h1: Moving Average part Filter Coefficients
;*                  - ny: the number of output samples
;* Output         : None
;* Return         : None
;*******************************************************************************/
; void iirarma_stm32(void *y, void *x,  short *h2, short *h1, int ny)

iirarma_stm32
           STMDB       SP!, {R4-R7}
           LDR         R12, [SP, #+16]

           CMP      val_ny, #+1        ;if val_ny==1
           BEQ     done_for_now        ;go to done_for_now

acc_five_data
           LDRSH    ar_a, [h2, #+0]
           LDRSH    ar_b, [x, #+8]
           LDRSH    ar_c, [h2, #+2]
           LDRSH    ar_d, [x, #+6]

           SUBS     val_ny, val_ny, #+1  ;decrement val_ny
           MUL      ar_c, ar_d, ar_c
           MLA      ar_a, ar_b, ar_a, ar_c
           LDRSH    ar_b, [h1, #+2]
           LDRSH    ar_c, [y, #+6]
           MUL      ar_b, ar_c,ar_b
           LDRSH    ar_c, [x, #+4]
           SUB      ar_a, ar_a, ar_b
           LDRSH    ar_b, [h2, #+4]
           MLA      ar_a, ar_c, ar_b, ar_a
           LDRSH    ar_b, [h1, #+4]
           LDRSH    ar_c, [y, #+4]
           MUL      ar_b, ar_c, ar_b
           LDRSH    ar_c, [x, #+2]
           SUB      ar_a, ar_a, ar_b
           LDRSH    ar_b, [h2, #+6]
           MLA      ar_a, ar_c, ar_b, ar_a
           LDRSH    ar_b, [h1, #+6]
           LDRSH    ar_c, [y, #+2]
           MUL      ar_b, ar_c, ar_b
           LDRSH    ar_c, [x], #+2
           SUB      ar_a, ar_a, ar_b
           LDRSH    ar_b, [h2, #+8]
           MLA      ar_a, ar_c, ar_b, ar_a
           LDRSH    ar_b, [h1, #+8]
           LDRSH    ar_c, [y, #+0]
           MUL      ar_b, ar_c, ar_b
           SUB      ar_a, ar_a, ar_b
           ASR      ar_a, ar_a, #+15
           STRH     ar_a, [y, #+8]
           ADD      y, y, #+2
           BNE      acc_five_data

done_for_now
           LDMIA    SP!, {R4-R7}
           BX       LR               ; return
           END

;/******************* (C) COPYRIGHT 2009  STMicroelectronics *****END OF FILE****/
