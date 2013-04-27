;/******************** (C) COPYRIGHT 2009  STMicroelectronics ********************
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

  THUMB
  REQUIRE8
  PRESERVE8

  AREA |.text|, CODE, READONLY, ALIGN=2

  EXPORT PID_stm32
  IMPORT IntTerm
  IMPORT PrevError

Err     RN R0    				; 1st function input: Error  
Coeff   RN R1    				; 2nd fct input: Address of coefficient table 
Kd      RN R1
Ki      RN R2
Kp      RN R3

Out     RN R4
Result  RN R2
Integ   RN R5
PrevErr RN R12

;/*******************************************************************************
;* Function Name  : DoPID
;* Description    : PID in ASM, Error computed outside the routine
;* Input          : Error: difference between reference and measured value
;*                  Coeff: pointer to the coefficient table
;* Output         : None
;* Return         : PID output (command)
;*******************************************************************************/
PID_stm32

  PUSH {R4, R5, R9}

  LDR R12, =IntTerm
  LDR R9, =PrevError

  LDRH Kp, [Coeff, #0]  		; Load Kp 
  LDRH Ki, [Coeff, #2]  		; Load Ki 
  LDRH Kd, [Coeff, #4]  		; Load Kd and destroy Coeff
  LDRH Integ, [R12, #0]  		; Last Integral Term 
  LDRH PrevErr, [R9, #0]  		; Previous Error 

  MLA Integ, Ki, Err, Integ   	; IntTerm += Ki*error 
  MLA Out, Kp, Err, Integ      	; Output = (Kp * error) + InTerm 
  SUBS PrevErr, Err, PrevErr    ; PrevErr now holds DeltaError = Error - PrevError 
  MLA Result, Kd, PrevErr, Out  ; Output += Kd * DeltaError 

  LDR R12, =IntTerm
  STRH Integ, [R12, #0]       	; Write back InTerm 
  STRH Err, [R9, #0]         	; Write back PrevError 

  MOV R0, Result
  UXTH R0, R0
  POP {R4, R5, R9}
  BX LR

  END

;/******************* (C) COPYRIGHT 2009  STMicroelectronics *****END OF FILE****/
