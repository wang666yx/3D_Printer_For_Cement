#ifndef __USBH_DISKIO_H
#define __USBH_DISKIO_H
#include "usbh_core.h"
#include "usbh_msc.h"
#include "diskio.h"	
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

extern USBH_HandleTypeDef  hUSBHost;

#define USB_DEFAULT_BLOCK_SIZE 512

DSTATUS USBH_initialize(void);
DSTATUS USBH_status(void);
DRESULT USBH_read(BYTE *buff, DWORD sector, UINT count);
DRESULT USBH_write(const BYTE *buff,DWORD sector,UINT count);
DRESULT USBH_ioctl(BYTE cmd,void *buff);
#endif 
