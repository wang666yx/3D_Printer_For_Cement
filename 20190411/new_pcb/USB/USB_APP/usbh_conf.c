#include "stm32h7xx_hal.h"
#include "usbh_conf.h"
#include "usbh_core.h"
#include "tmdelay.h"

HCD_HandleTypeDef hhcd;

//��ʼ��PCD MSP
//hpcd:PCD�ṹ��ָ��
//����ֵ:��
void HAL_HCD_MspInit(HCD_HandleTypeDef * hhcd)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef USBClkInit;

    //USBʱ�����ã�ʹ��HSI48MHz
    USBClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    USBClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    HAL_RCCEx_PeriphCLKConfig(&USBClkInit);
    
    if(hhcd->Instance == USB2_OTG_FS)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        __HAL_RCC_USB2_OTG_FS_CLK_ENABLE();
        HAL_NVIC_SetPriority(OTG_FS_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    }
    else if (hhcd->Instance == USB1_OTG_HS)
    {

    }
}

//USB OTG �жϷ�����
//��������USB�ж�
void OTG_FS_IRQHandler(void)
{
    HAL_HCD_IRQHandler(&hhcd);
}

///////////////////////USBH LL HCD�����ص�����(HCD->USB Host Library)///////////////////////
  
//USBH SOF�ص�����
//hhcd:HCD�ṹ��ָ��
//����ֵ:��
void HAL_HCD_SOF_Callback(HCD_HandleTypeDef * hhcd)
{
    USBH_LL_IncTimer(hhcd->pData);
}

//USBH ���ӳɹ��ص�����
//hhcd:HCD�ṹ��ָ��
//����ֵ:��
void HAL_HCD_Connect_Callback(HCD_HandleTypeDef * hhcd)
{
    printf("Connected!\r\n\r\n");	
    USBH_LL_Connect(hhcd->pData);
}

//USBH �Ͽ����ӻص�����
//hhcd:HCD�ṹ��ָ��
//����ֵ:��
void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef * hhcd)
{
    printf("Disconnected!\r\n\r\n");
    USBH_LL_Disconnect(hhcd->pData);
}


//USBH ֪ͨURB�仯�ص�����
//hhcd:HCD�ṹ��ָ��
//chnum:�˵���
//urb_state:URB״̬
//����ֵ:��
void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef * hhcd,
                                         uint8_t chnum,
                                         HCD_URBStateTypeDef urb_state)
{
  /* To be used with OS to sync URB state with the global state machine */
}

/////////////////////USBH LL �����ӿں���(USB Host Library --> HCD)/////////////////////
  
//USBH�ײ��ʼ������
//phost:USBH���ָ��
//����ֵ:0,����
//      1,æ
//    ����,ʧ��
USBH_StatusTypeDef USBH_LL_Init(USBH_HandleTypeDef * phost)
{
#ifdef USE_USB_FS
    //����LL������ز���
    hhcd.Instance = USB2_OTG_FS;                //ʹ��USB2 OTG
    hhcd.Init.Host_channels = 11;               //����ͨ����Ϊ11��
    hhcd.Init.dma_enable = 0;                   //��ʹ��DMA
    hhcd.Init.low_power_enable = 0;             //��ʹ�ܵ͹���ģʽ
    hhcd.Init.phy_itface = HCD_PHY_EMBEDDED;    //ʹ���ڲ�PHY
    hhcd.Init.Sof_enable = 0;                   //��ֹSOF�ж�
    hhcd.Init.speed = HCD_SPEED_FULL;           //USBȫ��(12Mbps)
    hhcd.Init.vbus_sensing_enable = 0;          //��ʹ��VBUS���
    hhcd.Init.lpm_enable = 0;                   //ʹ�����ӵ�Դ����           

    hhcd.pData = phost;                         //hhcd��pDataָ��phost
    phost->pData = &hhcd;                       //phost��pDataָ��hhcd 

    HAL_HCD_Init(&hhcd);                        //��ʼ��LL����
#endif

#ifdef USE_USB_HS
    //δʵ��
#endif                          
    USBH_LL_SetTimer(phost, HAL_HCD_GetCurrentFrame(&hhcd));
    return USBH_OK;
}

//USBD�ײ�ȡ����ʼ��(�ָ�Ĭ�ϸ�λ״̬)���� 
//phost:USBH���ָ��
//����ֵ:0,����
//      1,æ
//   ����,ʧ��
USBH_StatusTypeDef USBH_LL_DeInit(USBH_HandleTypeDef * phost)
{
    HAL_HCD_DeInit(phost->pData);
    return USBH_OK;
}

//USBH�ײ�������ʼ���� 
//phost:USBH���ָ��
//����ֵ:0,����
//      1,æ
//   ����,ʧ��
USBH_StatusTypeDef USBH_LL_Start(USBH_HandleTypeDef * phost)
{
    HAL_HCD_Start(phost->pData);
    return USBH_OK;
}

//USBH�ײ�����ֹͣ���� 
//phost:USBH���ָ��
//����ֵ:0,����
//      1,æ
//   ����,ʧ��
USBH_StatusTypeDef USBH_LL_Stop(USBH_HandleTypeDef * phost)
{
    HAL_HCD_Stop(phost->pData);
    return USBH_OK;
}

