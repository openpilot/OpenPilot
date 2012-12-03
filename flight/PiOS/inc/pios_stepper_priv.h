struct pios_stepper_dir_cfg {
	struct stm32_gpio pin;
};

struct pios_stepper_out_cfg {
	TIM_TimeBaseInitTypeDef tim_base_init;
	TIM_OCInitTypeDef tim_oc_init;
	GPIO_InitTypeDef gpio_init;
	uint32_t remap;
	const struct pios_tim_channel * channels;
	const struct pios_stepper_dir_cfg * directions;
	uint8_t num_channels;
};



extern int32_t PIOS_STEPPER_Out_Init(uint32_t * pios_stepper_id, const struct pios_stepper_out_cfg * cfg);
