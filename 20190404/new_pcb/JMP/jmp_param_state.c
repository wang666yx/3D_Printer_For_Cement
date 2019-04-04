#include "jmp_param_state.h"
#include "jmp_ex_config.h"


/***********************************************************************************************************
��ģ��Ϊ��������״̬ģ�飬���ڷ���һЩȫ�ֱ�����ȫ��״̬
***********************************************************************************************************/

JMPCofigParamStruct jmp_config_param_struct;//ȫ�ֲ���

JMPCofigStateStruct jmp_config_state_struct;//ȫ�ֱ���

//ȫ�ֲ�����ʼ��
void jmp_config_param_init(void)
{
	
}

//ȫ�ֱ�����ʼ��
void jmp_config_state_init(void)
{
	u8 i;
	jmp_config_state_struct.manual_step_length=10;//�ֶ���������
	
	jmp_config_state_struct.printing_run=0;//��ӡ����
	jmp_config_state_struct.printing_hold=0;//��ӡ��ͣ
	jmp_config_state_struct.printing_abort=0;//��ӡ�ж�
	jmp_config_state_struct.reading_end=0;//��ӡ����
	
	jmp_config_state_struct.xy_axis_speed=0;//xy���ٶ�
	jmp_config_state_struct.z_axis_speed=0;//z���ٶ�
	jmp_config_state_struct.e_axis_speed=0;//e���ٶ�
	
	jmp_config_state_struct.print_progress=0;//��ӡ����
	
	jmp_config_state_struct.speed_range=100;//�ٶȰٷֱ�
	jmp_config_state_struct.speed_range_e=100;//�����ٶȰٷֱ�
	
	jmp_config_state_struct.e_ex_print_speed=jmp_ex_config_struct.e_max_feedrate;//Ԥ��ӡ�ٶ�
	jmp_config_state_struct.e_ex_print_run=0;//Ԥ��ӡ����
	
	jmp_config_state_struct.homing_stop=0;//��ԭ��ֹͣ
	jmp_config_state_struct.script_stop=0;//�ű�ֹͣ
	jmp_config_state_struct.block_manage_stop=0;//block_manageֹͣ
	
	jmp_config_state_struct.executed_command_sum=0;//ִ��ָ������
	jmp_config_state_struct.motion_command_sum=0;//�˶�ָ������
	jmp_config_state_struct.n_motion_command_sum=0;//��Ч�˶�ָ������
	jmp_config_state_struct.current_command_num=0;//��ǰָ����
	
	for(i=0;i<4;i++)
	{
		jmp_config_state_struct.axis_position[i]=0;//λ��
	}
	jmp_config_state_struct.feedrate=300;//�ٶ�
}

//ģ���ʼ��
void jmp_configstate_init(void)
{
	jmp_config_param_init();
	jmp_config_state_init();
}



