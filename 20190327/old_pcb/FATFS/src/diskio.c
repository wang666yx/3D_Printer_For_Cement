/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include "diskio.h"			/* FatFs lower layer API */
#include "w25qxx.h"
#include <RTL.h>
#include <rl_usb.h>

#define SPI_FLASH 	0
#define USB_DISK	1
#define SD_CARD	 	2

 
#define FLASH_SECTOR_SIZE 	512	
#define FLASH_SECTOR_COUNT 	1024*8*2
#define FLASH_BLOCK_SIZE   	8	
  
 
//��ô���״̬
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{ 
	return RES_OK;
}  
//��ʼ������
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	u8 res=0;	    
	switch(pdrv)
	{
		case SPI_FLASH:		//�ⲿflash
		{
			w25qxx_init();  //W25QXX��ʼ��
 			break;
		}
		case USB_DISK:
		{
			//usbh_init(0);
			break;
		}
		default:
			res=1; 
	}		 
	if(res)
	{
		return  STA_NOINIT;
	}
	else 
	{
		return 0; //��ʼ���ɹ� 
	}
} 
//������
//pdrv:���̱��0~9
//*buff:���ݽ��ջ����׵�ַ
//sector:������ַ
//count:��Ҫ��ȡ��������
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	u8 res=0; 
  if (!count)
	{
		return RES_PARERR;//count���ܵ���0�����򷵻ز�������		 	 
	}
	switch(pdrv)
	{
		case SPI_FLASH://�ⲿflash
		{
			for(;count>0;count--)
			{
				w25qxx_read(buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
				sector++;
				buff+=FLASH_SECTOR_SIZE;
			}
			res=0;
			break;
		}
		case USB_DISK:
		{
			usbh_msc_read(0,0,sector,buff,count);
			break;
		}
		default:
			res=1; 
	}
	if(res==0x00)
	{
		return RES_OK;
	}			
	else 
	{
		return RES_ERROR;
	}			
}
//д����
//pdrv:���̱��0~9
//*buff:���������׵�ַ
//sector:������ַ
//count:��Ҫд��������� 
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	u8 res=0;  
  if (!count)
	{
		return RES_PARERR;//count���ܵ���0�����򷵻ز�������		
	}		
	switch(pdrv)
	{
		case SPI_FLASH://�ⲿflash
		{
			for(;count>0;count--)
			{										    
				w25qxx_write((u8*)buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
				sector++;
				buff+=FLASH_SECTOR_SIZE;
			}
			res=0;
			break;
		}
		case USB_DISK:
		{
			usbh_msc_write(0,0,sector,(U8*)buff,count);
			break;
		}
		default:
			res=1; 
	}
	if(res==0x00)
	{
		return RES_OK;
	}			
	else 
	{
		return RES_ERROR;
	}				
} 
//����������Ļ��
//pdrv:���̱��0~9
//ctrl:���ƴ���
//*buff:����/���ջ�����ָ�� 
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;						  			     
	
	if(pdrv==SPI_FLASH)	//�ⲿFLASH  
	{
	    switch(cmd)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = FLASH_SECTOR_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = FLASH_BLOCK_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = FLASH_SECTOR_COUNT;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}
	else if(pdrv==USB_DISK)
	{
		u32 blk_num;
		u32 blk_sz;
		usbh_msc_read_config(0,0,&blk_num,&blk_sz);
		switch(cmd)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = blk_sz;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = blk_sz;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = blk_num;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}
	else 
	{
		res=RES_ERROR;//�����Ĳ�֧��
	}
    return res;
} 
//���ʱ��
//User defined function to give a current time to fatfs module      */
//31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */                                                                                                                                                                                                                                          
//15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */                                                                                                                                                                                                                                                
DWORD get_fattime (void)
{				 
	return 0;
}			 










