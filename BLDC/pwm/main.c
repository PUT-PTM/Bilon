




#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"


void initLEDs() {


}
/*
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
*/

void TIM1_Conf()
{
	RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_TIM1EN;
	TIM1->CR1 = (u_int16)0x000; //[15:10] -reserved | [9:8] -ckd | [7] -ARPE | [6:5] -CMS | [5] -DIR |
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
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = &TIM_Output_Compare_Idle_State;
    TIM_OCInitStructure.TIM_OCNIdleState= &TIM_Output_Compare_Idle_State;
    TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_Low;
    TIM_OCInitStructure.TIM_OutputNState;

    TIM_OC2Init (TIM1 , &TIM_OCInitStructure);
    TIM_OC2PreloadConfig (TIM1, TIM_OCPreload_Enable );




    while (1) {
        TIM1->CCR2 = 25;

    }

}

void Delay(__IO uint32_t nCount)
{
  while(nCount--)
  {
  }
}

