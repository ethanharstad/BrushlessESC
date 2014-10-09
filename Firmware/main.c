#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>

GPIO_InitTypeDef gpio_init_structure;

int main(void) {

	/* Configure GPIO */
	// Enable GPIO peripheral clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	// Configure GPIO structure
	gpio_init_structure.GPIO_Pin = GPIO_Pin_5;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_OUT;
	gpio_init_structure.GPIO_OType = GPIO_OType_PP;
	gpio_init_structure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_25MHz;
	// Init GPIO peripheral
	GPIO_Init(GPIOA, &gpio_init_structure);

	int led_state = 0;

	while(1) {
		for(int i = 0; i < 1000000; i++);
		GPIO_WriteBit(GPIOA, GPIO_Pin_5, led_state ? Bit_SET : Bit_RESET);
		led_state = !led_state;
	}
}
