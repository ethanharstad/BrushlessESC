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

	// Update state
	SystemCoreClockUpdate();
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

	// Enable peripheral clocks
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC, ENABLE);

	// Setup MCO
	initMCO();

	// Setup timer
	TIM_TimeBaseInitTypeDef timer;
	timer.TIM_Prescaler = 4200;
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	timer.TIM_Period = 16;
	timer.TIM_ClockDivision = TIM_CKD_DIV1;
	timer.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &timer);
	TIM_Cmd(TIM3, ENABLE);
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	// Setup NVIC
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = TIM2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	// Init ESC
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	esc.init(TIM2, GPIOA,
			GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2,
			GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5,
			GPIO_Pin_7, GPIO_Pin_8, GPIO_Pin_9);

	/* Main loop */
	while(1) {
		/* Empty loop */
	}
}

extern "C" void TIM2_IRQHandler() {
	esc.IRQHandler();
}
