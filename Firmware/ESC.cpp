#include "ESC.h"

void ESC::init(
		TIM_TypeDef* timer, GPIO_TypeDef* gpio_port,
		uint32_t aH, uint32_t bH, uint32_t cH,
		uint32_t aL, uint32_t bL, uint32_t cL) {
	/* Initialize member variables */
	step = 0;
	count = 0;
	set = 42;
	tim = timer;
	port = gpio_port;
	a = aL;
	b = bL;
	c = cL;

	/* Configure GPIO */
	GPIO_InitTypeDef gpioInit;
	// PWM Pins
	gpioInit.GPIO_Pin = aH | bH | cH;
	gpioInit.GPIO_Mode = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd = GPIO_PuPd_DOWN;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(port, &gpioInit);
	// Output pins
	gpioInit.GPIO_Pin = aL | bL | cL;
	gpioInit.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(port, &gpioInit);

	/* Configure timer */
	TIM_TimeBaseInitTypeDef timerInit;
	timerInit.TIM_CounterMode = TIM_CounterMode_Up;
	timerInit.TIM_RepetitionCounter = 0;
	timerInit.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInit.TIM_Prescaler = 50;
	timerInit.TIM_Period = 31;
	TIM_TimeBaseInit(tim, &timerInit);
	TIM_Cmd(tim, ENABLE);
	TIM_ClearITPendingBit(tim, TIM_IT_Update);
	TIM_ITConfig(tim, TIM_IT_Update, ENABLE);

	/* Configure PWM */
	ocOff.TIM_OCMode = TIM_OCMode_PWM1;
	ocOff.TIM_OCPolarity = TIM_OCPolarity_High;
	ocOff.TIM_OutputState = TIM_OutputState_Disable;
	ocOff.TIM_Pulse = 15;
	TIM_OC1Init(tim, &ocOff);
	TIM_OC2Init(tim, &ocOff);
	TIM_OC3Init(tim, &ocOff);
	TIM_OC1PreloadConfig(tim, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(tim, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(tim, TIM_OCPreload_Enable);

	/* Connect PWM to GPIO */
	//TODO Decouple GPIO_PinSource
	//TODO Decouple GPIO_AF
	GPIO_PinAFConfig(port, GPIO_PinSource0, GPIO_AF_TIM2);
	GPIO_PinAFConfig(port, GPIO_PinSource1, GPIO_AF_TIM2);
	GPIO_PinAFConfig(port, GPIO_PinSource2, GPIO_AF_TIM2);

	this->commutate();
}

void ESC::IRQHandler(void) {
	if(TIM_GetITStatus(tim, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(tim, TIM_IT_Update);
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
		GPIO_WriteBit(port, a, Bit_RESET);
		GPIO_WriteBit(port, b, Bit_SET);
		GPIO_WriteBit(port, c, Bit_RESET);
		TIM_OC1Init(tim, &ocOn);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 2:
		GPIO_WriteBit(port, a, Bit_RESET);
		GPIO_WriteBit(port, b, Bit_RESET);
		GPIO_WriteBit(port, c, Bit_SET);
		TIM_OC1Init(tim, &ocOn);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 3:
		GPIO_WriteBit(port, a, Bit_RESET);
		GPIO_WriteBit(port, b, Bit_RESET);
		GPIO_WriteBit(port, c, Bit_SET);
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOn);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 4:
		GPIO_WriteBit(port, a, Bit_SET);
		GPIO_WriteBit(port, b, Bit_RESET);
		GPIO_WriteBit(port, c, Bit_RESET);
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOn);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 5:
		GPIO_WriteBit(port, a, Bit_SET);
		GPIO_WriteBit(port, b, Bit_RESET);
		GPIO_WriteBit(port, c, Bit_RESET);
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOn);
		break;
	case 6:
		GPIO_WriteBit(port, a, Bit_RESET);
		GPIO_WriteBit(port, b, Bit_SET);
		GPIO_WriteBit(port, c, Bit_RESET);
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOn);
		break;
	default:
		step = 1;
		break;
	}
}
