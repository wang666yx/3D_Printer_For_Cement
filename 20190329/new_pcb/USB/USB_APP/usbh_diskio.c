#include "usbh_diskio.h"
#include "ff.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//USBH-DISKIO ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2018/7/19
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   

USBH_HandleTypeDef  hUSBHost;

//��ʼ��USBH
DSTATUS USBH_initialize(void)
{
    return RES_OK;
}

//��ȡU��״̬
DSTATUS USBH_status(void)
{
    DRESULT res=RES_ERROR;
    MSC_HandleTypeDef *MSC_Handle=hUSBHost.pActiveClass->pData;

    if(USBH_MSC_UnitIsReady(&hUSBHost,MSC_Handle->current_lun))
    {
        printf("U��״̬��ѯ�ɹ�\r\n");
        res=RES_OK;
    }
    else
    {
        printf("U��״̬��ѯʧ��\r\n");
        res=RES_ERROR;
    }
    return res;
}

//U�̶���������
//lun��lun ID
//buff: ���ݻ�����
//sector: Ҫ��ȡ�������׵�ַ
//count��Ҫ��ȡ����������
DRESULT USBH_read(BYTE *buff, DWORD sector, UINT count)
{
    DRESULT res = RES_ERROR;
    MSC_LUNTypeDef info;
    MSC_HandleTypeDef *MSC_Handle=hUSBHost.pActiveClass->pData;
    
    if(USBH_MSC_Read(&hUSBHost,MSC_Handle->current_lun,sector,buff,count)==USBH_OK)
    {
        res = RES_OK;
    }
    else
    {
        printf("U�̶�ȡʧ��\r\n");
        USBH_MSC_GetLUNInfo(&hUSBHost,MSC_Handle->current_lun, &info);
        switch (info.sense.asc)
        {
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
                USBH_ErrLog("USB Disk is not ready!");
                res = RES_NOTRDY;
                break;
            default:
                res = RES_ERROR;
                break;
        }
    }
    return res;
}

//U��д��������
//lun��lun ID
//buff: ���ݻ�����
//sector: Ҫд��������׵�ַ
//count��Ҫд�����������
DRESULT USBH_write(const BYTE *buff,DWORD sector,UINT count)
{
    DRESULT res = RES_ERROR;
    MSC_LUNTypeDef info;
    MSC_HandleTypeDef *MSC_Handle=hUSBHost.pActiveClass->pData;
    if(USBH_MSC_Write(&hUSBHost,MSC_Handle->current_lun,sector,(BYTE *)buff,count) == USBH_OK)
    {
        res=RES_OK;
    }
    else
    {
        printf("U��д��ʧ��\r\n");
        USBH_MSC_GetLUNInfo(&hUSBHost,MSC_Handle->current_lun, &info);
        switch (info.sense.asc)
        {
            case SCSI_ASC_WRITE_PROTECTED:
                USBH_ErrLog("USB Disk is Write protected!");
                res = RES_WRPRT;
                break;
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
                USBH_ErrLog("USB Disk is not ready!");
                res = RES_NOTRDY;
                break;
            default:
                res = RES_ERROR;
                break;
        }
    }
    return res;
}

//U��IO���Ʋ���
//lun��lun ID
//cmd: ��������
//buff: ��������
DRESULT USBH_ioctl(BYTE cmd,void *buff)
{
    DRESULT res = RES_ERROR;
    MSC_LUNTypeDef info;
    MSC_HandleTypeDef *MSC_Handle=hUSBHost.pActiveClass->pData;
    switch(cmd)
    {
        case CTRL_SYNC:
            res=RES_OK;
            break;
        case GET_SECTOR_COUNT : //��ȡ��������
            if(USBH_MSC_GetLUNInfo(&hUSBHost,MSC_Handle->current_lun,&info)==USBH_OK)
            {
                *(DWORD*)buff=info.capacity.block_nbr;
                res = RES_OK;
                printf("��������:%d\r\n",info.capacity.block_nbr);
            }
            else
            {
                res = RES_ERROR;
            }
            break;
        case GET_SECTOR_SIZE ://��ȡ������С
            if(USBH_MSC_GetLUNInfo(&hUSBHost,MSC_Handle->current_lun,&info) == USBH_OK)
            {
                *(DWORD*)buff=info.capacity.block_size;
                res = RES_OK;
                printf("������С:%d\r\n",info.capacity.block_size);
            }
            else
            {
                res = RES_ERROR;
            }
            break;
        case GET_BLOCK_SIZE :   //��ȡһ���������������Ĵ�С��

            if(USBH_MSC_GetLUNInfo(&hUSBHost,MSC_Handle->current_lun,&info)==USBH_OK)
            {
                *(DWORD*)buff=info.capacity.block_size/USB_DEFAULT_BLOCK_SIZE;
                printf("ÿ������������:%d\r\n",info.capacity.block_size/USB_DEFAULT_BLOCK_SIZE);
                res = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            }
            break;
        default:
                res = RES_PARERR;
    }
    return res;
}