//USBH��ȡUSB�豸���ٶ� 
//phost:USBH���ָ��
//����ֵ:USBH�豸�ٶ�
USBH_SpeedTypeDef USBH_LL_GetSpeed(USBH_HandleTypeDef * phost)
{
    USBH_SpeedTypeDef speed = USBH_SPEED_FULL;

    switch (HAL_HCD_GetCurrentSpeed(phost->pData))
    {
        case 0:
            speed = USBH_SPEED_HIGH;
            printf("USB Host [HS]\r\n");
            break;
        case 1:
            speed = USBH_SPEED_FULL;
            printf("USB Host [FS]\r\n");
            break;
        case 2:
            speed = USBH_SPEED_LOW;
            printf("USB Host [LS]\r\n");
            break;
        default:
            speed = USBH_SPEED_FULL;
            printf("USB Host [FS]\r\n");
            break;
    }
    return speed;
}

//USBH��λUSB HOST�˿� 
//phost:USBH���ָ��
//����ֵ:0,����
//      1,æ
//   ����,ʧ��
USBH_StatusTypeDef USBH_LL_ResetPort(USBH_HandleTypeDef * phost)
{
    HAL_HCD_ResetPort(phost->pData);
    printf("USB Reset Port\r\n");
    return USBH_OK;
}

//USBH��ȡ���һ�δ���İ���С
//phost:USBH���ָ��
//pipe:�ܵ����
//����ֵ:����С
uint32_t USBH_LL_GetLastXferSize(USBH_HandleTypeDef * phost, uint8_t pipe)
{
    return HAL_HCD_HC_GetXferCount(phost->pData, pipe);
}

//USBH��һ���ܵ�(ͨ��)
//phost:USBH���ָ��
//pipe:�ܵ����
//epnum:�˵��
//dev_address:�豸��ַ
//speed:�豸�ٶ�
//ep_type:�˵�����
//mps:�˵�������С
//����ֵ:0,����
//      1,æ
//      2,ʧ��
USBH_StatusTypeDef USBH_LL_OpenPipe(USBH_HandleTypeDef * phost,
                                    uint8_t pipe,
                                    uint8_t epnum,
                                    uint8_t dev_address,
                                    uint8_t speed,
                                    uint8_t ep_type, uint16_t mps)
{
    HAL_HCD_HC_Init(phost->pData, pipe, epnum, dev_address, speed, ep_type, mps);
    return USBH_OK;
}

//USBH�ر�һ���ܵ�(ͨ��)
//phost:USBH���ָ��
//pipe:�ܵ����
//����ֵ:0,����
//      1,æ
//      2,ʧ��
USBH_StatusTypeDef USBH_LL_ClosePipe(USBH_HandleTypeDef * phost, uint8_t pipe)
{
    HAL_HCD_HC_Halt(phost->pData, pipe);
    return USBH_OK;
}

//USBH�ύһ���µ�URB
//phost:USBH���ָ��
//pipe:�ܵ����
//direction:��������  
//ep_type:�˵�����
//token:�˵��־
//pbuff:URB�������׵�ַ
//length:URB���ݳ���
//do_ping:����do ping protocol,USB HS���õ�
//����ֵ:0,����
//      1,æ
//      2,ʧ��
USBH_StatusTypeDef USBH_LL_SubmitURB(USBH_HandleTypeDef * phost,
                                     uint8_t pipe,
                                     uint8_t direction,
                                     uint8_t ep_type,
                                     uint8_t token,
                                     uint8_t * pbuff,
                                     uint16_t length, uint8_t do_ping)
{
    HAL_HCD_HC_SubmitRequest(phost->pData,
                           pipe,
                           direction, ep_type, token, pbuff, length, do_ping);
    return USBH_OK;
}

//USBH��ȡURB״̬
//phost:USBH���ָ��
//pipe:�ܵ����
//����ֵ:URB״̬
USBH_URBStateTypeDef USBH_LL_GetURBState(USBH_HandleTypeDef * phost,
                                         uint8_t pipe)
{
    return (USBH_URBStateTypeDef) HAL_HCD_HC_GetURBState(phost->pData, pipe);
}

//USBH����VBUS״̬
//phost:USBH���ָ��
//state:vbus״̬.0,����;1,������
//����ֵ:0,����
//      1,æ
//      2,ʧ�� 
USBH_StatusTypeDef USBH_LL_DriverVBUS(USBH_HandleTypeDef * phost, uint8_t state)
{
#ifdef USE_USB_FS
    if (state == 0)
    {
    }
    else
    {
    }
#endif
    tmdelay_ms(500);
    return USBH_OK;
}

//USBH���ùܵ��ķ�ת
//phost:USBH���ָ��
//pipe:�ܵ����
//toggle:��ת״̬.
//����ֵ:0,����
//      1,æ
//      2,ʧ�� 
USBH_StatusTypeDef USBH_LL_SetToggle(USBH_HandleTypeDef * phost, uint8_t pipe,
                                     uint8_t toggle)
{
    if (hhcd.hc[pipe].ep_is_in)
    {
        hhcd.hc[pipe].toggle_in = toggle;
    }
    else
    {
        hhcd.hc[pipe].toggle_out = toggle;
    }
    return USBH_OK;
}

//USBH��ȡ�ܵ���ת״̬
//phost:USBH���ָ��
//pipe:�ܵ���� 
//����ֵ:��ת״̬
uint8_t USBH_LL_GetToggle(USBH_HandleTypeDef * phost, uint8_t pipe)
{
    uint8_t toggle = 0;

    if (hhcd.hc[pipe].ep_is_in)
    {
        toggle = hhcd.hc[pipe].toggle_in;
    }
    else
    {
        toggle = hhcd.hc[pipe].toggle_out;
    }
    return toggle;
}

//USBH ��ʱ����(��msΪ��λ)
//Delay:��ʱ��ms��.
void USBH_Delay(uint32_t Delay)
{
    tmdelay_ms(Delay);
}

