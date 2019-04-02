#include "usb_test.h"
#include "ff.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "diskio.h"
#include "usbh_diskio.h"

/***********************************************************************************************************
��ģ��U����ص�Ӧ��
***********************************************************************************************************/

FATFS fs[_VOLUMES]; 

//usb host �ص�����
void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id)
{
	FRESULT res;
	switch (id)
	{
		case HOST_USER_SELECT_CONFIGURATION:
				break;
		case HOST_USER_DISCONNECTION:
				res=f_mount(0,"1:",1); 	//ж��U��
				if(res==FR_OK)
				{
					printf("ж��U�̳ɹ�\r\n");
				}
				else
				{
					printf("ж��U��ʧ��\r\n");
				}
				break;
		case HOST_USER_CLASS_ACTIVE:	
				res=f_mount(&fs[1],"1:",1); 	//���¹���U��
				if(res==FR_OK)
				{
					printf("����U�̳ɹ�\r\n");
				}
				else
				{
					printf("����U��ʧ��\r\n");
				}
				break;
		case HOST_USER_CONNECTION:
				break;
		default:
				break;
	}
}

// usb��ʼ��
void usb_init(void)
{
	USBH_Init(&hUSBHost, USBH_UserProcess, 0);
  USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
  USBH_Start(&hUSBHost);
  HAL_PWREx_EnableUSBVoltageDetector();
}

