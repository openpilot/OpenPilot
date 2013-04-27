/******************** (C) COPYRIGHT 2009  STMicroelectronics ********************
* File Name          : iirarma_stm32.s
* Author             : MCD Application Team
;* Version            : V2.0.0
;* Date               : 04/27/2009
* Description        : This source file contains IIR ARMA filter source code
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

  SECTION .text:CODE(2)

  PUBLIC  iirarma_stm32

#define y       R0    /* Assigned to output array y[]  */
#define x       R1    /* Assigned to input array x[]   */
#define h2      R2    /* Assigned to coefficients h2[] */
#define h1      R3    /* Assigned to coefficients h1[] */

#define ar_a    R4
#define ar_b    R5
#define ar_c    R6
#define ar_d    R7

#define val_ny  R12   /* number of output samples */


/*******************************************************************************
* Function Name  : iirarma_stm32
* Description    : IIR order ARMA 16-bit filter
* Input          : - y: Output array .
*                  - x: Input array 
*                  - h2: AutoRegressive part Filter Coefficients
*                  - h1: Moving Average part Filter Coefficients
*                  - ny: the number of output samples
* Output         : None
* Return         : None
*******************************************************************************/
/* void iirarma_stm32(void *y, void *x,  short *h2, short *h1, int ny); */
iirarma_stm32:
           STMDB       SP!, {R4-R7}
           LDR         R12, [SP, #+16]
;
           CMP      val_ny, #+1           ;if val_ny==1
           BEQ     done_for_now          ;go to done_for_now
;
acc_five_data:
           LDRSH    ar_a, [h2, #+0]
           LDRSH    ar_b, [x, #+8]
           LDRSH    ar_c, [h2, #+2]
           LDRSH    ar_d, [x, #+6]
;
           SUBS     val_ny, val_ny, #+1     ;decrement val_ny
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
;
done_for_now:
           LDMIA    SP!, {R4-R7}
           BX       LR               ;; return
           END

/******************* (C) COPYRIGHT 2009  STMicroelectronics *****END OF FILE****/
