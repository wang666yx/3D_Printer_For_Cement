#include "bsp_uart.h"


UART_HandleTypeDef uart1_handler;
u8 uart1_rxbuf[UART1_RX_SIZE];

UART_HandleTypeDef uart2_handler;
u8 uart2_rxbuf[UART2_RX_SIZE];

void bsp_uart1_init(u32 baud)
{
	uart1_handler.Instance=USART1;
	uart1_handler.Init.BaudRate=baud;
	uart1_handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	uart1_handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	uart1_handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	uart1_handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	uart1_handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	HAL_UART_Init(&uart1_handler);					    //HAL_UART_Init()��ʹ��UART1
	
	HAL_UART_Receive_IT(&uart1_handler,(u8 *)uart1_rxbuf,UART1_RX_SIZE);
}

void bsp_uart2_init(u32 baud)
{
	uart2_handler.Instance=USART2;
	uart2_handler.Init.BaudRate=baud;
	uart2_handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	uart2_handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	uart2_handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	uart2_handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	uart2_handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	HAL_UART_Init(&uart2_handler);					    //HAL_UART_Init()��ʹ��UART2
	
	HAL_UART_Receive_IT(&uart2_handler,(u8 *)uart2_rxbuf,UART2_RX_SIZE);
	
}

void bsp_uart1_sendbyte(u8 data)
{
	__HAL_UART_DISABLE_IT(&uart1_handler,UART_IT_RXNE);
	HAL_UART_Transmit(&uart1_handler,&data,1,1000);	//���ͽ��յ�������
	while(__HAL_UART_GET_FLAG(&uart1_handler,UART_FLAG_TC)!=SET);		//�ȴ����ͽ���
	__HAL_UART_ENABLE_IT(&uart1_handler,UART_IT_RXNE);
}

void bsp_uart2_sendbyte(u8 data)
{
	__HAL_UART_DISABLE_IT(&uart2_handler,UART_IT_RXNE);
	HAL_UART_Transmit(&uart2_handler,&data,1,1000);	//���ͽ��յ�������
	while(__HAL_UART_GET_FLAG(&uart2_handler,UART_FLAG_TC)!=SET);		//�ȴ����ͽ���
	__HAL_UART_ENABLE_IT(&uart2_handler,UART_IT_RXNE);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)//����Ǵ���1�����д���1 MSP��ʼ��
	{
		BSP_UART1_TXD_GPIO_CLK_ENABLE();
		BSP_UART1_RXD_GPIO_CLK_ENABLE();
		__HAL_RCC_USART1_CLK_ENABLE();			//ʹ��USART1ʱ��
		
	
		GPIO_Initure.Pin=BSP_UART1_TXD_GPIO_PIN;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
		GPIO_Initure.Pull=GPIO_PULLUP;			//����
		GPIO_Initure.Speed=GPIO_SPEED_FAST;		//����
		GPIO_Initure.Alternate=GPIO_AF7_USART1;
		HAL_GPIO_Init(BSP_UART1_TXD_GPIO,&GPIO_Initure);

		GPIO_Initure.Pin=BSP_UART1_RXD_GPIO_PIN;
		HAL_GPIO_Init(BSP_UART1_RXD_GPIO,&GPIO_Initure);
		
		HAL_NVIC_EnableIRQ(USART1_IRQn);				//ʹ��USART1�ж�ͨ��
		HAL_NVIC_SetPriority(USART1_IRQn,3,0);				
	}
	
	if(huart->Instance==USART2)//����Ǵ���2�����д���2 MSP��ʼ��
	{
		BSP_UART2_TXD_GPIO_CLK_ENABLE();
		BSP_UART2_RXD_GPIO_CLK_ENABLE();
		__HAL_RCC_USART2_CLK_ENABLE();			//ʹ��USART2ʱ��
		
	
		GPIO_Initure.Pin=BSP_UART2_TXD_GPIO_PIN;			//PD5
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
		GPIO_Initure.Pull=GPIO_PULLUP;			//����
		GPIO_Initure.Speed=GPIO_SPEED_FAST;		//����
		GPIO_Initure.Alternate=GPIO_AF7_USART2;
		HAL_GPIO_Init(BSP_UART2_TXD_GPIO,&GPIO_Initure);

		GPIO_Initure.Pin=BSP_UART2_RXD_GPIO_PIN;
		HAL_GPIO_Init(BSP_UART2_RXD_GPIO,&GPIO_Initure);
		
		HAL_NVIC_EnableIRQ(USART2_IRQn);				//ʹ��USART2�ж�ͨ��
		HAL_NVIC_SetPriority(USART2_IRQn,3,0);				
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//����Ǵ���1
	{
		bsp_uart1_rx_interrupt(uart1_rxbuf[0]);
	}
	if(huart->Instance==USART2)//����Ǵ���2
	{
		bsp_uart2_rx_interrupt(uart2_rxbuf[0]);
	}
}


void USART1_IRQHandler(void)                	
{ 
	HAL_UART_IRQHandler(&uart1_handler);	//����HAL���жϴ����ú���
  while (HAL_UART_GetState(&uart1_handler)!=HAL_UART_STATE_READY);//�ȴ�����
	while(HAL_UART_Receive_IT(&uart1_handler,(u8 *)uart1_rxbuf, UART1_RX_SIZE)!=HAL_OK);//һ�δ������֮�����¿����жϲ�����RxXferCountΪ1	
} 



void USART2_IRQHandler(void)                	
{ 
	HAL_UART_IRQHandler(&uart2_handler);	//����HAL���жϴ����ú���
  while (HAL_UART_GetState(&uart2_handler)!=HAL_UART_STATE_READY);//�ȴ�����
	while(HAL_UART_Receive_IT(&uart2_handler,(u8 *)uart2_rxbuf, UART2_RX_SIZE)!=HAL_OK);//һ�δ������֮�����¿����жϲ�����RxXferCountΪ1
}

void bsp_uart1_rx_interrupt(u8 data)
{
	
}


extern void jmp_gui_uart_command_get(u8 data);
void bsp_uart2_rx_interrupt(u8 data)
{
	jmp_gui_uart_command_get(data);
}


//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}



