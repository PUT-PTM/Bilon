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
#define FAZA_UG				GPIO_Pin_1
#define FAZA_VG				GPIO_Pin_2
#define FAZA_WG				GPIO_Pin_3
#define FAZA_UDG			GPIO_Pin_4
#define FAZA_VDG			GPIO_Pin_5
#define FAZA_WDG			GPIO_Pin_6
#define FAZA_UDD			GPIO_Pin_7
#define FAZA_VDD			GPIO_Pin_8
#define FAZA_WDD			GPIO_Pin_9

#define OUT_PWM_PERIOD		20000
unsigned long long  bufor;
unsigned int fi = 10;


void inline turn_on(int gora, int dol){
	GPIO_ResetBits(GPIOD, dol);
	GPIO_SetBits(GPIOD, gora);
}

void inline turn_off(int gora, int dol){
	GPIO_ResetBits(GPIOD, gora);
	GPIO_SetBits(GPIOD, dol);
}

static inline void GPIO_outputInit(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |GPIO_Pin_4 |GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |GPIO_Pin_8 |GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_Init(GPIOD, &GPIO_InitStructure);

		GPIO_ResetBits(GPIOD, FAZA_VDG);
		GPIO_ResetBits(GPIOD, FAZA_VDD);

		GPIO_ResetBits(GPIOD, FAZA_VG);
		GPIO_ResetBits(GPIOD, FAZA_WDG);
		GPIO_ResetBits(GPIOD, FAZA_WDD);

		GPIO_ResetBits(GPIOD, FAZA_WG);
		GPIO_ResetBits(GPIOD, FAZA_UDG);
		GPIO_ResetBits(GPIOD, FAZA_UDD);

		GPIO_ResetBits(GPIOD, FAZA_UG);
}

static inline void PWM_initIO(void)
{
	/* Enable GPIO clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 |GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Connect TIM pins to AF */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_TIM1);

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
/*
static inline void ADC_initIO(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //pwr for ADC1
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //pwr for input ADC pins

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = FAZA_VG | FAZA_UG | FAZA_WG | FAZA_VD | FAZA_UD | FAZA_WD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
*/

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
	ADC1->SQR1 |= ADC_SQR1_L_2;
	ADC1->SQR3 = (uint32_t) 0x00000000;
	ADC1->SQR3 |= ADC_SQR3_SQ1_0| ADC_SQR3_SQ2_1 | ADC_SQR3_SQ3_0 | ADC_SQR3_SQ3_1 | ADC_SQR3_SQ4_2;

	ADC1->CR2 |= ADC_CR2_SWSTART;
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

void wait(int i)
{
	while(i!=0)
	{
		i--;
	}
}

static inline void Rozruch(int * krok){
	TIM1->CCR1=100;
	//GPIO_SetBits(GPIOD, FAZA_WD);
	turn_on(FAZA_WDG,FAZA_WDD);
	GPIO_SetBits(GPIOD, FAZA_UG);
	*krok=2;
	int i = 20;
	int j = 100000;
	while((i--)>0)
	{
	switch(*krok){

				case 1:

					{
						//prze³¹czenie na krok 2
						krok=2;

						turn_off(FAZA_VDG, FAZA_VDD);
						//GPIO_SetBits(GPIOD, FAZA_WD);
						turn_on(FAZA_WDG,FAZA_WDD);
						//w³¹czenie fazy u(+) i w(-), v(np)
						wait(j);
						j-=2000;
					}
					break;
				case 2:

					{
						//prze³¹czenie na krok 3
						krok=3;
						GPIO_ResetBits(GPIOD, FAZA_UG);
						GPIO_SetBits(GPIOD, FAZA_VG);
						//w³¹czenie fazy u(np), w(-), v(+)
						j-=2000;
					}
					break;
				case 3:

					{
						//prze³¹czenie na krok 4
						krok=4;
						//GPIO_ResetBits(GPIOD, FAZA_WD);
						turn_off(FAZA_WDG,FAZA_WDD);
						//GPIO_SetBits(GPIOD, FAZA_UD);
						turn_on(FAZA_UDG,FAZA_UDD);
						//w³¹czenie fazy u(-), w(np), v(+)
						j-=2000;
					}
					break;
				case 4:

					{
						//prze³¹czenie na krok 5
						krok=5;
						GPIO_ResetBits(GPIOD, FAZA_VG);
						GPIO_SetBits(GPIOD, FAZA_WG);
						//w³¹czenie fazy u(-), w(+), v(np)
						j-=2000;
					}
					break;
				case 5:
					{
						//prze³¹czenie na krok 6
						krok=6;
						//GPIO_ResetBits(GPIOD, FAZA_UD);
						turn_off(FAZA_UDG,FAZA_UDD);
						//GPIO_SetBits(GPIOD, FAZA_VD);
						turn_on(FAZA_VDG,FAZA_VDD);
						//w³¹czenie fazy u(np), w(+), v(-)
						j-=2000;
					}
					break;
				case 6:
					{
						//prze³¹czenie na krok 1
						krok=1;
						GPIO_ResetBits(GPIOD, FAZA_WG);
						GPIO_SetBits(GPIOD, FAZA_UG);
						//w³¹czenie fazy u(+), w(np), v(-)
						j-=2000;
					}
					break;
				}
			}
}



