#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>

class ESC {
	uint32_t step;
	uint32_t pinA, pinB, pinC;
	GPIO_TypeDef* port;

public:
	ESC(GPIO_TypeDef* port, uint32_t a, uint32_t b, uint32_t c);
	void commutate(void);
};

ESC::ESC(GPIO_TypeDef* gpio_port, uint32_t a, uint32_t b, uint32_t c) {
	step = 0;
	port = gpio_port;
	pinA = a;
	pinB = b;
	pinC = c;

	GPIO_InitTypeDef gpio;
	gpio.GPIO_Pin = a | b | c;
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(port, &gpio);

	this->commutate();
}

void ESC::commutate(void) {
	if(++step > 6) step = 1;
	switch(step) {
	case 1:
	case 2:
		GPIO_WriteBit(port, pinA, Bit_SET);
		GPIO_WriteBit(port, pinB | pinC, Bit_RESET);
		break;
	case 3:
	case 4:
		GPIO_WriteBit(port, pinB, Bit_SET);
		GPIO_WriteBit(port, pinA | pinC, Bit_RESET);
		break;
	case 5:
	case 6:
		GPIO_WriteBit(port, pinC, Bit_SET);
		GPIO_WriteBit(port, pinA | pinB, Bit_RESET);
		break;
	default:
		break;
	}
}

int main(void) {
	ESC esc(GPIOA, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	while(1) {
		for(int i = 0; i < 200000; i++);
		esc.commutate();
	}
}
