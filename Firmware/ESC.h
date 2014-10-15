#ifndef ESC_H
#define ESC_H

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_adc.h>

class ESC {
	uint32_t count, step;
	uint32_t set;
	GPIO_TypeDef* port;
	uint32_t a, b, c;
	uint32_t chA, chB, chC;
	TIM_TypeDef* tim;
	TIM_OCInitTypeDef ocOn, ocOff;

	void commutate(void);
	void measureBEMF(void);

public:
	void init(
			TIM_TypeDef* timer, GPIO_TypeDef* port,
			uint32_t aH, uint32_t bH, uint32_t cH,
			uint32_t aL, uint32_t bL, uint32_t cL,
			uint32_t aA, uint32_t bA, uint32_t cA);
	void setDutyCycle(uint32_t dc);
	void setCommutationRate(uint32_t rate);
	void IRQHandler(void);
};

#endif
