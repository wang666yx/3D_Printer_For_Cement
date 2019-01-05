#include "bsp_timer.h"

TIM_HandleTypeDef tim3_handler;
TIM_HandleTypeDef tim4_handler;


void bsp_timer3_init(u16 arr,u16 psc)
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	
	tim3_handler.Instance=TIM3;
  tim3_handler.Init.Prescaler=psc;                     //��Ƶ
  tim3_handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
  tim3_handler.Init.Period=arr;                        //�Զ�װ��ֵ
  tim3_handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
  HAL_TIM_Base_Init(&tim3_handler);
	
	HAL_TIM_Base_Start_IT(&tim3_handler);  
}


void bsp_timer4_init(u16 arr,u16 psc)
{
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	tim4_handler.Instance=TIM4;
  tim4_handler.Init.Prescaler=psc;                     //��Ƶ
  tim4_handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
  tim4_handler.Init.Period=arr;                        //�Զ�װ��ֵ
  tim4_handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
  HAL_TIM_Base_Init(&tim4_handler);
	
	HAL_TIM_Base_Stop_IT(&tim4_handler);  
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM3)
	{           
		HAL_NVIC_SetPriority(TIM3_IRQn,4,0);    
		HAL_NVIC_EnableIRQ(TIM3_IRQn);            
	}
  if(htim->Instance==TIM4)
	{           
		HAL_NVIC_SetPriority(TIM4_IRQn,4,0);    
		HAL_NVIC_EnableIRQ(TIM4_IRQn);            
	}
}

void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&tim3_handler);
}

void TIM4_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&tim4_handler);
}


void bsp_timer3_int(void)
{
	static u8 timer_once;
	if(timer_once==0)
	{
		timer_once=1;
	}
	else
	{
		
	}
}

extern void jmp_gui_uart_timeout(void);

void bsp_timer4_int(void)
{
	static u8 timer_once;
	if(timer_once==0)
	{
		timer_once=1;
	}
	else
	{
		jmp_gui_uart_timeout();
	}
}

//�ص���������ʱ���жϷ���������
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==(&tim3_handler))
	{
		bsp_timer3_int();
	}
	if(htim==(&tim4_handler))
	{
		bsp_timer4_int();
	}
}





