/*
*********************************************************************************************************
*
*	ģ������ : FlashFS�ļ�ϵͳ��ʾģ�顣
*	�ļ����� : demo_sd_flashfs.c
*	��    �� : V1.0
*	˵    �� : ��������ֲFlashFS�ļ�ϵͳ����ҪѧϰFlashFS���ۺϲ�����
*   ע������ : 1. FlashFS��֧�ֲ��������ļ������ļ�����֧����ʹ�ö��ļ�����FSN_CM3.lib�г�8.3��ʽ��������
*                 ����ʹ��fopen�������������ļ������ļ�ʱ�����
*              2. FlashFSֻ��MDK�Ŀ⣬û��IAR��GCC��
*
*	�޸ļ�¼ :
*		�汾��    ����         ����            ˵��
*       V1.0    2015-09-10   Eric2013    1. RL-FlashFS���ļ����汾V4.74
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"


/* ���ڲ��Զ�д�ٶ� */
#define TEST_FILE_LEN			(2*1024*1024)	/* ���ڲ��Ե��ļ����� */
#define BUF_SIZE				(4*1024)		/* ÿ�ζ�дSD����������ݳ��� */
uint8_t g_TestBuf[BUF_SIZE];

/* �������ļ��ڵ��õĺ������� */
static uint8_t InitUSB(void);
static void UnInitUSB(void);
static void ViewSDCapacity(void);
static void ViewRootDir(void);
static void CreateNewFile(void);
static void ReadFileData(void);
static void SeekFileData(void);
static void DeleteDirFile(void);
static void WriteFileTest(void);
static void WriteCSVFile(void);
static void ReadCSVFileData(void);
static void DispMenu(void);
static void DotFormat(uint64_t _ullVal, char *_sp);
extern int getkey (void);

/* FlashFS API�ķ���ֵ */
static const char * ReVal_Table[]= 
{
	"0���ɹ�",				                        
	"1��IO����I/O������ʼ��ʧ�ܣ�����û�д洢�豸�������豸��ʼ��ʧ��",
	"2������󣬹���ʧ�ܣ�����FAT�ļ�ϵͳ��ζ����Ч��MBR��������¼���߷�FAT��ʽ",
	"3��FAT��־��ʼ��ʧ�ܣ�FAT��ʼ���ɹ��ˣ�������־��ʼ��ʧ��",
};

/*
*********************************************************************************************************
*	�� �� ��: DemoFlashFS
*	����˵��: FlashFS��ϵͳ��ʾ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoFlashFS(void)
{
	int8_t cmd=0x30;
	u32 i=0;
	uint8_t con, con_ex;
	uint8_t ucRunFlag = 0;
	
	/* ��ӡ�����б��û�����ͨ�����ڲ���ָ�� */
	DispMenu();

	con = InitUSB();
	con_ex = con | 0x80;  /* �״��ϵ�ǿ��ִ��һ��if(con^con_ex)��������� */

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
					con = InitUSB();				
				}		
			}
			con_ex = con;
		}
		if(i>=1000)
		{
			i=0;
					if (cmd > 0)
		{
			switch (cmd)
			{
				case 'L':
					printf("��L - InitUSB��\r\n");
					con = InitUSB();	/* SD���Ĺ��أ�ж�ؼ���������ʾ */
					ucRunFlag = 0;   /* ��ֹ�ظ���ʼ�� */
					break;
				
				case 'U':
					printf("��U - UnInitUSB��\r\n");
					UnInitUSB();		/* SD���Ĺ��أ�ж�ؼ���������ʾ */
					break;
				
				case '1':
					printf("��1 - ViewSDCapacity��\r\n");
					ViewSDCapacity();		/* SD���Ĺ��أ�ж�ؼ���������ʾ */
					break;
				
				case '2':
					printf("��2 - ViewRootDir��\r\n");
					ViewRootDir();		    /* ��ʾSD����Ŀ¼�µ��ļ��� */
					break;
				
				case '3':
					printf("��3 - CreateNewFile��\r\n");
					CreateNewFile();	    /* ��������text�ı���ʹ�ò�ͬ����д������ */
					break;
				
				case '4':
					printf("��4 - ReadFileData��\r\n");
					ReadFileData();	        /* ʹ��������ͬ������ȡ�ı�  */
					break;

				case '5':
					printf("��5 - SeekFileData��\r\n");
					SeekFileData();	        /* ����һ��text�ı�����ָ��λ�ö�����ж�д����  */
					break;
				
				case '6':
					printf("��6 - DeleteDirFile��\r\n");
					DeleteDirFile();	    /* ɾ���ļ��к��ļ�  */
					break;
				
				case '7':
					printf("��7 - WriteFileTest��\r\n");
				//	WriteFileTest();	    /* �����ļ���д�ٶ�  */
					break;
				
				case '8':
					printf("��8 - WriteCSVFile��\r\n");
					WriteCSVFile();	        /* д���ݵ�CSV�ļ���  */
					break;
				
				case '9':
					printf("��9 - ReadCSVFileData��\r\n");
					ReadCSVFileData();	    /* ��CSV�ļ��ж�����  */
					break;
				
				default:
					//DispMenu();
					break;
			}
		}
			if(cmd<=0x3b)
			{
				cmd++;
			}
		}
		i++;
		os_dly_wait(1);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/* �������Ĳ�Ҫȥ�����޸ģ�������̴���û���κ������ */
