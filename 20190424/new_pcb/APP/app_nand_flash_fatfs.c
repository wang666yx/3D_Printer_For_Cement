#include "app_nand_flash_fatfs.h"
#include "ff.h"


/***********************************************************************************************************
��ģ��Ϊnand flash�ļ�ϵͳ����ĳ�ʼ���Ͳ���
***********************************************************************************************************/

FRESULT nf_result;
FATFS nf_fs;
FIL nf_file;
DIR nf_DirInf;
FILINFO nf_FileInf;

char nf_buf[128];

void nf_CreateNewWriteFile(void)
{
	u32 bw;
/****************************************/
	nf_result = f_open(&nf_file,  "2:/hello.txt", FA_CREATE_ALWAYS | FA_WRITE);
	if(nf_result!=FR_OK)
	{
		printf("�ļ�����ʧ��%d\r\n",nf_result);
	}
	else
	{
		printf("�ļ������ɹ�\r\n");
	}
/***************************************/
	nf_result=f_write(&nf_file, "0987654321", 11, &bw);
	if(nf_result!=FR_OK)
	{
		printf("д��ʧ��%d\r\n",nf_result);
	}
	else
	{
		printf("д��ɹ�\r\n");
	}
/*****************************************/
	f_close(&nf_file);
}

void nf_ReadFileData(void)
{
	u32 bw;
/************************************************/
	nf_result = f_open(&nf_file, "2:/hello.txt", FA_OPEN_EXISTING | FA_READ);
	if(nf_result!=FR_OK)
	{
		printf("�ļ���ʧ��%d\r\n",nf_result);
	}
	else
	{
		printf("�ļ��򿪳ɹ�\r\n");
	}
/***************************************/
	nf_result = f_read(&nf_file, &nf_buf, sizeof(nf_buf) - 1, &bw);
	if (bw > 0)
	{
		nf_buf[bw] = 0;
		printf("�ļ����� : \r\n%s\r\n", nf_buf);
	}
	else
	{
		printf("�ļ����� : \r\n");
	}
/************************************************************/
	f_close(&nf_file);
}

//nand flash�ļ�ϵͳ��ʼ��
void app_nand_flash_fatfs_init(void)
{
	nf_result=f_mount(&nf_fs,"2:",1);
	if(nf_result==FR_NO_FILESYSTEM)
	{
		nf_result=f_mkfs("2:",0,4096);							//��ʽ��
		nf_result = f_mount(&nf_fs,"2:",0);						//��ʽ������ȡ������
		nf_result = f_mount(&nf_fs,"2:",1);						//���¹���
	}
	
//	nf_CreateNewWriteFile();
//	nf_ReadFileData();
}
