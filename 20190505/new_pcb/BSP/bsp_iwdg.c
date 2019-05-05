#include "bsp_iwdg.h"




IWDG_HandleTypeDef iwdg_handler; //�������Ź����

//��ʼ���������Ź�
//prer:��Ƶ��:0~7(ֻ�е�3λ��Ч!)
//rlr:�Զ���װ��ֵ,0~0XFFF.
//��Ƶ����=4*2^prer.�����ֵֻ����256!
//rlr:��װ�ؼĴ���ֵ:��11λ��Ч.
//ʱ�����(���):Tout=((4*2^prer)*rlr)/32 (ms).
void bsp_iwdg_init(u8 prer,u16 rlr)
{
    iwdg_handler.Instance=IWDG1;
    iwdg_handler.Init.Prescaler=prer;    //����IWDG��Ƶϵ��
    iwdg_handler.Init.Reload=rlr;        //��װ��
    iwdg_handler.Init.Window=IWDG_WINDOW_DISABLE;//�رմ��ڹ���
    HAL_IWDG_Init(&iwdg_handler);    
}
    
//ι�������Ź�
void bsp_iwdg_feed(void)
{   
    HAL_IWDG_Refresh(&iwdg_handler); //��װ��
}


