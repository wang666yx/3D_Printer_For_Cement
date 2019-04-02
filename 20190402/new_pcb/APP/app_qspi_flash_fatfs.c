#include "app_qspi_flash_fatfs.h"
#include "ff.h"


/***********************************************************************************************************
��ģ��Ϊspi flash�ļ�ϵͳ����ĳ�ʼ���Ͳ���
***********************************************************************************************************/

FRESULT qf_result;
FATFS qf_fs;
FIL qf_file;
DIR qf_DirInf;
FILINFO qf_FileInf;

char buf[128];

void CreateNewWriteFile(void)
{
	u32 bw;
/****************************************/
	qf_result = f_open(&qf_file,  "0:/hello.txt", FA_CREATE_ALWAYS | FA_WRITE);
	if(qf_result!=FR_OK)
	{
		printf("�ļ�����ʧ��%d\r\n",qf_result);
	}
	else
	{
		printf("�ļ������ɹ�\r\n");
	}
/***************************************/
	qf_result=f_write(&qf_file, "1234567890", 11, &bw);
	if(qf_result!=FR_OK)
	{
		printf("д��ʧ��%d\r\n",qf_result);
	}
	else
	{
		printf("д��ɹ�\r\n");
	}
/*****************************************/
	f_close(&qf_file);
}

void ReadFileData(void)
{
	u32 bw;
/************************************************/
	qf_result = f_open(&qf_file, "0:/hello.txt", FA_OPEN_EXISTING | FA_READ);
	if(qf_result!=FR_OK)
	{
		printf("�ļ���ʧ��%d\r\n",qf_result);
	}
	else
	{
		printf("�ļ��򿪳ɹ�\r\n");
	}
/***************************************/
	qf_result = f_read(&qf_file, &buf, sizeof(buf) - 1, &bw);
	if (bw > 0)
	{
		buf[bw] = 0;
		printf("�ļ����� : \r\n%s\r\n", buf);
	}
	else
	{
		printf("�ļ����� : \r\n");
	}
/************************************************************/
	f_close(&qf_file);
}

//spi flash�ļ�ϵͳ��ʼ��

void app_qspi_flash_fatfs_init(void)
{
	qf_result=f_mount(&qf_fs,"0:",1);
	if(qf_result==FR_NO_FILESYSTEM)
	{
		qf_result=f_mkfs("0:",0,4096);							//��ʽ��
		qf_result = f_mount(&qf_fs,"0:",0);						//��ʽ������ȡ������
		qf_result = f_mount(&qf_fs,"0:",1);						//���¹���
	}
	
//	CreateNewWriteFile();
//	ReadFileData();
}
