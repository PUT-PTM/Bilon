#include <stm32f4xx_conf.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_tim.h>
#include <misc.h>

GPIO_InitTypeDef GPIO_InitStruct;
TIM_TimeBaseInitTypeDef TIM_InitStruct;
TIM_OCInitTypeDef  TIM_OCInitStructure;
NVIC_InitTypeDef NVIC_InitStructure;


unsigned int counter = 0;
unsigned int arr = 0;
unsigned int Pin = 0;
void RCC_Conf()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	//RCC_APB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
}

void TIM_Conf()
{
	TIM_TimeBaseStructInit(&TIM_InitStruct);
	TIM_InitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_InitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period=65535;
	TIM_InitStruct.TIM_Prescaler=7200;
	TIM_TimeBaseInit(TIM1, &TIM_InitStruct);

	TIM_OCStructInit( &TIM_OCInitStructure );
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_Cmd(TIM1,ENABLE);


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
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);
}


int main(void)
{
	SystemInit();
	RCC_Conf();
	TIM_Conf();
	NVIC_Conf();
	GPIO_Conf();
	counter=1;
	int z = 0;
    while(1)
    {
    	counter=TIM1->CNT;
    	arr=TIM1->ARR;
    	Pin=GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_9);

    }
}


/*
DZIA£A

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"


void initLEDs() {


}

void TIM3_IRQHandler()
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        //TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        GPIO_SetBits(GPIOA, GPIO_Pin_9);
        Delay(50000000);
        GPIO_ResetBits(GPIOA, GPIO_Pin_9);
        //TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        //GPIO_ToggleBits(GPIOA, GPIO_Pin_9);
    }
}

void TIM1_IRQHandler()
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
        GPIO_ToggleBits(GPIOA, GPIO_Pin_13);
    }
}

int main(void)
{
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemInit();
    SystemCoreClockUpdate();


    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 100;
    TIM_TimeBaseStructure.TIM_Prescaler = 8399;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    //TIM_Cmd(TIM3, ENABLE);
    TIM_Cmd(TIM1, ENABLE);

    TIM_OCInitTypeDef TIM_OCInitStructure;

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable ;
    TIM_OCInitStructure.TIM_Pulse = 0 ;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High ;

    TIM_OC2Init (TIM1 , &TIM_OCInitStructure);
    TIM_OC2PreloadConfig (TIM1, TIM_OCPreload_Enable );



    TIM1->CCR2 = 10;

    while (1) {
        TIM1->CCR2 = 25;
        Delay(10000000);
        TIM1->CCR2 = 50;
        Delay(10000000);
        TIM1->CCR2 = 75;
        Delay(10000000);
        TIM1->CCR2 = 100;
        Delay(10000000);
    }

}

void Delay(__IO uint32_t nCount)
{
  while(nCount--)
  {
  }
}
 */
