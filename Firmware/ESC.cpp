#include "ESC.h"

void ESC::init(TIM_TypeDef* timer, GPIO_TypeDef* gpio_port, uint32_t a, uint32_t b, uint32_t c) {
	/* Initialize member variables */
	step = 0;
	tim = timer;
	port = gpio_port;

	/* Configure GPIO */
	GPIO_InitTypeDef gpioInit;
	gpioInit.GPIO_Pin = a | b | c;
	gpioInit.GPIO_Mode = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(port, &gpioInit);

	/* Configure timer */
	TIM_TimeBaseInitTypeDef timerInit;
	timerInit.TIM_CounterMode = TIM_CounterMode_Up;
	timerInit.TIM_RepetitionCounter = 0;
	timerInit.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInit.TIM_Prescaler = 420;
	timerInit.TIM_Period = 16;
	TIM_TimeBaseInit(timer, &timerInit);
	TIM_Cmd(timer, ENABLE);

	/* Configure PWM */
	TIM_OCInitTypeDef ocInit;
	ocInit.TIM_OCMode = TIM_OCMode_PWM1;
	ocInit.TIM_OCPolarity = TIM_OCPolarity_High;
	ocInit.TIM_OutputState = TIM_OutputState_Enable;
	ocInit.TIM_Pulse = 8;
	TIM_OC1Init(timer, &ocInit);
	TIM_OC2Init(timer, &ocInit);
	TIM_OC3Init(timer, &ocInit);
	TIM_OC1PreloadConfig(timer, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(timer, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(timer, TIM_OCPreload_Enable);
	GPIO_PinAFConfig(port, GPIO_PinSource0, GPIO_AF_TIM2);
	GPIO_PinAFConfig(port, GPIO_PinSource1, GPIO_AF_TIM2);
	GPIO_PinAFConfig(port, GPIO_PinSource2, GPIO_AF_TIM2);

	this->commutate();
}

void ESC::commutate(void) {
	// Wrap step counter
	if(++step > 6) step = 1;

	TIM_OCInitTypeDef ocOn, ocOff;
	ocOn.TIM_OCMode = TIM_OCMode_PWM1;
	ocOn.TIM_OCPolarity = TIM_OCPolarity_High;
	ocOn.TIM_OutputState = TIM_OutputState_Enable;
	ocOn.TIM_Pulse = 8;
	ocOff = ocOn;
	ocOff.TIM_OutputState = TIM_OutputState_Disable;

	switch(step) {
	case 1:
	case 2:
		TIM_OC1Init(tim, &ocOn);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 3:
	case 4:
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOn);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 5:
	case 6:
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOn);
		break;
	default:
		break;
	}
}
