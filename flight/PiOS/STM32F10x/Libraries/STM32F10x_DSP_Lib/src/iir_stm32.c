/**
  ******************************************************************************
  * @file STM32F10x_DSP_Lib/src/iir_stm32.c 
  * @author  MCD Application Team
  * @version  V2.0.0
  * @date  04/27/2009
  * @brief  This source file contains IIR functions in C
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


/* Includes ------------------------------------------------------------------*/
#include "stm32_dsp.h"
#include "stm32f10x.h"


/** @addtogroup STM32F10x_DSP_Lib
  * @{
  */ 


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Canonique Form of 8th order IIR filter, factorized in 
  *   4 biquads sections in series.
  * @param y: Output array .
  * @param x: Input array 
  * @param IIRCoeff: IIR Filter Coefficients, an array of 20 shorts
  * @param ny: the number of output samples
  * @retval : None
  */
void iir_biquad_stm32(uint16_t *y, uint16_t *x, int16_t *IIRCoeff, uint16_t ny)
{
  uint32_t i;
  uint32_t w1_2 = 0, w1_1 = 0, w1;
  uint32_t w2_2 = 0, w2_1 = 0, w2;
  uint32_t w3_2 = 0, w3_1 = 0, w3;
  uint32_t w4_2 = 0, w4_1 = 0, w4;

  /** Canonic form **/
  /* 1st section */
  for (i=0; i<ny-2; i++)
  {
    w1 = x[2+i] - IIRCoeff[0]*w1_1 - IIRCoeff[1]*w1_2;
    y[2+i] = (IIRCoeff[2]*w1 + IIRCoeff[3]*w1_1 + IIRCoeff[4]*w1_2);
    w1_2 = w1_1;
    w1_1 = w1;
  }

  /* 2nd section */
  for (i=0; i<ny-2; i++)
  {
    w2 = y[2+i] - IIRCoeff[5]*w2_1 - IIRCoeff[6]*w2_2;
    y[2+i] = (IIRCoeff[7]*w2 + IIRCoeff[8]*w2_1 + IIRCoeff[9]*w2_2);
    w2_2 = w2_1;
    w2_1 = w2;
  }

  /* 3rd section */
  for (i=0; i<ny-2; i++)
  {
    w3 = y[2+i] - IIRCoeff[10]*w3_1 - IIRCoeff[11]*w3_2;
    y[2+i] = (IIRCoeff[12]*w3 + IIRCoeff[13]*w3_1 + IIRCoeff[14]*w3_2);
    w3_2 = w3_1;
    w3_1 = w3;
  }

  /* 4th section */
  for (i=0; i<ny-2; i++)
  {
    w4 = y[2+i] - IIRCoeff[15]*w4_1 - IIRCoeff[16]*w4_2;
    y[2+i] = (IIRCoeff[17]*w4 + IIRCoeff[18]*w4_1 + IIRCoeff[19]*w4_2);
    w4_2 = w4_1;
    w4_1 = w4;
  }

}

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
