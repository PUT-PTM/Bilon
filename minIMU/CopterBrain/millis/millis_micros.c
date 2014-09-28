#include "millis_micros.h"
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_tim.h"


volatile uint32_t	t_millis=0;


void init_millis_micros(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Prescaler = 168;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 500;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM2, ENABLE);
}
//===========================================================================
//===========================================================================
//===========================================================================
void TIM2_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	t_millis++;
}
//===========================================================================
//===========================================================================
//===========================================================================

uint32_t	millis(void)
{
	return t_millis;
}
//===========================================================================
//===========================================================================
//===========================================================================

uint32_t	micros(void)
{

	return ((millis()*1000) + TIM2->CNT );
}
//===========================================================================
//===========================================================================
//===========================================================================
 void	delay(uint32_t delay_ms)
{
	uint32_t start = millis();
	uint32_t ans;
	while(1)
	{
		ans = millis()- start;
		if(ans >= delay_ms)
			return;
	}
}
//===========================================================================
//===========================================================================
//===========================================================================

void	delayMicroseconds(uint32_t delay_us)
{
	uint32_t start = micros();
	uint32_t ans;
	while(1)
	{
		ans = micros()- start;
		if(ans >= delay_us)
			return;
	}

}
