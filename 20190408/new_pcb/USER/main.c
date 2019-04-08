#include "tmsys.h"
#include "tmdelay.h" 
#include "bsp_uart.h"
#include "bsp_gpio.h"
#include "bsp_sdram.h"
#include "bsp_test.h"
#include "usb_test.h"
#include "usb_app.h"

#include "app_qspi_flash_fatfs.h"

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
	HAL_Init();				                    //��ʼ��HAL��
	Stm32_Clock_Init(160,5,2,4);                //����ʱ��,400Mhz 
	tmdelay_init();
	
	bsp_uart1_init(115200);//��ʼ������
	bsp_uart2_init(115200);
	bsp_uart3_init(115200);
	bsp_uart4_init(115200);

	bsp_gpio_init();//��ʼ��gpio
	bsp_sdram_init();//��ʼ��sdram
	
	usb_app_init();//��ʼ��USB
	
	app_qspi_flash_fatfs_init();//��ʼ��spi flash
	
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