int main(void)
{
	SystemInit();

	SystemCoreClockUpdate();


	PWM_initIO();
	TIMER_t1Init();

	//ADC_initIO();
	ADC_adc1Init();

	DMA2_Init();
	DMA2_Start();
	int value = 100;

	TIM1->CCR1 = (OUT_PWM_PERIOD/12);
	TIM1->CCR2 = (OUT_PWM_PERIOD/12);
	TIM1->CCR3 = (OUT_PWM_PERIOD/12);
	TIM1->CCR4 = (OUT_PWM_PERIOD/12);
	while(1)
	{
		int krok=1; //stan silnika
		int bufor1, bufor2, bufor3, bufor4;
		value=ADC_GetConversionValue(ADC1);
		TIM1->CCR1=value;
		while(1)
		Rozruch(&krok);
		//funkcja startuj¹ca silnik
		/*if((ADC1->SR & 0x00000002)==1)
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
					GPIO_ResetBits(GPIOD, FAZA_VD);
					GPIO_SetBits(GPIOD, FAZA_WD);
					GPIO_SetBits(GPIOD, FAZA_UG);
					//w³¹czenie fazy u(+) i w(-), v(np)
				}
				break;
			case 2:
				if((bufor2 >= bufor4-fi)&&(bufor2 <= bufor4 + fi))
				{
					//prze³¹czenie na krok 3
					krok=3;
					GPIO_ResetBits(GPIOD, FAZA_UG);
					GPIO_SetBits(GPIOD, FAZA_VG);
					//w³¹czenie fazy u(np), w(-), v(+)
				}
				break;
			case 3:
				if((bufor3 >= bufor4-fi)&&(bufor3 <= bufor4 + fi))
				{
					//prze³¹czenie na krok 4
					krok=4;
					GPIO_ResetBits(GPIOD, FAZA_WD);
					GPIO_SetBits(GPIOD, FAZA_UD);
					//w³¹czenie fazy u(-), w(np), v(+)
				}
				break;
			case 4:
				if((bufor1 >= bufor4-fi)&&(bufor1 <= bufor4 + fi)&&krok==1)
				{
					//prze³¹czenie na krok 5
					krok=5;
					GPIO_ResetBits(GPIOD, FAZA_VG);
					GPIO_SetBits(GPIOD, FAZA_WG);
					//w³¹czenie fazy u(-), w(+), v(np)
				}
				break;
			case 5:
				if((bufor2 >= bufor4-fi)&&(bufor2 <= bufor4 + fi))
				{
					//prze³¹czenie na krok 6
					krok=6;
					GPIO_ResetBits(GPIOD, FAZA_UD);
					GPIO_SetBits(GPIOD, FAZA_VD);
					//w³¹czenie fazy u(np), w(+), v(-)
				}
				break;
			case 6:
				if((bufor3 >= bufor4-fi)&&(bufor3 <= bufor4 + fi))
				{
					//prze³¹czenie na krok 1
					krok=1;
					GPIO_ResetBits(GPIOD, FAZA_WG);
					GPIO_SetBits(GPIOD, FAZA_UG);
					//w³¹czenie fazy u(+), w(np), v(-)
				}
				break;
			}
		}*/
			DMA2_Start();
			//funkcja zmieniaj¹ca wype³nienie pwm
	}


	return 1;
}
