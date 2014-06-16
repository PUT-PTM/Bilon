#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"

#define OUT_PWM_1_RCC		RCC_AHB1Periph_GPIOA
#define OUT_PWM_1_GPIO		GPIOA
#define OUT_PWM_1_PIN		GPIO_Pin_8
#define OUT_PWM_1_PINSOURCE	GPIO_PinSource8
#define OUT_PWM_1_TIM		GPIO_AF_TIM1

#define OUT_PWM_PERIOD		20000
unsigned long long  bufor;
unsigned int fi = 10;

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
	ADC1->CR1 = (uint32_t) 0x00000000; //czyszczenie rejestru
	ADC1->CR1|= ADC_CR1_DISCNUM_2 | ADC_CR1_DISCEN | ADC_CR1_SCAN;
	ADC1->CR2 = (uint32_t) 0x00000000;
	ADC1->CR2 |= ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
	ADC1->SMPR2 = (uint32_t) 0x00000000; //3 cykle dla wszystkich kana³ów
	ADC1->SQR1 = (uint32_t) 0x00000000;
	ADC1->SQR1 |= ADC_SQR1_L2;
	ADC1->SQR3 = (uint32_t) 0x00000000;
	ADC1->SQR3 |= ADC_SQR3_SQ1_0| ADC_SQR3_SQ2_1 | ADC_SQR3_SQ3_0 | ADC_SQR3_SQ3_1 | ADC_SQR3_SQ4_2;

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

static inline void DMA2_Init(void){
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2, ENABLE);
	DMA_DeInit(DMA2_Stream0);
	DMA2_Stream0->CR |= DMA_MemoryDataSize_HalfWord | DMA_PeripheralDataSize_HalfWord | DMA_MemoryInc_Enable | DMA_Mode_Circular;
	DMA2_Stream0->NDTR |= (uint32_t)0x00000004;
	DMA2_Stream0->PAR |= (uint32_t)0x4001204c;//adres rejestru adc
	DMA2_Stream0->M0AR |= (uint32_t)&bufor;
	DMA2->LISR = 0x00000000;
	DMA2->HISR = 0x00000000;

}

static inline void DMA2_Start(void){
	DMA2->LISR = 0x00000000;
	DMA2->HISR = 0x00000000;
	DMA2_Stream0->NDTR |= (uint32_t)0x00000004;
	DMA2_Stream0->M0AR |= (uint32_t)&bufor;
	DMA2_Stream0->CR |= DMA_SxCR_EN;
}

int main(void)
{
	SystemInit();

	SystemCoreClockUpdate();


	PWM_initIO();
	TIMER_t1Init();

	ADC_initIO();
	ADC_adc1Init();

	DMA2_Init();
	DMA2_Start();
	int value = 0;

	TIM1->CCR1 = (OUT_PWM_PERIOD/12);
	TIM1->CCR2 = (OUT_PWM_PERIOD/12);
	TIM1->CCR3 = (OUT_PWM_PERIOD/12);
	TIM1->CCR4 = (OUT_PWM_PERIOD/12);
	while(1)
	{
		int krok=1; //stan silnika
		int
		int bufor1, bufor2, bufor3, bufor4;
		value=ADC_GetConversionValue(ADC1);
		TIM1->CCR1=value;
		if((ADC1->SR & 0x00000002)==1)
		{
			bufor1=(int)(bufor>>48); //faza w
			bufor2=(int)((bufor & 0x0000111100000000)>>32); //faza v
			bufor3=(int)((bufor & 0x0000000011110000)>>16);// faza u
			bufor4=(int)(bufor & 0x0000000000001111); // suma

			switch(krok){

			case 1:
				if((bufor1 >= bufor4-fi)&&(bufor1 <= bufor4 + fi)&&krok==1)
				{
					//prze³¹czenie na krok 2
					krok=2;
					//w³¹czenie fazy u(+) i w(-), v(np)
				}
				break;
			case 2:
				if((bufor2 >= bufor4-fi)&&(bufor2 <= bufor4 + fi))
				{
					//prze³¹czenie na krok 3
					krok=3;
					//w³¹czenie fazy u(np), w(-), v(+)
				}
				break;
			case 3:
				if((bufor3 >= bufor4-fi)&&(bufor3 <= bufor4 + fi))
				{
					//prze³¹czenie na krok 4
					krok=4;
					//w³¹czenie fazy u(-), w(np), v(+)
				}
				break;
			case 4:
				if((bufor1 >= bufor4-fi)&&(bufor1 <= bufor4 + fi)&&krok==1)
				{
					//prze³¹czenie na krok 5
					krok=5;
					//w³¹czenie fazy u(-), w(+), v(np)
				}
				break;
			case 5:
				if((bufor2 >= bufor4-fi)&&(bufor2 <= bufor4 + fi))
				{
					//prze³¹czenie na krok 6
					krok=6;
					//w³¹czenie fazy u(np), w(+), v(-)
				}
				break;
			case 6:
				if((bufor3 >= bufor4-fi)&&(bufor3 <= bufor4 + fi))
				{
					//prze³¹czenie na krok 1
					krok=1;
					//w³¹czenie fazy u(+), w(np), v(-)
				}
				break;
			}
	}


	return 1;
}
