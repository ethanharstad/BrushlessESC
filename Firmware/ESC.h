#ifndef ESC_H
#define ESC_H

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_tim.h>

class ESC {
	uint32_t step;
	uint32_t pinA, pinB, pinC;
	GPIO_TypeDef* port;
	TIM_TypeDef* tim;

public:
	void init(TIM_TypeDef* timer, GPIO_TypeDef* port, uint32_t a, uint32_t b, uint32_t c);
	void commutate(void);
};

#endif