static void DispMenu(void)
{
	printf("\r\n------------------------------------------------\r\n");
	//printf("֧���Ȳ�Σ������Զ�ʶ������ʼ������������L��U����\r\n");
	printf("֧���Ȳ�Σ������Զ�ʶ������ʼ��������������ü��غ�ж��ָ��\r\n");
	printf("��ѡ�񴮿ڲ���������Լ��̴�ӡ���ּ���:\r\n");
	printf("L - ����USB\r\n");
	printf("U - ж��USB\r\n");	
	printf("1 - ��ʾSD��������ʣ������\r\n");
	printf("2 - ��ʾSD����Ŀ¼�µ��ļ�\r\n");
	printf("3 - ��������text�ı���ʹ�ò�ͬ����д������\r\n");
	printf("4 - ʹ��������ͬ������ȡ�ı�\r\n");
	printf("5 - ����һ��text�ı���ָ��һ��λ�ö�����ж�д����\r\n");
	printf("6 - ɾ���ļ��к��ļ�\r\n");
	printf("7 - �����ļ���д�ٶ�\r\n");
	printf("8 - д���ݵ�CSV�ļ���\r\n");
	printf("9 - ��CSV�ļ��ж�����\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DotFormat;
*	����˵��: �����ݹ淶����ʾ�������û��鿴
*             ����
*             2345678   ---->  2.345.678
*             334426783 ---->  334.426.783
*             ��������Ϊ��λ������ʾ
*	��    ��: _ullVal   ��Ҫ�淶��ʾ����ֵ
*             _sp       �淶��ʾ�����ݴ洢��buf��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DotFormat(uint64_t _ullVal, char *_sp) 
{
	/* ��ֵ���ڵ���10^9 */
	if (_ullVal >= (U64)1e9) 
	{
		_sp += sprintf (_sp, "%d.", (uint32_t)(_ullVal / (uint64_t)1e9));
		_ullVal %= (uint64_t)1e9;
		_sp += sprintf (_sp, "%03d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp, "%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ��ֵ���ڵ���10^6 */
	if (_ullVal >= (uint64_t)1e6) 
	{
		_sp += sprintf (_sp,"%d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp,"%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ��ֵ���ڵ���10^3 */
	if (_ullVal >= 1000) 
	{
		sprintf (_sp, "%d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ������ֵ */
	sprintf (_sp,"%d",(U32)(_ullVal));
}

/*
*********************************************************************************************************
*	�� �� ��: InitUSB
*	����˵��: ��ʼ��USB
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t InitUSB(void)
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

/*
*********************************************************************************************************
*	�� �� ��: UnInitUSB
*	����˵��: ж��USB
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void UnInitUSB(void)
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

/*
*********************************************************************************************************
*	�� �� ��: ViewSDCapacity
*	����˵��: SD���Ĺ��أ�ж�ؼ���������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ViewSDCapacity(void)
{
	uint64_t ullSdCapacity;
	uint8_t buf[15];
	
	/* ��ȡvolume label */
	if (fvol ("U0:", (char *)buf) == 0) 
	{
		if (buf[0]) 
		{
			printf ("SD����volume label�� %s\r\n", buf);
		}
		else 
		{
			printf ("SD��û��volume label\r\n");
		}
	}
	else 
	{
		printf ("Volume���ʴ���\r\n");
	}

	/* ��ȡSD��ʣ������ */
	ullSdCapacity = ffree("U0:");
	DotFormat(ullSdCapacity, (char *)buf);
	printf("SD��ʣ������ = %10s�ֽ�\r\n", buf);
	printf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: ViewRootDir
*	����˵��: ��ʾSD����Ŀ¼�µ��ļ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ViewRootDir(void)
{
	FINFO info;
	uint8_t buf[15];
	
    info.fileID = 0;   /* ÿ��ʹ��ffind����ǰ��info.fileID�����ʼ��Ϊ0 */

	printf("------------------------------------------------------------------\r\n");
	printf("�ļ���                 |  �ļ���С     | �ļ�ID  | ����      |����\r\n");
	
	/* 
	   ����Ŀ¼�µ������ļ��г�����
	   1. "*" ���� "*.*" ����ָ��·���µ������ļ�
	   2. "abc*"         ����ָ��·������abc��ͷ�������ļ�
	   3. "*.jpg"        ����ָ��·������.jpg��β�������ļ�
	   4. "abc*.jpg"     ����ָ��·������abc��ͷ��.jpg��β�������ļ�
	
	   ������ʵ��������Ŀ¼�������ļ�
	*/
	while(ffind ("U0:*.*", &info) == 0)  
	{ 
		/* �����ļ���ʾ��С��ʽ */
		DotFormat(info.size, (char *)buf);
		
		/* ��ӡ��Ŀ¼�µ������ļ� */
		printf ("%-20s %12s bytes, ID: %04d  ",
				info.name,
				buf,
				info.fileID);
		
		/* �ж����ļ�������Ŀ¼ */
		if (info.attrib & ATTR_DIRECTORY)
		{
			printf("(0x%02x)Ŀ¼", info.attrib);
		}
		else
		{
			printf("(0x%02x)�ļ�", info.attrib);
		}
		
		/* ��ʾ�ļ����� */
		printf ("  %04d.%02d.%02d  %02d:%02d\r\n",
                 info.time.year, info.time.mon, info.time.day,
               info.time.hr, info.time.min);
    }
	
	if (info.fileID == 0)  
	{
		printf ("����û�д���ļ�\r\n");
	}

	printf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: CreateNewFile
*	����˵��: ��SD����������text�ı����ֱ�ʹ��fwrite��fprintf��fputsд���ݡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void CreateNewFile(void)
{
	const uint8_t WriteText[] = {"�人�������������޹�˾\r\n2015-09-06\r\nwww.armfly.com\r\nWWW.ARMFLY.COM"};
	const uint8_t WriteText1[] = {"�人�������������޹�˾\r"};
	FILE *fout;
	uint32_t bw;
	uint32_t i = 2;

	printf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test1.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fout = fopen ("U0:\\test\\test1.txt", "w"); 
	if (fout != NULL) 
	{
		printf("���ļ�U0:\\test\\test1.txt�ɹ������û�����ļ��к�txt�ļ����Զ�����\r\n");
		/* д���� */
		bw = fwrite (WriteText, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fout);
		if(bw == sizeof(WriteText)/sizeof(uint8_t))
		{
			printf("д�����ݳɹ�\r\n");
		}
		else
		{ 
			printf("д������ʧ��\r\n");
		}
		
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		printf("���ļ�U0:\\test\\test.txtʧ��\r\n");
	}
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test2.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fout = fopen ("U0:\\test\\test2.txt", "w"); 
	if (fout != NULL) 
	{
		printf("���ļ�U0:\\test\\test2.txt�ɹ������û�����ļ��к�txt�ļ����Զ�����\r\n");
		
		/* д���� */
		bw = fprintf (fout, "%d %d %f\r\n", i, i*5, i*5.55f);
		
		/* ʹ�ú���ferror����Ƿ��������� */
		if (ferror(fout) != NULL)  
		{
			printf("д������ʧ��\r\n");
		}
		else
		{
			printf("д�����ݳɹ�\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		printf("���ļ�U0:\\test\\test.txtʧ��\r\n");
	}
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test3.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fout = fopen ("U0:\\test\\test3.txt", "w"); 
	if (fout != NULL) 
	{
		printf("���ļ�U0:\\test\\test3.txt�ɹ������û�����ļ��к�txt�ļ����Զ�����\r\n");
		
		/* д���� */
		fputs((const char *)WriteText1, fout);
		
		/* ʹ�ú���ferror����Ƿ��������� */
		if (ferror(fout) != NULL)  
		{
			printf("д������ʧ��\r\n");
		}
		else
		{
			printf("д�����ݳɹ�\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		printf("���ļ�U0:\\test\\test.txtʧ��\r\n");
	}

	printf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: ReadFileData
*	����˵��: �ֱ�ʹ��fread��fscan��fgets��ȡ������ͬ��txt�ļ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ReadFileData(void)
{
	const uint8_t WriteText[] = {"�人�������������޹�˾\r\n2015-09-06\r\nwww.armfly.com\r\nWWW.ARMFLY.COM"};
	uint8_t Readbuf[100];
	FILE *fin;
	uint32_t bw;
	uint32_t index1, index2;
	float  count = 0.0f;

	printf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test1.txt */
	fin = fopen ("U0:\\test\\test1.txt", "r"); 
	if (fin != NULL) 
	{
		printf("<1>���ļ�U0:\\test\\test1.txt�ɹ�\r\n");
		
		/* ��ֹ���� */
		(void) WriteText;
		
		/* ������ */
		bw = fread(Readbuf, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fin);
		if(bw == sizeof(WriteText)/sizeof(uint8_t))
		{
			Readbuf[bw] = NULL;
			printf("test1.txt �ļ����� : \r\n%s\r\n", Readbuf);
		}
		else
		{ 
			printf("������ʧ��\r\n");
		}
		
		/* �ر��ļ� */
		fclose(fin);
	}
	else
	{
		printf("���ļ�U0:\\test\\test.txtʧ��, �����ļ�������\r\n");
	}
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test2.txt */
	fin = fopen ("U0:\\test\\test2.txt", "r"); 
	if (fin != NULL) 
	{
		printf("\r\n<2>���ļ�M0:\\test\\test2.txt�ɹ�\r\n");
		
		bw = fscanf(fin, "%d %d %f", &index1, &index2, &count);

		/* 3�������������� */
		if (bw == 3)  
		{
			printf("��������ֵ\r\nindex1 = %d index2 = %d count = %f\r\n", index1, index2, count);
		}
		else
		{
			printf("������ʧ��\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fin);
	}
	else
	{
		printf("���ļ�U0:\\test\\test.txtʧ��\r\n");
	}
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test3.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fin = fopen ("U0:\\test\\test3.txt", "r"); 
	if (fin != NULL) 
	{
		printf("\r\n<3>���ļ�U0:\\test\\test3.txt�ɹ�\r\n");
		
		/* ������ */
		if(fgets((char *)Readbuf, sizeof(Readbuf), fin) != NULL)
		{
			printf("test3.txt �ļ����� : \r\n%s\r\n", Readbuf);
		}
		else
		{
			printf("������ʧ��\r\n");
		}
		
		/* �ر��ļ� */
		fclose(fin);
	}
	else
	{
		printf("���ļ�U0:\\test\\test.txtʧ��\r\n");
	}
	
	printf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: SeekFileData
*	����˵��: ����һ��text�ı�����ָ��λ�ö�����ж�д������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SeekFileData(void)
{
	const uint8_t WriteText[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};
	FILE *fin, *fout;
	uint32_t bw;
	uint32_t uiPos;
	uint8_t ucChar;

	printf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test1.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fout = fopen ("U0:\\test.txt", "w"); 
	if (fout != NULL) 
	{
		printf("���ļ�U0:\\test.txt�ɹ������û��txt�ļ����Զ�����\r\n");
		/* д���� */
		bw = fwrite (WriteText, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fout);
		if(bw == sizeof(WriteText)/sizeof(uint8_t))
		{
			printf("д�����ݳɹ�\r\n");
		}
		else
		{ 
			printf("д������ʧ��\r\n");
		}
			
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		printf("���ļ�U0:\\test.txtʧ��\r\n");
	}
	
	/***********************************************/
	fin = fopen ("U0:\\test.txt","r");
	if (fin != NULL)  
	{
		printf("\r\n���ļ�U0:\\test.txt�ɹ�\r\n");
		
		/* ��ȡ�ļ�test.txt��λ��0���ַ� */
		fseek (fin, 0L, SEEK_SET);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		printf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* ��ȡ�ļ�test.txt��λ��1���ַ� */
		fseek (fin, 1L, SEEK_SET);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		printf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);

		/* ��ȡ�ļ�test.txt��λ��25���ַ� */
		fseek (fin, 25L, SEEK_SET);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		printf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* ͨ�����溯���Ĳ�������ǰ��дλ����26
		   ���溯�����ڵ�ǰλ�õĻ����Ϻ���2��λ�ã�Ҳ����24�����ú���fgetc��λ�þ���25
		 */
		fseek (fin, -2L, SEEK_CUR);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		printf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* ��ȡ�ļ�test.txt�ĵ�����2���ַ�, ���һ����'\0' */
		fseek (fin, -2L, SEEK_END); 
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);
		printf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* ����ȡλ�����¶�λ���ļ���ͷ */
		rewind(fin);  
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);
		printf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);	
		
		/* 
		   ��������ʾһ��ungetc�����ã��˺������ǽ���ǰ�Ķ�ȡλ��ƫ�ƻ�һ���ַ���
		   ��fgetc���ú�λ������һ���ַ���
		 */
		fseek (fin, 0L, SEEK_SET);
		ucChar = fgetc (fin);
		uiPos = ftell(fin); 
		printf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		ungetc(ucChar, fin); 
		uiPos = ftell(fin); 
		printf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* �ر�*/
		fclose (fin);
	}
	else
	{
		printf("���ļ�U0:\\test.txtʧ��\r\n");
	}
	
	/***********************************************/
	fin = fopen ("U0:\\test.txt","r+");
	if (fin != NULL)  
	{
		printf("\r\n���ļ�U0:\\test.txt�ɹ�\r\n");
		
		/* �ļ�test.txt��λ��2�������ַ� '��' */
		fseek (fin, 2L, SEEK_SET);
		ucChar = fputc ('!', fin);
		/* ˢ�����ݵ��ļ��� */
		fflush(fin);		

		fseek (fin, 2L, SEEK_SET);
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);				
		printf("�ļ�test.txtλ�ã�%02d���Ѿ��������ַ���%c\r\n", uiPos, ucChar);
	
		
		/* �ļ�test.txt�ĵ�����2���ַ�, �������ַ� ��&�� �����һ����'\0' */
		fseek (fin, -2L, SEEK_END); 
		ucChar = fputc ('&', fin);
		/* ˢ�����ݵ��ļ��� */
		fflush(fin);	

		fseek (fin, -2L, SEEK_END); 
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);	
		printf("�ļ�test.txtλ�ã�%02d���Ѿ��������ַ���%c\r\n", uiPos, ucChar);		
		
		/* �ر�*/
		fclose (fin);
	}
	else
	{
		printf("���ļ�U0:\\test.txtʧ��\r\n");
	}

	printf("------------------------------------------------------------------\r\n");
	
}

/*
*********************************************************************************************************
*	�� �� ��: DeleteDirFile
*	����˵��: ɾ���ļ��к��ļ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DeleteDirFile(void)
{
	uint8_t result;
	uint8_t i;
	char FileName[50];
	
	printf("------------------------------------------------------------------\r\n");
	
	/* ɾ���ļ� speed1.txt */
	for (i = 0; i < 20; i++)
	{
		sprintf(FileName, "U0:\\Speed%02d.txt", i);		/* ÿд1�Σ���ŵ��� */
		result = fdelete (FileName);
		if (result != NULL) 
		{
			printf("%s�ļ������ڣ�����ֵ��%d��\r\n", FileName, result);
		}
		else
		{
			printf("ɾ��%s�ļ��ɹ�\r\n", FileName);
		}
	}
	
	for(i = 1; i < 4; i++)
	{
		sprintf(FileName, "U0:\\test\\test%01d.txt", i);   /* ÿд1�Σ���ŵ��� */
		result = fdelete (FileName);
		if (result != NULL) 
		{
			printf("%s�ļ������ڣ�����ֵ��%d��\r\n", FileName, result);
		}
		else
		{
			printf("ɾ��%s�ļ��ɹ�\r\n", FileName);
		}	
	}
	
	/* ɾ���ļ���test */
	result = fdelete ("U0:\\test\\");
	if (result != NULL) 
	{
		printf("test�ļ��зǿջ򲻴��ڣ�����ֵ��%d��\r\n", result);
	}
	else
	{
		printf("ɾ��test�ļ��гɹ�\r\n");
	}
	
	/* ɾ��csv�ļ� */
	result = fdelete ("U0:\\record.csv");
	if (result != NULL) 
	{
		printf("record.csv�ļ������ڣ�����ֵ��%d��\r\n", result);
	}
	else
	{
		printf("ɾ��record.csv���ɹ�\r\n");
	}
		
	printf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: WriteFileTest
*	����˵��: �����ļ���д�ٶ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void WriteFileTest(void)
{
	FILE *fout;
	uint32_t bw;
	uint32_t i,k;
	uint32_t runtime1,runtime2,timelen;
	uint8_t err = 0;
	char TestFileName[13];
	static uint8_t s_ucTestSn = 0;

	for (i = 0; i < sizeof(g_TestBuf); i++)
	{
		g_TestBuf[i] = (i / 512) + '0';
	}
	
	printf("------------------------------------------------------------------\r\n");

	/* ���ļ� -----------------------------------------*/
	sprintf(TestFileName, "Speed%02d.txt", s_ucTestSn++);		/* ÿд1�Σ���ŵ��� */
	fout = fopen (TestFileName, "w");
	if (fout ==  NULL)
	{
		printf("�ļ�: %s����ʧ��\r\n", TestFileName);
		/* ���ʧ�ܣ���ز�Ҫ�ٲ������ļ�����ֹ����Ӳ���쳣 */
//		goto access_fail;
	}
	
	/* дһ������ ����д�ٶ�-----------------------------------------*/
	printf("��ʼд�ļ�%s %dKB ...\r\n", TestFileName, TEST_FILE_LEN / 1024);
	runtime1 = os_time_get ();	/* ��ȡϵͳ����ʱ�� */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		bw = fwrite (g_TestBuf, sizeof(uint8_t), BUF_SIZE, fout);   
		if (bw == BUF_SIZE)
		{
			if (((i + 1) % 8) == 0)
			{
				printf(".");
			}
		}
		else
		{
			err = 1;
			printf("%s�ļ�дʧ��\r\n", TestFileName);
			break;
		}
	}
	runtime2 = os_time_get ();	/* ��ȡϵͳ����ʱ�� */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		printf("\r\n  д��ʱ : %dms   ƽ��д�ٶ� : %dB/S (%dKB/S)\r\n",
			timelen,
			(TEST_FILE_LEN * 1000) / timelen,
			((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}
	
	/* ʹ�ú���ferror�ٴμ���Ƿ��������� -----------------------*/
	if (ferror (fout))  
	{
		printf ("д���ݹ����г��ֹ�����\r\n\n");
    }
	else
	{
		printf ("д���ݹ�����δ���ֹ�����\r\n\n");		
	}

	fclose (fout);	/* �ر��ļ�*/


	/* ��ʼ���ļ������Զ��ٶ� ---------------------------------------*/
	fout = fopen (TestFileName, "r");
	if (fout ==  NULL)
	{
		printf("�ļ�: %s��ȡʧ��\r\n", TestFileName);
		//goto access_fail;
	}

	printf("��ʼ���ļ� %dKB ...\r\n", TEST_FILE_LEN / 1024);
	runtime1 = os_time_get ();	/* ��ȡϵͳ����ʱ�� */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		bw = fread(g_TestBuf, sizeof(uint8_t), BUF_SIZE, fout);   
		if (bw == BUF_SIZE)
		{
			if (((i + 1) % 8) == 0)
			{
				printf(".");
			}

			/* �Ƚ�д��������Ƿ���ȷ�������ᵼ�¶����ٶȽ������ */
			for (k = 0; k < sizeof(g_TestBuf); k++)
			{
				if (g_TestBuf[k] != (k / 512) + '0')
				{
				  	err = 1;
					printf("Speed1.txt �ļ����ɹ����������ݳ���\r\n");
					break;
				}
			}
			if (err == 1)
			{
				break;
			}
		}
		else
		{
			err = 1;
			printf("Speed1.txt �ļ���ʧ��\r\n");
			break;
		}
	}
	runtime2 = os_time_get ();	/* ��ȡϵͳ����ʱ�� */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		printf("\r\n  ����ʱ : %dms   ƽ�����ٶ� : %dB/S (%dKB/S)\r\n", timelen,
			(TEST_FILE_LEN * 1000) / timelen, ((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}
	
	/* ʹ�ú���ferror�ٴμ���Ƿ��������� -----------------------*/
	if (ferror (fout))  
	{
		printf ("�����ݹ����г��ֹ�����\r\n");
    }
	else
	{
		printf ("�����ݹ�����δ���ֹ�����\r\n");		
	}

	fclose (fout);	/* �ر��ļ�*/
	
	printf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: WriteCSVFile
*	����˵��: д���ݵ�CSV�ļ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void WriteCSVFile(void)
{
	const uint8_t WriteText[] = {"��¼ʱ��,���,��¼��ֵһ,��¼��ֵ��\r\n"};
	FILE *fout;
	static uint16_t i = 1;
	static uint8_t ucFirstRunFlag = 0;

	printf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* 
	  1. ���ļ�record.csv�����û�д��ļ����Զ�������
	  2. �ڶ���������ʾ����ļ�д���ݶ���β����ʼ��ӡ�
	*/
	fout = fopen ("U0:\\record.csv", "a"); 
	if (fout != NULL) 
	{
		printf("���ļ�M0:\\record.csvt�ɹ������û�д��ļ����Զ�����\r\n");
		
		/* д���ݣ�����ǵ�һ��д���ݣ���дCSV�ļ��ı������Ŀ���Ժ�д���ݲ�����Ҫд�˱��� */
		if(ucFirstRunFlag == 0)
		{
			fprintf(fout, (char *)WriteText);
			ucFirstRunFlag = 1;
		}
		
		/* ����д5������ */
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		
		i++;
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		
		i++;
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		
		i++;
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		
		i++;
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		i++;
		
		/* ʹ�ú���ferror����Ƿ��������� */
		if (ferror(fout) != NULL)  
		{
			printf("д������ʧ��\r\n");
		}
		else
		{
			printf("д�����ݳɹ�\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		printf("���ļ�U0:\\record.csvʧ��\r\n");
	}

	printf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: ReadCSVFileData
*	����˵��: ��ȡCSV�ļ��е����ݡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ReadCSVFileData(void)
{
	uint8_t Readbuf[50];
	FILE *fin;

	printf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* ��record.csv�ļ� */
	fin = fopen ("U0:\\record.csv", "r"); 
	if (fin != NULL) 
	{
		printf("���ļ�U0:\\record.csv�ɹ�\r\n");
		
		/* ���������ݶ���������ʹ��fgets�����ĺô����ǿ���һ��һ�еĶ�ȡ����Ϊ�˺�������'\n'���з��ͻ�ֹͣ��ȡ */
		while (fgets((char *)Readbuf, sizeof(Readbuf), fin) != NULL)  
		{
			printf("%s", Readbuf);	
		}
		
		/* ʹ�ú���ferror����Ƿ��������� */
		if (ferror(fin) != NULL)  
		{
			printf("��������ʧ��\r\n");
		}
		else
		{
			printf("record.csv���������ȫ����ȷ����\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fin);
	}
	else
	{
		printf("���ļ�U0:\\record.csvʧ��\r\n");
	}

	printf("------------------------------------------------------------------\r\n");
}


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
