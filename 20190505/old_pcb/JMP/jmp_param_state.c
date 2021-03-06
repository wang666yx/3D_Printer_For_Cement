#include "jmp_param_state.h"




JMPCofigParamStruct jmp_config_param_struct;

JMPCofigStateStruct jmp_config_state_struct;


void jmp_config_param_init(void)
{
}

void jmp_config_state_init(void)
{
	u8 i;
	jmp_config_state_struct.manual_step_length=10;
	
	jmp_config_state_struct.printing_run=0;
	jmp_config_state_struct.printing_hold=0;
	jmp_config_state_struct.printing_abort=0;
	jmp_config_state_struct.reading_end=0;
	
	jmp_config_state_struct.xy_axis_speed=33.3;
	jmp_config_state_struct.z_axis_speed=25.9;
	jmp_config_state_struct.e_axis_speed=24.7;
	
	jmp_config_state_struct.print_progress=0;
	
	jmp_config_state_struct.speed_range=100;
	
	for(i=0;i<4;i++)
	{
		jmp_config_state_struct.axis_position[i]=0;
	}
	jmp_config_state_struct.feedrate=300;
}

void jmp_configstate_init(void)
{
	jmp_config_param_init();
	jmp_config_state_init();
}



