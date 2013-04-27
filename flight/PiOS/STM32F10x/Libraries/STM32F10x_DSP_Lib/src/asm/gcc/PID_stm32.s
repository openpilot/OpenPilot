/*;******************* (C) COPYRIGHT 2009  STMicroelectronics ********************
;* File Name          : PID_stm32.s
;* Author             : MCD Application Team
;* Version            : V2.0.0
;* Date               : 04/27/2009
;* Description        : This source file contains assembly optimized source code
;*                      of a PID controller.
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

.global  PID_stm32
.extern IntTerm
.extern PrevError


/*;******************************************************************************
;* Function Name  : DoPID
;* Description    : PID in ASM, Error computed outside the routine
;* Input          : Error: difference between reference and measured value
;*                  Coeff: pointer to the coefficient table
;* Output         : None
;* Return         : PID output (command)
;*******************************************************************************/
.thumb_func
PID_stm32:

  PUSH {R4, R5, R9}

  LDR R12, =IntTerm
  LDR R9, =PrevError

  LDRH r3, [r1, #0]  		/* Load Kp */
  LDRH r2, [r1, #2]  		/* Load Ki */
  LDRH r1, [r1, #4]  		/* Load Kd and destroy Coeff */
  LDRH r5, [R12, #0]  		/* Last Integral Term */
  LDRH r12, [R9, #0]  		/* Previous Error */

  MLA r5, r2, r0, r5   	   /* IntTerm += Ki*error  */
  MLA r4, r3, r0, r5      	/* Output = (Kp * error) + InTerm  */
  SUBS r12, r0, r12        /* PrevErr now holds DeltaError = Error - PrevError */
  MLA r2, r1, r12, r4      /* Output += Kd * DeltaError */

  LDR R12, =IntTerm
  STRH r5, [R12, #0]       /* Write back InTerm  */
  STRH r0, [R9, #0]        /* Write back PrevError */

  MOV R0, r2
  UXTH R0, R0
  POP {R4, R5, R9}
  BX LR

.end

/;******************* (C) COPYRIGHT 2009  STMicroelectronics *****END OF FILE****/
