/**
  ******************************************************************************
  * @file STM32F10x_DSP_Lib/inc/stm32_dsp.h 
  * @author  MCD Application Team
  * @version  V2.0.0
  * @date  04/27/2009
  * @brief  This source file contains prototypes of DSP functions
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_DSP_H
#define __STM32F10x_DSP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


/* Exported types ------------------------------------------------------------*/
/* for block FIR module */
typedef struct {
  uint16_t *h;
  uint32_t nh;
} COEFS;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* FIR 16-bit filter in assembly */
void fir_16by16_stm32(void *y, void *x, COEFS *c, uint32_t N);

/* PID controller in C, error computed outside the function */
uint16_t DoPID(uint16_t Error, uint16_t *Coeff);

/* Full PID in C, error computed inside the function */
uint16_t DoFullPID(uint16_t In, uint16_t Ref, uint16_t *Coeff);

/* PID controller in assembly, error computed outside the function */
uint16_t PID_stm32(uint16_t Error, uint16_t *Coeff);

/* Radix-4 complex FFT for STM32, in assembly  */
/* 64 points*/
void cr4_fft_64_stm32(void *pssOUT, void *pssIN, uint16_t Nbin);
/* 256 points */
void cr4_fft_256_stm32(void *pssOUT, void *pssIN, uint16_t Nbin);
/* 1024 points */
void cr4_fft_1024_stm32(void *pssOUT, void *pssIN, uint16_t Nbin);

/* IIR filter in assembly */
void iirarma_stm32(void *y, void *x, uint16_t *h2, uint16_t *h1, uint32_t ny );

/* IIR filter in C */
void iir_biquad_stm32(uint16_t *y, uint16_t *x, int16_t *IIRCoeff, uint16_t ny);

#endif /* __STM32F10x_DSP_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
