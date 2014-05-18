#include <stm32f4xx_conf.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_adc.h>
#include <misc.h>

GPIO_InitTypeDef GPIO_InitStruct;
TIM_TimeBaseInitTypeDef TIM_InitStruct;
TIM_OCInitTypeDef  TIM_OCInitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
ADC_InitTypeDef ADC_init_structure;


int ConvertedValue = 0;
unsigned int counter = 0;
unsigned int arr = 0;
unsigned int Pin = 0;


void RCC_Conf()//Trzeba poprawiæ
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOCEN,ENABLE);//Clock for the ADC port!! Do not forget about this one ;)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); // 9Mhz

	        // Wlacz taktowanie GPIOD iGPIOE i GPIOC i TIM2 i GPIOA
	        RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOC , ENABLE);
	       // Wlacz taktowanie TIM2
	        RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2 , ENABLE);

	        //DMA
	        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);


}

void TIM_Conf()
{
	TIM_TimeBaseStructInit(&TIM_InitStruct);
	TIM_InitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_InitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period=50000;
	TIM_InitStruct.TIM_Prescaler=1;
	TIM_TimeBaseInit(TIM1, &TIM_InitStruct);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_Cmd(TIM1,ENABLE);

	TIM_OCStructInit( &TIM_OCInitStructure );
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM1->CCR1 = 50000;
	TIM1->CCR2 = 50000;
	TIM1->CCR3 = 50000;
	TIM1->CCR4 = 50000;
}

void NVIC_Conf()
{

	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
}

void GPIO_Conf()
{
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 ;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_TIM1);

	//ADC GPIO conf
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5  ;//trzeba sprawdziæ czy s¹ to odpowiednie piny
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	   GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void ADC_Conf(void)
{
   ADC_InitTypeDef ADC_InitStructure;

   ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
   ADC_InitStructure.ADC_ScanConvMode = ENABLE;
   ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_NbrOfChannel = 6;
   ADC_Init(ADC1, &ADC_InitStructure);

   ADC_RegularChannelConfig(ADC1, ADC_Channel_10,1, ADC_SampleTime_71Cycles5);
   ADC_RegularChannelConfig(ADC1, ADC_Channel_11,2, ADC_SampleTime_71Cycles5);
   ADC_RegularChannelConfig(ADC1, ADC_Channel_12,3, ADC_SampleTime_71Cycles5);
   ADC_RegularChannelConfig(ADC1, ADC_Channel_13,4, ADC_SampleTime_71Cycles5);
   ADC_RegularChannelConfig(ADC1, ADC_Channel_14,5, ADC_SampleTime_71Cycles5);
   ADC_RegularChannelConfig(ADC1, ADC_Channel_15,6, ADC_SampleTime_71Cycles5);

   ADC_DMACmd(ADC1, ENABLE);
   ADC_Cmd(ADC1, ENABLE);

   // kalibracja
   ADC_ResetCalibration(ADC1);
   while(ADC_GetResetCalibrationStatus(ADC1));

   ADC_StartCalibration(ADC1);
   while(ADC_GetCalibrationStatus(ADC1));

   ADC_SoftwareStartConvCmd(ADC1,ENABLE);

}

void DMA_Conf(void)
{
   DMA_InitTypeDef DMA_InitStruct;

   DMA_DeInit(DMA1_Channel1);
   DMA_InitStruct.DMA_PeripheralBaseAddr = ADC1_DR_Address;
   DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&ADCVal;

   // kierunek: zrod³o to ADC
   DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;

   // rozmiar buffora u nas 6
   DMA_InitStruct.DMA_BufferSize = 6;

   DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
   DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

   //ciag³e przesy³anie danych
   DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;

   DMA_InitStruct.DMA_Priority = DMA_Priority_High;
   DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

   DMA_Init(DMA1_Channel1, &DMA_InitStruct);

   //wlacz DMA
   DMA_Cmd(DMA1_Channel1, ENABLE);


}


int adc_convert(){
 ADC_SoftwareStartConv(ADC1);//Start the conversion
 while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));//Processing the conversion
 return ADC_GetConversionValue(ADC1); //Return the converted data
}

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();
	RCC_Conf();
	TIM_Conf();
	GPIO_Conf();
	ADC_Conf();
    while(1)
    {
    	ConvertedValue = adc_convert();//Read the ADC converted value
    }
}
