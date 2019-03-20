#include "bsp_spi.h"


SPI_HandleTypeDef spi1_handler;



void bsp_spi1_init(void)
{
	spi1_handler.Instance=SPI1;                         //SP1
	spi1_handler.Init.Mode=SPI_MODE_MASTER;             //����SPI����ģʽ������Ϊ��ģʽ
	spi1_handler.Init.Direction=SPI_DIRECTION_2LINES;   //����SPI�������˫�������ģʽ:SPI����Ϊ˫��ģʽ
	spi1_handler.Init.DataSize=SPI_DATASIZE_8BIT;       //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	spi1_handler.Init.CLKPolarity=SPI_POLARITY_HIGH;    //����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	spi1_handler.Init.CLKPhase=SPI_PHASE_2EDGE;         //����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	spi1_handler.Init.NSS=SPI_NSS_SOFT;                 //NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	spi1_handler.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_256;//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	spi1_handler.Init.FirstBit=SPI_FIRSTBIT_MSB;        //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	spi1_handler.Init.TIMode=SPI_TIMODE_DISABLE;        //�ر�TIģʽ
	spi1_handler.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;//�ر�Ӳ��CRCУ��
	spi1_handler.Init.CRCPolynomial=7;                  //CRCֵ����Ķ���ʽ
	HAL_SPI_Init(&spi1_handler);//��ʼ��
	
	__HAL_SPI_ENABLE(&spi1_handler);                    //ʹ��SPI1

	bsp_spi1_rw_byte(0Xff);                           //��������
}


void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	if(hspi->Instance==SPI1)
	{
    
    BSP_SPI1_SCK_GPIO_CLK_ENABLE();
		BSP_SPI1_MISO_GPIO_CLK_ENABLE();
		BSP_SPI1_MOSI_GPIO_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();        //ʹ��SPI1ʱ��
    
    GPIO_Initure.Pin=BSP_SPI1_SCK_GPIO_PIN;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //�����������
    GPIO_Initure.Pull=GPIO_PULLUP;                  //����
    GPIO_Initure.Speed=GPIO_SPEED_FAST;             //����            
    GPIO_Initure.Alternate=GPIO_AF5_SPI1;           //����ΪSPI1
    HAL_GPIO_Init(BSP_SPI1_SCK_GPIO,&GPIO_Initure);
		
		GPIO_Initure.Pin=BSP_SPI1_MISO_GPIO_PIN;
		HAL_GPIO_Init(BSP_SPI1_MISO_GPIO,&GPIO_Initure);
		
		GPIO_Initure.Pin=BSP_SPI1_MOSI_GPIO_PIN;
		HAL_GPIO_Init(BSP_SPI1_MOSI_GPIO,&GPIO_Initure);
	}
}

void bsp_spi1_set_speed(u8 spi_baud)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(spi_baud));//�ж���Ч��
    __HAL_SPI_DISABLE(&spi1_handler);            //�ر�SPI
    spi1_handler.Instance->CR1&=0XFFC7;          //λ3-5���㣬�������ò�����
    spi1_handler.Instance->CR1|=spi_baud;//����SPI�ٶ�
    __HAL_SPI_ENABLE(&spi1_handler);             //ʹ��SPI
}

u8 bsp_spi1_rw_byte(u8 tx)
{
	u8 rx;
	HAL_SPI_TransmitReceive(&spi1_handler,&tx,&rx,1, 1000);       
 	return rx;          		    //�����յ�������		
}




