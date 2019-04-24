#include "bsp_exti.h"
#include "bsp_qspi.h"





void bsp_exti_init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();               //����GPIOBʱ��
	
	GPIO_Initure.Pin=GPIO_PIN_14;               //PC13
	GPIO_Initure.Mode=GPIO_MODE_IT_RISING;     //�½��ش���
	GPIO_Initure.Pull=GPIO_PULLUP;				//����
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
	
	HAL_NVIC_SetPriority(EXTI15_10_IRQn,3,0);   //��ռ���ȼ�Ϊ3�������ȼ�Ϊ3
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);         //ʹ���ж���13  
}


void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);  //�����жϴ����ú���
}


extern void jmp_breakpoint_int(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
		case GPIO_PIN_14:
		{
			jmp_breakpoint_int();
			__set_FAULTMASK(1);
			NVIC_SystemReset();
			break;
		}
	}
}

