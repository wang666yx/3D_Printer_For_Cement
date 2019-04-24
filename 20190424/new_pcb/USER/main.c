#include "tmsys.h"
#include "tmdelay.h" 
#include "bsp_uart.h"
#include "bsp_gpio.h"
#include "bsp_sdram.h"
#include "bsp_test.h"
#include "bsp_exti.h"
#include "bsp_mpu.h"
#include "usb_test.h"
#include "usb_app.h"
#include "w25qxx.h"

#include "app_qspi_flash_fatfs.h"
#include "app_nand_flash_fatfs.h"
#include "app_beep.h"

#include "tmmalloc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"

#include "jmp_config.h"
#include "jmp_storage_read.h"
#include "jmp_gcode_analysis.h"
#include "jmp_command_execution.h"
#include "jmp_param_state.h"
#include "jmp_gui.h"
#include "jmp_motion.h"
#include "jmp_print_time.h"

static TaskHandle_t xHandleTaskLED = NULL;

//HAL_InitTick���壬����freeRTOS
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  return HAL_OK;
}

//HAL_Delay����
void HAL_Delay(uint32_t Delay)
{
	tmdelay_ms(Delay);
}


//LED���е�������˸��ʾϵͳ�ڹ���
static void vTaskLED(void *pvParameters)
{
	while(1)
	{
		bsp_led_run_control(1);
		vTaskDelay(1000);
		bsp_led_run_control(0);
		vTaskDelay(1000);
	}
}

int main(void)
{
	__set_PRIMASK(1);
	Cache_Enable();                             //��L1-Cache
	MPU_Memory_Protection();
	HAL_Init();				                    //��ʼ��HAL��
	Stm32_Clock_Init(160,5,2,4);                //����ʱ��,400Mhz 
	tmdelay_init();
	
	//tmdelay_ms(20000);
	
	bsp_uart1_init(115200);//��ʼ������
	bsp_uart2_init(115200);
	bsp_uart3_init(115200);
	bsp_uart4_init(115200);

	bsp_gpio_init();//��ʼ��gpio
	bsp_sdram_init();//��ʼ��sdram
	
	bsp_exti_init();//��ʼ���ⲿ�ж�
	
	while(1)
	{
		if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14)==1)
		{
			tmdelay_ms(10);
		}
		else
		{
			break;
		}
	}
	
	usb_app_init();//��ʼ��USB
	
//	W25QXX_Init();
//	W25QXX_ReadSR(1);
//	W25QXX_ReadSR(2);
//	W25QXX_ReadSR(3);
//	W25QXX_Write_Unlock();
//	W25QXX_ReadSR(1);
//	W25QXX_ReadSR(2);
//	W25QXX_ReadSR(3);
	
//	W25QXX_Erase_Chip();

	tm_mem_init(SRAMIN);		    		//��ʼ���ڲ��ڴ��
	tm_mem_init(SRAMEX);		    		//��ʼ���ⲿ�ڴ��
	tm_mem_init(SRAMDTCM);		    		//��ʼ��CCM�ڴ�� 
	
	app_qspi_flash_fatfs_init();//��ʼ��spi flash
	app_nand_flash_fatfs_init();//��ʼ��nand flash
	app_beep_init();

	jmp_ex_config_init();//�ⲿ���ó�ʼ��
	jmp_configstate_init();//����״̬��ʼ��
	jmp_storage_read_init();//�洢��ȡ��ʼ��
	jmp_command_execution_init();//����ִ�г�ʼ��
	jmp_gui_init();//GUI��ʼ��
	jmp_motion_init();//�˶�ģ���ʼ��
	jmp_print_time_init();//��ӡʱ���ʼ��
	
	//����LED���е�����
	xTaskCreate( vTaskLED,
							"vTaskLED",
								512,
								NULL,
								2,
								&xHandleTaskLED );
								
	//��ʼϵͳ����
	vTaskStartScheduler();
	
	while(1)
	{
	}
}
