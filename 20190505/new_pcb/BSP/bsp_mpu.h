#ifndef __MPU_H
#define __MPU_H
#include "tmsys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32H7������
//MPU��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

u8 MPU_Set_Protection(u32 baseaddr,u32 size,u32 rnum,u32 ap);
void MPU_Memory_Protection(void);
#endif
