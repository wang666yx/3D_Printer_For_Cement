/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include "diskio.h"			/* FatFs lower layer API */

#include "usbh_diskio.h" 
#include "w25qxx.h"
#include "tmsys.h"



#define EX_FLASH 	0			
#define USB_DISK	1			
#define SD_CARD	 	2  			

//����W25Q256
//ǰ25M�ֽڸ�fatfs��,25M�ֽں�,���ڴ���ֿ�,�ֿ�ռ��6.01M.	ʣ�ಿ��,���ͻ��Լ���	 
#define SPI_FLASH_SECTOR_SIZE 	512	
#define SPI_FLASH_SECTOR_COUNT 	1024*25*2	//W25Q256,ǰ25M�ֽڸ�FATFSռ��	
#define SPI_FLASH_BLOCK_SIZE   	8     		//ÿ��BLOCK��8������		
  
  
  
//��ô���״̬
DSTATUS disk_status (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
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
		case EX_FLASH:		//�ⲿflash
			W25QXX_Init();
 			break;
		case USB_DISK:		//U��  
			res=USBH_initialize();//U�̳�ʼ��
			break;
		default:
			res=1; 
	}		 
	if(res)return  STA_NOINIT;
	else return 0; //��ʼ���ɹ� 
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
    if (!count)return RES_PARERR;//count���ܵ���0�����򷵻ز�������		 	 
	switch(pdrv)
	{
		case EX_FLASH://�ⲿflash
			for(;count>0;count--)
			{
				W25QXX_Read(buff,sector*SPI_FLASH_SECTOR_SIZE,SPI_FLASH_SECTOR_SIZE);
				sector++;
				buff+=SPI_FLASH_SECTOR_SIZE;
			}
			res=0;
			break;
    case USB_DISK:	//U��    
			res=USBH_read(buff,sector,count);  	
			break;
		default:
			res=1; 
	}
   //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
    if(res==0x00)return RES_OK;	 
    else return RES_ERROR;	   
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
    if (!count)return RES_PARERR;//count���ܵ���0�����򷵻ز�������		 	 
	switch(pdrv)
	{
		case EX_FLASH://�ⲿflash
			for(;count>0;count--)
			{										    
				W25QXX_Write((u8*)buff,sector*SPI_FLASH_SECTOR_SIZE,SPI_FLASH_SECTOR_SIZE);
				sector++;
				buff+=SPI_FLASH_SECTOR_SIZE;
			}
			res=0;
			break;
    case USB_DISK:	    //U��
      res=USBH_write(buff,sector,count);
      break;
		default:
			res=1; 
	}
    //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
    if(res == 0x00)return RES_OK;	 
    else return RES_ERROR;	
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
	
	if(pdrv==EX_FLASH)	//�ⲿFLASH  
	{
	    switch(cmd)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = SPI_FLASH_SECTOR_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = SPI_FLASH_BLOCK_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = SPI_FLASH_SECTOR_COUNT;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}else if(pdrv==USB_DISK)	//U��
    {
        res=USBH_ioctl(cmd,buff);
    }else res=RES_ERROR;//�����Ĳ�֧��
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


















