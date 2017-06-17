#include "pwm.h"

void pwmInit(void) {
	/* enable gpio a clock */
	RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;

	/* pwm pin, timer2 ch4, pa3, alternative function output push-pull */
	GPIOA -> CRL &= ~GPIO_CRL_MODE3_0;
	GPIOA -> CRL |= GPIO_CRL_MODE3_1;
	GPIOA -> CRL &= ~GPIO_CRL_CNF3_0;
	GPIOA -> CRL |= GPIO_CRL_CNF3_1;

	/* enable timer2 clock */
	RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;

	/* timer2 configuration */
	TIM2 -> ARR = 256;
	TIM2 -> PSC = 256;

	/* pwm configuration */
	TIM2 -> CCR4 = 0;
	TIM2 -> CCMR2 &= ~TIM_CCMR2_CC4S_0;
	TIM2 -> CCMR2 &= ~TIM_CCMR2_CC4S_1;
	TIM2 -> CCMR2 |= TIM_CCMR2_OC4PE;
	TIM2 -> CCMR2 &= ~TIM_CCMR2_OC4M_0;
	TIM2 -> CCMR2 |= TIM_CCMR2_OC4M_1;
	TIM2 -> CCMR2 |= TIM_CCMR2_OC4M_2;
	TIM2 -> CCER |= TIM_CCER_CC4E;

	/* enable timer2 */
	TIM2 -> CR1 |= TIM_CR1_CEN;
}

void pwmSet(uint8 dutyCycle) {
	TIM2 -> CCR4 = dutyCycle;
}
