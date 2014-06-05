#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
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
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 |GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Connect TIM pins to AF */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);
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
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM1, ENABLE);

	/* TIM1 Main Output Enable */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	TIMER_t1Start();
}

static inline void ADC_initIO(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //pwr for ADC1
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //pwr for input ADC pins

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static inline void ADC_adc1Init(void)
{

/*
	 ADC1->CR1 &= 0xF83F0000; //clear mask for CR1
	 ADC1->CR1 |= 0x00800000 |//set ovrie = 0| res = 00 (12bit)| awden = 1 (allow reg group analog watchdog)| jawden = 0 ( disallow injected group analog watchdog)
			 0x00000301;
*/
	ADC->CR1 = (uint32_t) 0x00000000; //czyszczenie rejestru
	ADC-CR1|= ADC_CR1_DISCNUM_2 | ADC_CR1_DISCEN | ADC_CR1_SCAN;
	ADC->CR2 = (uint32_t) 0x00000000;
	ADC->CR2 |= ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
	ADC->SMPR2 = (uint32_t) 0x00000000; //3 cykle dla wszystkich kana³ów
	ADC->SQR1 = (uint32_t) 0x00000000;
	ADC->SQR1 |= ADC_SQR1_L2;
	ADC->SQR3 = (uint32_t) 0x00000000;
	ADC->SQR3 |= ADC_SQR3_SQ1_0| ADC_SQR3_SQ2_1 | ADC_SQR3_SQ3_0 | ADC_SQR3_SQ3_1 | ADC_SQR3_SQ4_2;

	ADC->CR2 |= ADC_CR2_SWSTART;
	/*
	ADC_InitTypeDef ADC_InitStructure;

	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = 0;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_NbrOfConversion = 4;
	ADC_InitStructure.ADC_Resolution = 0; //12 bitów (najwiêksza)
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;

	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 4, ADC_SampleTime_3Cycles);

	ADC1->CR2 |= ADC_CR2_EOCS | 0x00000001; //ustawienie przerwania po ka¿dym kanale, ustawienie bitu ADON

	ADC_DiscModeChannelCountConfig(ADC1, 4);
	ADC_DiscModeCmd(ADC1, ENABLE);

	*/

}

static inline void ADC_dmaInit(void)
{
	ADC_DMACmd(ADC1, ENABLE);
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
}

int main(void)
{
	SystemInit();

	SystemCoreClockUpdate();


	PWM_initIO();
	TIMER_t1Init();

	ADC_initIO();
	ADC_adc1Init();

	int value = 0;

	TIM1->CCR1 = (OUT_PWM_PERIOD/12);
	TIM1->CCR2 = (OUT_PWM_PERIOD/12);
	TIM1->CCR3 = (OUT_PWM_PERIOD/12);
	TIM1->CCR4 = (OUT_PWM_PERIOD/12);
	while(1)
	{
		value=ADC_GetConversionValue(ADC1);
		TIM1->CCR1=value;
	}


	return 1;
}
