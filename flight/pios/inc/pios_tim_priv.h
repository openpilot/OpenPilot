#ifndef PIOS_TIM_PRIV_H
#define PIOS_TIM_PRIV_H

#include <pios_stm32.h>

struct pios_tim_clock_cfg {
	TIM_TypeDef * timer;
	const TIM_TimeBaseInitTypeDef * time_base_init;
	struct stm32_irq irq;
};

struct pios_tim_channel {
	TIM_TypeDef * timer;
	uint8_t timer_chan;

	struct stm32_gpio pin;
	uint32_t remap;
};

struct pios_tim_callbacks {
	void (*overflow)(uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count);
	void (*edge)(uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count);
};

extern int32_t PIOS_TIM_InitClock(const struct pios_tim_clock_cfg * cfg);
extern int32_t PIOS_TIM_InitChannels(uint32_t * tim_id, const struct pios_tim_channel * channels, uint8_t num_channels, const struct pios_tim_callbacks * callbacks, uint32_t context);

#endif	/* PIOS_TIM_PRIV_H */
