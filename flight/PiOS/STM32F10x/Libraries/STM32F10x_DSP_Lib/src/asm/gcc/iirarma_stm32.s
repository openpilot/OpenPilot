/*;******************* (C) COPYRIGHT 2009  STMicroelectronics ********************
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

.cpu cortex-m3
.fpu softvfp   
.syntax unified
.thumb
.text	

.global  iirarma_stm32


/*;******************************************************************************
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
/* void iirarma_stm32(void *y, void *x,  short *h2, short *h1, int ny) */
.thumb_func
iirarma_stm32:
           STMDB       SP!, {R4-R7}
           LDR         R12, [SP, #+16]

           CMP      r12, #+1        /*if val_ny==1*/
           BEQ     done_for_now     /*go to done_for_now*/

acc_five_data:
           LDRSH    r4, [r2, #+0]
           LDRSH    r5, [r1, #+8]
           LDRSH    r6, [r2, #+2]
           LDRSH    r7, [r1, #+6]

           SUBS     r12, r12, #+1   /*decrement val_ny*/
           MUL      r6, r7, r6
           MLA      r4, r5, r4, r6
           LDRSH    r5, [r3, #+2]
           LDRSH    r6, [r0, #+6]
           MUL      r5, r6,r5
           LDRSH    r6, [r1, #+4]
           SUB      r4, r4, r5
           LDRSH    r5, [r2, #+4]
           MLA      r4, r6, r5, r4
           LDRSH    r5, [r3, #+4]
           LDRSH    r6, [r0, #+4]
           MUL      r5, r6, r5
           LDRSH    r6, [r1, #+2]
           SUB      r4, r4, r5
           LDRSH    r5, [r2, #+6]
           MLA      r4, r6, r5, r4
           LDRSH    r5, [r3, #+6]
           LDRSH    r6, [r0, #+2]
           MUL      r5, r6, r5
           LDRSH    r6, [r1], #+2
           SUB      r4, r4, r5
           LDRSH    r5, [r2, #+8]
           MLA      r4, r6, r5, r4
           LDRSH    r5, [r3, #+8]
           LDRSH    r6, [r0, #+0]
           MUL      r5, r6, r5
           SUB      r4, r4, r5
           ASR      r4, r4, #+15
           STRH     r4, [r0, #+8]
           ADD      r0, r0, #+2
           BNE      acc_five_data

done_for_now:
           LDMIA    SP!, {R4-R7}
           BX       LR               /* return */
.end          

/;******************* (C) COPYRIGHT 2009  STMicroelectronics *****END OF FILE****/
