#include "ESC.h"

void ESC::init(
		TIM_TypeDef* timer, GPIO_TypeDef* gpio_port,
		uint32_t aH, uint32_t bH, uint32_t cH,
		uint32_t aL, uint32_t bL, uint32_t cL,
		uint32_t aA, uint32_t bA, uint32_t cA) {
	/* Initialize member variables */
	step = 0;
	count = 0;
	set = 42;
	tim = timer;
	port = gpio_port;
	a = aL;
	b = bL;
	c = cL;
	chA = aA;
	chB = bA;
	chC = cA;

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
	// Analog pins
	gpioInit.GPIO_Pin = aA | bA | cA;
	gpioInit.GPIO_Mode = GPIO_Mode_AN;
	gpioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
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
	TIM_ClearITPendingBit(tim, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3);
	TIM_ITConfig(tim, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3, ENABLE);

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

	/* Configure ADC */
	ADC_InitTypeDef adcInit;
	adcInit.ADC_ContinuousConvMode = DISABLE;
	adcInit.ADC_DataAlign = ADC_DataAlign_Right;
	adcInit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	adcInit.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	adcInit.ADC_NbrOfConversion = 1;
	adcInit.ADC_Resolution = ADC_Resolution_12b;
	adcInit.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1, &adcInit);
	ADC_Cmd(ADC1, ENABLE);

	// Initial commutation step
	this->commutate();
}

void ESC::IRQHandler(void) {
	if(TIM_GetITStatus(tim, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(tim, TIM_IT_Update);
		if(++count >= set) {
			this->commutate();
		}
	} else if(TIM_GetITStatus(tim, TIM_IT_CC1) != RESET) {
		TIM_ClearITPendingBit(tim, TIM_IT_CC1);
		this->measureBEMF();
	} else if(TIM_GetITStatus(tim, TIM_IT_CC2) != RESET) {
		TIM_ClearITPendingBit(tim, TIM_IT_CC2);
		this->measureBEMF();
	} else if(TIM_GetITStatus(tim, TIM_IT_CC3) != RESET) {
		TIM_ClearITPendingBit(tim, TIM_IT_CC3);
		this->measureBEMF();
	}
}

void ESC::commutate(void) {
	// Wrap step counter
	if(++step > 6) step = 1;

	// Reset commutation counter
	count = 0;

	// Configure outputs for next step
	switch(step) {
	case 1:
		// A high, B low, C float
		GPIO_WriteBit(port, a, Bit_RESET);
		GPIO_WriteBit(port, b, Bit_SET);
		GPIO_WriteBit(port, c, Bit_RESET);
		TIM_OC1Init(tim, &ocOn);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 2:
		// A high, B float, C low
		GPIO_WriteBit(port, a, Bit_RESET);
		GPIO_WriteBit(port, b, Bit_RESET);
		GPIO_WriteBit(port, c, Bit_SET);
		TIM_OC1Init(tim, &ocOn);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 3:
		// A float, B high, C low
		GPIO_WriteBit(port, a, Bit_RESET);
		GPIO_WriteBit(port, b, Bit_RESET);
		GPIO_WriteBit(port, c, Bit_SET);
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOn);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 4:
		// A low, B high, C float
		GPIO_WriteBit(port, a, Bit_SET);
		GPIO_WriteBit(port, b, Bit_RESET);
		GPIO_WriteBit(port, c, Bit_RESET);
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOn);
		TIM_OC3Init(tim, &ocOff);
		break;
	case 5:
		// A low, B float, C high
		GPIO_WriteBit(port, a, Bit_SET);
		GPIO_WriteBit(port, b, Bit_RESET);
		GPIO_WriteBit(port, c, Bit_RESET);
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOn);
		break;
	case 6:
		// A float, B low, C high
		GPIO_WriteBit(port, a, Bit_RESET);
		GPIO_WriteBit(port, b, Bit_SET);
		GPIO_WriteBit(port, c, Bit_RESET);
		TIM_OC1Init(tim, &ocOff);
		TIM_OC2Init(tim, &ocOff);
		TIM_OC3Init(tim, &ocOn);
		break;
	default:
		// Invalid step, return to 1
		step = 1;
		// Redo commutation
		this->commutate();
	}
}

void ESC::measureBEMF(void) {
	bool rising;

	// Setup ADC channel
	switch(step) {
	case 1:
		ADC_RegularChannelConfig(ADC1, chC, 1, ADC_SampleTime_3Cycles);
		rising = false;
	case 2:
		ADC_RegularChannelConfig(ADC1, chB, 1, ADC_SampleTime_3Cycles);
		rising = true;
	case 3:
		ADC_RegularChannelConfig(ADC1, chA, 1, ADC_SampleTime_3Cycles);
		rising = false;
	case 4:
		ADC_RegularChannelConfig(ADC1, chC, 1, ADC_SampleTime_3Cycles);
		rising = true;
	case 5:
		ADC_RegularChannelConfig(ADC1, chB, 1, ADC_SampleTime_3Cycles);
		rising = false;
	case 6:
		ADC_RegularChannelConfig(ADC1, chA, 1, ADC_SampleTime_3Cycles);
		rising = true;
	default:
		step = 1;
		this->commutate();
	}

	// Read BEMF
	ADC_SoftwareStartConv(ADC1);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != SET);
	ADC_GetConversionValue(ADC1);
}

void ESC::setDutyCycle(uint32_t dc) {
	// Set new duty cycle
	ocOff.TIM_Pulse = dc;

	// Copy timer init structs
	ocOn = ocOff;
	ocOn.TIM_OutputState = TIM_OutputState_Enable;
}

void ESC::setCommutationRate(uint32_t rate) {
	// Set new commutation rate
	set = rate;

	// Check if commutation is required
	if(count >= set) {
		this->commutate();
	}
}
