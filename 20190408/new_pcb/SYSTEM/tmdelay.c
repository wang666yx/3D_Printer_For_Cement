#include "tmdelay.h"
#include "tmsys.h"

/***********************************************************************************************************
���ļ�Ϊϵͳ�ײ�BSP����ʱģ�飬����һ����ʱ����Ϊ��ʱ�Ļ�׼���ṩ�����΢�����ʱ
***********************************************************************************************************/




TIM_HandleTypeDef tim7_handler;

void tmdelay_init(void)
{
	__HAL_RCC_TIM7_CLK_ENABLE();
	
	tim7_handler.Instance=TIM7;
  tim7_handler.Init.Prescaler=199;                     //��Ƶ
  tim7_handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
  tim7_handler.Init.Period=999;                        //�Զ�װ��ֵ
  tim7_handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
  HAL_TIM_Base_Init(&tim7_handler);
	
	
	HAL_TIM_Base_Start(&tim7_handler);
}								    

//��ʱnus
//nusΪҪ��ʱ��us��.	
void tmdelay_us(u32 nus)
{		
	u32 told,tnow,tcnt=0;
	u32 reload;
	reload=TIM7->ARR;				//LOAD��ֵ	    	 
	told=TIM7->CNT;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=TIM7->CNT;	
		if(tnow!=told)
		{	    
			if(tnow>told)
			{
				tcnt+=tnow-told;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			}
			else 
			{
				tcnt+=reload+tnow-told;
			}				
			told=tnow;
			if(tcnt>=nus)
			{
				break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
			}
		}  
	};
}
 
//��ʱnms
//nms:Ҫ��ʱ��ms��
void tmdelay_ms(u16 nms)
{
	u32 i;
	for(i=0;i<nms;i++) 
	{
		tmdelay_us(1000);
	}
}

			 



































