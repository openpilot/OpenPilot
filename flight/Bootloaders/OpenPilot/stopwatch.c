

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include "stm32f10x_tim.h"





/////////////////////////////////////////////////////////////////////////////
// Local definitions
/////////////////////////////////////////////////////////////////////////////

#define STOPWATCH_TIMER_BASE                 TIM6
#define STOPWATCH_TIMER_RCC   RCC_APB1Periph_TIM6

uint32_t STOPWATCH_Init(u32 resolution)
{
  // enable timer clock
  if( STOPWATCH_TIMER_RCC == RCC_APB2Periph_TIM1 || STOPWATCH_TIMER_RCC == RCC_APB2Periph_TIM8 )
    RCC_APB2PeriphClockCmd(STOPWATCH_TIMER_RCC, ENABLE);
  else
    RCC_APB1PeriphClockCmd(STOPWATCH_TIMER_RCC, ENABLE);

  // time base configuration
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = 0xffff; // max period
  TIM_TimeBaseStructure.TIM_Prescaler = (72 * resolution)-1; // <resolution> uS accuracy @ 72 MHz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(STOPWATCH_TIMER_BASE, &TIM_TimeBaseStructure);

  // enable interrupt request
  TIM_ITConfig(STOPWATCH_TIMER_BASE, TIM_IT_Update, ENABLE);

  // start counter
  TIM_Cmd(STOPWATCH_TIMER_BASE, ENABLE);

  return 0; // no error
}


/////////////////////////////////////////////////////////////////////////////
//! Resets the stopwatch
//! \return < 0 on errors
/////////////////////////////////////////////////////////////////////////////
uint32_t STOPWATCH_Reset(void)
{
  // reset counter
  STOPWATCH_TIMER_BASE->CNT = 1; // set to 1 instead of 0 to avoid new IRQ request
  TIM_ClearITPendingBit(STOPWATCH_TIMER_BASE, TIM_IT_Update);

  return 0; // no error
}


/////////////////////////////////////////////////////////////////////////////
//! Returns current value of stopwatch
//! \return 1..65535: valid stopwatch value
//! \return 0xffffffff: counter overrun
/////////////////////////////////////////////////////////////////////////////
uint32_t STOPWATCH_ValueGet(void)
{
	uint32_t value = STOPWATCH_TIMER_BASE->CNT;

  if( TIM_GetITStatus(STOPWATCH_TIMER_BASE, TIM_IT_Update) != RESET )
    value = 0xffffffff;

  return value;
}

