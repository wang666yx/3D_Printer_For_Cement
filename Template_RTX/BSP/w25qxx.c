#include "w25qxx.h"

u16 W25QXX_TYPE=W25Q32;


void w25qxx_init(void)
{ 
	u8 temp;
	GPIO_InitTypeDef GPIO_Initure;

	BSP_W25QXX_CS_GPIO_CLK_ENABLE();   

	GPIO_Initure.Pin=BSP_W25QXX_CS_GPIO_PIN;
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
	GPIO_Initure.Pull=GPIO_PULLUP;          //����
	GPIO_Initure.Speed=GPIO_SPEED_FAST;     //����         
	HAL_GPIO_Init(BSP_W25QXX_CS_GPIO,&GPIO_Initure);     //��ʼ��

	W25QXX_CS=1;			                //SPI FLASH��ѡ��
	bsp_spi1_init();		   			        //��ʼ��SPI
	bsp_spi1_set_speed(SPI_BAUDRATEPRESCALER_2); //����Ϊ45Mʱ��,����ģʽ
	W25QXX_TYPE=w25qxx_readID();	        //��ȡFLASH ID.
	if(W25QXX_TYPE==W25Q256)                //SPI FLASHΪW25Q256
	{
		temp=w25qxx_readSR(3);              //��ȡ״̬�Ĵ���3���жϵ�ַģʽ
		if((temp&0X01)==0)			        //�������4�ֽڵ�ַģʽ,�����4�ֽڵ�ַģʽ
		{
			W25QXX_CS=0; 			        //ѡ��
			bsp_spi1_rw_byte(W25X_Enable4ByteAddr);//���ͽ���4�ֽڵ�ַģʽָ��   
			W25QXX_CS=1;       		        //ȡ��Ƭѡ   
		}
	}
}

u8 w25qxx_readSR(u8 regno)   
{  
	u8 byte=0,command=0; 
	switch(regno)
	{
		case 1:
			command=W25X_ReadStatusReg1;    //��״̬�Ĵ���1ָ��
			break;
		case 2:
			command=W25X_ReadStatusReg2;    //��״̬�Ĵ���2ָ��
			break;
		case 3:
			command=W25X_ReadStatusReg3;    //��״̬�Ĵ���3ָ��
			break;
		default:
			command=W25X_ReadStatusReg1;    
			break;
	}    
	W25QXX_CS=0;                            //ʹ������   
	bsp_spi1_rw_byte(command);            //���Ͷ�ȡ״̬�Ĵ�������    
	byte=bsp_spi1_rw_byte(0Xff);          //��ȡһ���ֽ�  
	W25QXX_CS=1;                            //ȡ��Ƭѡ     
	return byte;   
} 


void w25qxx_writeSR(u8 regno,u8 sr)   
{   
	u8 command=0;
	switch(regno)
	{
		case 1:
			command=W25X_WriteStatusReg1;    //д״̬�Ĵ���1ָ��
			break;
		case 2:
			command=W25X_WriteStatusReg2;    //д״̬�Ĵ���2ָ��
			break;
		case 3:
			command=W25X_WriteStatusReg3;    //д״̬�Ĵ���3ָ��
			break;
		default:
			command=W25X_WriteStatusReg1;    
			break;
	}   
	W25QXX_CS=0;                            //ʹ������   
	bsp_spi1_rw_byte(command);            //����дȡ״̬�Ĵ�������    
	bsp_spi1_rw_byte(sr);                 //д��һ���ֽ�  
	W25QXX_CS=1;                            //ȡ��Ƭѡ     	      
}

void w25qxx_write_enable(void)   
{
	W25QXX_CS=0;                            //ʹ������   
  bsp_spi1_rw_byte(W25X_WriteEnable);   //����дʹ��  
	W25QXX_CS=1;                            //ȡ��Ƭѡ     	      
} 

void w25qxx_write_disable(void)   
{  
	W25QXX_CS=0;                            //ʹ������   
  bsp_spi1_rw_byte(W25X_WriteDisable);  //����д��ָֹ��    
	W25QXX_CS=1;                            //ȡ��Ƭѡ     	      
} 

u16 w25qxx_readID(void)
{
	u16 Temp = 0;	  
	W25QXX_CS=0;				    
	bsp_spi1_rw_byte(0x90);//���Ͷ�ȡID����	    
	bsp_spi1_rw_byte(0x00); 	    
	bsp_spi1_rw_byte(0x00); 	    
	bsp_spi1_rw_byte(0x00); 	 			   
	Temp|=bsp_spi1_rw_byte(0xFF)<<8;  
	Temp|=bsp_spi1_rw_byte(0xFF);	 
	W25QXX_CS=1;				    
	return Temp;
}

