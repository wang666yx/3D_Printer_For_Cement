#include "app_usb.h"
#include "usbh_stm32f40x.h"




static const char * ReVal_Table[]= 
{
	"0���ɹ�",				                        
	"1��IO����I/O������ʼ��ʧ�ܣ�����û�д洢�豸�������豸��ʼ��ʧ��",
	"2������󣬹���ʧ�ܣ�����FAT�ļ�ϵͳ��ζ����Ч��MBR��������¼���߷�FAT��ʽ",
	"3��FAT��־��ʼ��ʧ�ܣ�FAT��ʼ���ɹ��ˣ�������־��ʼ��ʧ��",
};


//static void DotFormat(uint64_t _ullVal, char *_sp) 
//{
//	/* ��ֵ���ڵ���10^9 */
//	if (_ullVal >= (U64)1e9) 
//	{
//		_sp += sprintf (_sp, "%d.", (uint32_t)(_ullVal / (uint64_t)1e9));
//		_ullVal %= (uint64_t)1e9;
//		_sp += sprintf (_sp, "%03d.", (uint32_t)(_ullVal / (uint64_t)1e6));
//		_ullVal %= (uint64_t)1e6;
//		sprintf (_sp, "%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
//		return;
//	}
//	
//	/* ��ֵ���ڵ���10^6 */
//	if (_ullVal >= (uint64_t)1e6) 
//	{
//		_sp += sprintf (_sp,"%d.", (uint32_t)(_ullVal / (uint64_t)1e6));
//		_ullVal %= (uint64_t)1e6;
//		sprintf (_sp,"%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
//		return;
//	}
//	
//	/* ��ֵ���ڵ���10^3 */
//	if (_ullVal >= 1000) 
//	{
//		sprintf (_sp, "%d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
//		return;
//	}
//	
//	/* ������ֵ */
//	sprintf (_sp,"%d",(U32)(_ullVal));
//}

//static void ViewRootDir(void)
//{
//	FINFO info;
//	uint8_t buf[15];
//	
//    info.fileID = 0;   /* ÿ��ʹ��ffind����ǰ��info.fileID�����ʼ��Ϊ0 */

//	printf("------------------------------------------------------------------\r\n");
//	printf("�ļ���                 |  �ļ���С     | �ļ�ID  | ����      |����\r\n");
//	
//	/* 
//	   ����Ŀ¼�µ������ļ��г�����
//	   1. "*" ���� "*.*" ����ָ��·���µ������ļ�
//	   2. "abc*"         ����ָ��·������abc��ͷ�������ļ�
//	   3. "*.jpg"        ����ָ��·������.jpg��β�������ļ�
//	   4. "abc*.jpg"     ����ָ��·������abc��ͷ��.jpg��β�������ļ�
//	
//	   ������ʵ��������Ŀ¼�������ļ�
//	*/
//	while(ffind ("U0:*.*", &info) == 0)  
//	{ 
//		/* �����ļ���ʾ��С��ʽ */
//		DotFormat(info.size, (char *)buf);
//		
//		/* ��ӡ��Ŀ¼�µ������ļ� */
//		printf ("%-20s %12s bytes, ID: %04d  ",
//				info.name,
//				buf,
//				info.fileID);
//		
//		/* �ж����ļ�������Ŀ¼ */
//		if (info.attrib & ATTR_DIRECTORY)
//		{
//			printf("(0x%02x)Ŀ¼", info.attrib);
//		}
//		else
//		{
//			printf("(0x%02x)�ļ�", info.attrib);
//		}
//		
//		/* ��ʾ�ļ����� */
//		printf ("  %04d.%02d.%02d  %02d:%02d\r\n",
//                 info.time.year, info.time.mon, info.time.day,
//               info.time.hr, info.time.min);
//    }
//	
//	if (info.fileID == 0)  
//	{
//		printf ("����û�д���ļ�\r\n");
//	}

//	printf("------------------------------------------------------------------\r\n");
//}

void app_usb_task(void)
{
	uint8_t con, con_ex;
	uint8_t ucRunFlag = 0;
	u32 *state=(u32*)0x50000440;
	
	USBH_STM32_Pins_Config(0,1);
	USBH_STM32_Init(0,1);
	
	while(1)
	{
		u32 a;
		a=*state;
		if((a&(1<<10)))
		{
			break;
		}
		os_dly_wait(1);
	}
	
	con = app_usb_initusb();
	con_ex = con | 0x80;  /* �״��ϵ�ǿ��ִ��һ��if(con^con_ex)��������� */
	
//	ViewRootDir();
	while(1)
	{	
		/* �Ͽ����Ӻ󣬴˺���Ҳ��ж����Դ */
		usbh_engine(0); 
		con = usbh_msc_status(0, 0);
		if(con^con_ex)
		{
			if (!con)  
			{
				printf ("U���Ѿ��Ͽ�\r\n");
				printf("------------------------------------------------------------------\r\n");	
			}
			else 
			{
				/* ϵͳ�ϵ��״����к͵���ָ��L�󣬲���Ҫ�ظ���ʼ�� */
				if(ucRunFlag == 0)
				{
					ucRunFlag = 1;
				}
				else
				{
					con = app_usb_initusb();				
				}
			}
			con_ex = con;
		}
		os_dly_wait(1);
	}
}


u8 app_usb_initusb(void)
{
	uint8_t result;
	
	/* ����SD�� */
	result = finit("U0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		printf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		
		return 0;
	}
	
	printf ("U���Ѿ�������\r\n");
	printf("------------------------------------------------------------------\r\n");	
	return 1;
}

void app_usb_uninitusb(void)
{
	uint8_t result;

	/* ж��SD�� */
	result = funinit("U0:");
	if(result != NULL)
	{
		printf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		printf("ж���ļ�ϵͳ�ɹ�\r\n");
	}
}


