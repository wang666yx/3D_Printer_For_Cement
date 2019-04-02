#include "jmp_command_execution.h"
#include "jmp_gcode_analysis.h"
#include "tmsys.h"
#include "bsp_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "planner.h"
#include "jmp_motion.h"
#include "jmp_param_state.h"


/***********************************************************************************************************
��ģ��Ϊ�˶�ָ��ִ��ģ�飬���ڶԽ������ָ����зַ�ִ��
***********************************************************************************************************/

static TaskHandle_t xHandleTask_CommandExec = NULL;

CommandStruct jmp_command_struct;

//jmp_command_struct��պ���
void jmp_command_struct_clear(void)
{
	uint8_t i;
	jmp_command_struct.command_type=0;
	jmp_command_struct.command=0;
	for(i=0;i<COMMAND_EXECUTION_PARAM_SUM;i++)
	{
		jmp_command_struct.param[i]=0;
	}
	jmp_command_struct.param_flag=0;
}

//G0ָ��ִ�к���
void jmp_command_g0_exec(void)
{
	printf("%d ",jmp_config_state_struct.executed_command_sum);
	printf("\r\n");
}
//G1ָ��ִ�к���
void jmp_command_g1_exec(void)
{
	u32 param_flag;
	double x,y,z,e;
	
	printf("%d ",jmp_config_state_struct.executed_command_sum);
	printf("%d ",jmp_config_state_struct.motion_command_sum);
	
	printf("G1");
	
	//��ȡָ������ı�־
	param_flag=jmp_command_struct.param_flag;
	//��ȡ��ǰ��λ��
	x=jmp_config_state_struct.axis_position[0];
	y=jmp_config_state_struct.axis_position[1];
	z=jmp_config_state_struct.axis_position[2];
	e=jmp_config_state_struct.axis_position[3];
	
	//ͨ��ָ���λ�ò��������޸�
	if(param_flag&(1<<0))
	{
		printf(" ");
		printf("X");
		printf("%f",jmp_command_struct.param[0]);
		x=jmp_command_struct.param[0];
	}
	if(param_flag&(1<<1))
	{
		printf(" ");
		printf("Y");
		printf("%f",jmp_command_struct.param[1]);
		y=jmp_command_struct.param[1];
	}
	if(param_flag&(1<<2))
	{
		printf(" ");
		printf("Z");
		printf("%f",jmp_command_struct.param[2]);
		z=jmp_command_struct.param[2];
	}
	if(param_flag&(1<<3))
	{
		printf(" ");
		printf("E");
		printf("%f",jmp_command_struct.param[3]);
		e=jmp_command_struct.param[3];
	}
	if(param_flag&(1<<4))
	{
		printf(" ");
		printf("F");
		printf("%f",jmp_command_struct.param[4]);
		jmp_config_state_struct.feedrate=jmp_command_struct.param[4];
	}
	
	printf("\r\n");
	
	//ִ���ƶ�ָ��
	jmp_motion_prepare_move(x,y,z,e);
}

//G4ָ���
void jmp_command_g4_exec(void)
{
	printf("%d ",jmp_config_state_struct.executed_command_sum);
	printf("G4");
	printf("\r\n");
}

//G28ָ���
void jmp_command_g28_exec(void)
{
	printf("%d ",jmp_config_state_struct.executed_command_sum);
	printf("%d ",jmp_config_state_struct.motion_command_sum);
	printf("G28");
	printf("\r\n");
	//����ԭ��
	plan_buffer_line(0,0,0,0,50,active_extruder);
}

//G90ָ���
void jmp_command_g90_exec(void)
{
	printf("%d ",jmp_config_state_struct.executed_command_sum);
	printf("G90");
	printf("\r\n");
}

//G92ָ���
void jmp_command_g92_exec(void)
{
	u32 param_flag;
	double x,y,z,e;
	printf("%d ",jmp_config_state_struct.executed_command_sum);
	printf("G92");
	
	//��ȡ������־λ
	param_flag=jmp_command_struct.param_flag;
	//��ȡ��ǰλ��
	x=jmp_config_state_struct.axis_position[0];
	y=jmp_config_state_struct.axis_position[1];
	z=jmp_config_state_struct.axis_position[2];
	e=jmp_config_state_struct.axis_position[3];
	//ͨ��ָ�����λ��
	if(param_flag&(1<<0))
	{
		printf(" ");
		printf("X");
		printf("%f",jmp_command_struct.param[0]);
		x=jmp_command_struct.param[0];
	}
	if(param_flag&(1<<1))
	{
		printf(" ");
		printf("Y");
		printf("%f",jmp_command_struct.param[1]);
		y=jmp_command_struct.param[1];
	}
	if(param_flag&(1<<2))
	{
		printf(" ");
		printf("Z");
		printf("%f",jmp_command_struct.param[2]);
		z=jmp_command_struct.param[2];
	}
	if(param_flag&(1<<3))
	{
		printf(" ");
		printf("E");
		printf("%f",jmp_command_struct.param[3]);
		e=jmp_command_struct.param[3];
	}
	
	printf("\r\n");
	//�ȴ�����˶���λ
	while(blocks_queued())
	{
		if(jmp_config_state_struct.printing_abort==1)
		{
			return;
		}
		vTaskDelay(10);
	}
	//���õ��λ��
	jmp_motion_set_postion(x,y,z,e);
}

//M82ָ���
void jmp_command_m82_exec(void)
{
	printf("%d ",jmp_config_state_struct.executed_command_sum);
	printf("M82");
	printf("\r\n");
}

//ָ��ִ�к���
void jmp_command_execution(void)
{
	uint32_t comm;
	uint32_t comm_type;
	comm=jmp_command_struct.command;//��ȡָ���
	comm_type=jmp_command_struct.command_type;//��ȡָ������
	switch(comm_type)
	{
		case G_COMMAND:
		{
			switch(comm)
			{
				case 0://G0
				{
					jmp_command_g0_exec();
					break;
				}
				case 1://G1
				{
					jmp_command_g1_exec();
					jmp_config_state_struct.motion_command_sum++;
					break;
				}
				case 4://G4
				{
					jmp_command_g4_exec();
					break;
				}
				case 28://G28
				{
					jmp_command_g28_exec();
					jmp_config_state_struct.motion_command_sum++;
					break;
				}
				case 90://G90
				{
					jmp_command_g90_exec();
					break;
				}
				case 92://G92
				{
					jmp_command_g92_exec();
					break;
				}
				default:
				{
					break;
				}
			}
			break;
		}
		case M_COMMAND:
		{
			switch(comm)
			{
				case 82://M82
				{
					jmp_command_m82_exec();
					break;
				}
				default:
				{
					break;
				}
			}
			break;
		}
		case T_COMMAND:
		{
			switch(comm)
			{
				default:
				{
					break;
				}
			}
			break;
		}
		case S_COMMAND:
		{
			switch(comm)
			{
				default:
				{
					break;
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}
	//ͳ��ִ�е�ָ������
	jmp_config_state_struct.executed_command_sum++;
}

//ָ��ִ������
static void jmp_command_execution_task(void *pvParameters)
{
	jmp_motion_set_postion(0,0,0,0);//����λ��Ϊ0
	while(1)
	{
		jmp_gcode_analysis();//����gcode
		jmp_command_execution();//ִ��gcode
		vTaskDelay(10);
	}
}
//ָ��ִ��ģ���ʼ��
void jmp_command_execution_init(void)
{
	//����ָ��ִ������
	xTaskCreate( jmp_command_execution_task,
							"jmp_command_execution_task",
								2048,
								NULL,
								3,
								&xHandleTask_CommandExec );
}

