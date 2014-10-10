#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_tim.h>
#include <misc.h>

#include "ESC.h"

ESC esc;

void initRCC(void) {
	// Enable HSI
	RCC_HSICmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);

	// Configure PLL
	RCC_PLLConfig(RCC_PLLSource_HSI, 12, 252, 8, 4);
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

	// Set PLL as SYSCLK
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
}

void initMCO(void) {
	// Configure MCO pin
	GPIO_InitTypeDef gpio;
	gpio.GPIO_Pin = GPIO_Pin_8;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &gpio);

	// Map MCO to pin
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);

	// Configure MCO
	RCC_MCO1Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_4);
}

int main(void) {
	// Setup clocks
	initRCC();

	// Enable GPIOA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// Setup MCO
	initMCO();

	// Setup timer
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef timer;
	timer.TIM_Prescaler = 4200;
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	timer.TIM_Period = 16;
	timer.TIM_ClockDivision = TIM_CKD_DIV1;
	timer.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &timer);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	// Setup NVIC
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = TIM2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	// Init ESC
	esc.init(GPIOA, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7);

	/* Main loop */
	while(1) {
		/* False commutation loop
		for(int i = 0; i < 6260; i++);
		esc.commutate();*/
	}
}

extern "C" void TIM2_IRQHandler() {
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		esc.commutate();
	}
}
