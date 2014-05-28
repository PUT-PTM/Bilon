#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"

#define OUT_PWM_1_RCC		RCC_AHB1Periph_GPIOA
#define OUT_PWM_1_GPIO		GPIOA
#define OUT_PWM_1_PIN		GPIO_Pin_8
#define OUT_PWM_1_PINSOURCE	GPIO_PinSource8
#define OUT_PWM_1_TIM		GPIO_AF_TIM1

#define OUT_PWM_PERIOD		20000

static inline void PWM_initIO(void)
{
	/* Enable GPIO clocks */
	RCC_AHB1PeriphClockCmd(OUT_PWM_1_RCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = OUT_PWM_1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(OUT_PWM_1_GPIO, &GPIO_InitStructure);

	/* Connect TIM pins to AF */
	GPIO_PinAFConfig(OUT_PWM_1_GPIO, OUT_PWM_1_PINSOURCE, OUT_PWM_1_TIM);
}

static inline void TIMER_t1Start(void)
{
	TIM_SetCounter(TIM1, 0);

	TIM_Cmd(TIM1, ENABLE);
}

static inline void TIMER_t1Stop(void)
{
	TIM_Cmd(TIM1, DISABLE);
}

static inline void TIMER_t1Init(void)
{
	// TODO: uwaga zegra APB2, inny podzielnik, nie zweryfikowane jak dziala
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	TIM_TimeBaseStructure.TIM_Prescaler = 185;	// define how much slower the timer counts
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = OUT_PWM_PERIOD;		// define to what value the timer counts
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x00;

	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	TIM_OCInitTypeDef  TIM_OCInitStructure;

	uint32_t CCR_Val1 = 0;

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = CCR_Val1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM1, ENABLE);

	/* TIM1 Main Output Enable */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	TIMER_t1Stop();
}

int main(void)
{
	SystemInit();

	SystemCoreClockUpdate();

	// TIM1 PWM test
	PWM_initIO();
	TIMER_t1Init();

	TIMER_t1Start();

	int value = 0;

	TIM1->CCR1 = (OUT_PWM_PERIOD/12);

	while(1)
	{
		asm("nop");
	}


	return 1;
}
