#include "ESC.h"

void ESC::init(TIM_TypeDef* timer, GPIO_TypeDef* gpio_port, uint32_t a, uint32_t b, uint32_t c) {
	/* Initialize member variables */
	step = 0;
	count = 0;
	set = 42;
	tim = timer;
	port = gpio_port;

	/* Configure GPIO */
	GPIO_InitTypeDef gpioInit;
	gpioInit.GPIO_Pin = a | b | c;
	gpioInit.GPIO_Mode = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd = GPIO_PuPd_DOWN;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(port, &gpioInit);

	/* Configure timer */
	TIM_TimeBaseInitTypeDef timerInit;
	timerInit.TIM_CounterMode = TIM_CounterMode_Up;
	timerInit.TIM_RepetitionCounter = 0;
	timerInit.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInit.TIM_Prescaler = 50;
	timerInit.TIM_Period = 31;
	TIM_TimeBaseInit(timer, &timerInit);
	TIM_Cmd(timer, ENABLE);
	TIM_ClearITPendingBit(timer, TIM_IT_Update);
	TIM_ITConfig(timer, TIM_IT_Update, ENABLE);

	/* Configure PWM */
	ocOff.TIM_OCMode = TIM_OCMode_PWM1;
	ocOff.TIM_OCPolarity = TIM_OCPolarity_High;
	ocOff.TIM_OutputState = TIM_OutputState_Disable;
	ocOff.TIM_Pulse = 15;
	TIM_OC1Init(timer, &ocOff);
	TIM_OC2Init(timer, &ocOff);
	TIM_OC3Init(timer, &ocOff);
	TIM_OC1PreloadConfig(timer, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(timer, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(timer, TIM_OCPreload_Enable);
	GPIO_PinAFConfig(port, GPIO_PinSource0, GPIO_AF_TIM2);
	GPIO_PinAFConfig(port, GPIO_PinSource1, GPIO_AF_TIM2);
	GPIO_PinAFConfig(port, GPIO_PinSource2, GPIO_AF_TIM2);

	this->commutate();
}

void ESC::IRQHandler(void) {
	if(TIM_GetITStatus(timer, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(timer, TIM_IT_Update);
		if(++count >= set) {
			count = 0;
			this->commutate();
		}
	}
}

void ESC::commutate(void) {
	// Wrap step counter
	if(++step > 6) step = 1;

	ocOn = ocOff;
	ocOn.TIM_OutputState = TIM_OutputState_Enable;

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