void w25qxx_read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
	u16 i;   										    
	W25QXX_CS=0;                            //ʹ������   
	bsp_spi1_rw_byte(W25X_ReadData);      //���Ͷ�ȡ����  
	if(W25QXX_TYPE==W25Q256)                //�����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
	{
		bsp_spi1_rw_byte((u8)((ReadAddr)>>24));    
	}
	bsp_spi1_rw_byte((u8)((ReadAddr)>>16));   //����24bit��ַ    
	bsp_spi1_rw_byte((u8)((ReadAddr)>>8));   
	bsp_spi1_rw_byte((u8)ReadAddr);   
	for(i=0;i<NumByteToRead;i++)
	{ 
		pBuffer[i]=bsp_spi1_rw_byte(0XFF);    //ѭ������  
	}
	W25QXX_CS=1;  				    	      
}  

void w25qxx_write_page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
	u16 i;  
	w25qxx_write_enable();                  //SET WEL 
	W25QXX_CS=0;                            //ʹ������   
	bsp_spi1_rw_byte(W25X_PageProgram);   //����дҳ����   
	if(W25QXX_TYPE==W25Q256)                //�����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
	{
		bsp_spi1_rw_byte((u8)((WriteAddr)>>24)); 
	}
	bsp_spi1_rw_byte((u8)((WriteAddr)>>16)); //����24bit��ַ    
	bsp_spi1_rw_byte((u8)((WriteAddr)>>8));   
	bsp_spi1_rw_byte((u8)WriteAddr);   
	for(i=0;i<NumByteToWrite;i++)
	{
		bsp_spi1_rw_byte(pBuffer[i]);//ѭ��д��  
	}
	W25QXX_CS=1;                            //ȡ��Ƭѡ 
	w25qxx_wait_busy();					   //�ȴ�д�����
} 

void w25qxx_write_nocheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���		 	    
	if(NumByteToWrite<=pageremain)
	{
		pageremain=NumByteToWrite;//������256���ֽ�
	}
	while(1)
	{	   
		w25qxx_write_page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)
		{
			break;//д�������
		}
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite>256)
			{
				pageremain=256; //һ�ο���д��256���ֽ�
			}
			else 
			{
				pageremain=NumByteToWrite; 	  //����256���ֽ���
			}
		}
	}	    
} 

u8 W25QXX_BUFFER[4096];		 
void w25qxx_write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8* W25QXX_BUF;	  
  W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;//������ַ  
	secoff=WriteAddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С   
 	if(NumByteToWrite<=secremain)
	{
		secremain=NumByteToWrite;//������4096���ֽ�
	}
	while(1) 
	{	
		w25qxx_read(W25QXX_BUF,secpos*4096,4096);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			w25qxx_erase_sector(secpos);//�����������
			for(i=0;i<secremain;i++)	   //����
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			w25qxx_write_nocheck(W25QXX_BUF,secpos*4096,4096);//д����������  
		}
		else 
		{
			w25qxx_write_nocheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		}
		if(NumByteToWrite==secremain)
		{
			break;//д�������
		}
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		  pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		  NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)
			{
				secremain=4096;	//��һ����������д����
			}
			else 
			{
				secremain=NumByteToWrite;			//��һ����������д����
			}
		}	 
	}
}

void w25qxx_erase_chip(void)   
{                                   
	w25qxx_write_enable();                  //SET WEL 
	w25qxx_wait_busy();   
	W25QXX_CS=0;                            //ʹ������   
	bsp_spi1_rw_byte(W25X_ChipErase);        //����Ƭ��������  
	W25QXX_CS=1;                            //ȡ��Ƭѡ     	      
	w25qxx_wait_busy();   				   //�ȴ�оƬ��������
}

void w25qxx_erase_sector(u32 Dst_Addr)   
{  
	Dst_Addr*=4096;
	w25qxx_write_enable();                  //SET WEL 	 
	w25qxx_wait_busy();   
	W25QXX_CS=0;                            //ʹ������   
	bsp_spi1_rw_byte(W25X_SectorErase);   //������������ָ�� 
	if(W25QXX_TYPE==W25Q256)                //�����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
	{
		bsp_spi1_rw_byte((u8)((Dst_Addr)>>24)); 
	}
	bsp_spi1_rw_byte((u8)((Dst_Addr)>>16));  //����24bit��ַ    
	bsp_spi1_rw_byte((u8)((Dst_Addr)>>8));   
	bsp_spi1_rw_byte((u8)Dst_Addr);  
	W25QXX_CS=1;                            //ȡ��Ƭѡ     	      
	w25qxx_wait_busy();   				    //�ȴ��������
}

void w25qxx_wait_busy(void)   
{   
	while((w25qxx_readSR(1)&0x01)==0x01);   // �ȴ�BUSYλ���
}

void w25qxx_power_down(void)   
{ 
	W25QXX_CS=0;                            //ʹ������   
	bsp_spi1_rw_byte(W25X_PowerDown);     //���͵�������  
	W25QXX_CS=1;                            //ȡ��Ƭѡ     	      
	tmdelay_us(3);                            //�ȴ�TPD  
}

void w25qxx_wakeup(void)   
{  
	W25QXX_CS=0;                                //ʹ������   
	bsp_spi1_rw_byte(W25X_ReleasePowerDown);  //  send W25X_PowerDown command 0xAB    
	W25QXX_CS=1;                                //ȡ��Ƭѡ     	      
	tmdelay_us(3);                                //�ȴ�TRES1
}   



