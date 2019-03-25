#include "planner.h"
#include "stepper.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "jmp_ex_config.h"
#include "jmp_param_state.h"




//===========================================================================
//=============================public variables ============================
//===========================================================================

float max_feedrate[4];
float axis_steps_per_unit[4];
unsigned long max_acceleration_units_per_sq_second[4];
unsigned long axis_steps_per_sqr_second[NUM_AXIS];

unsigned long minsegmenttime=DEFAULT_MINSEGMENTTIME;
float minimumfeedrate=DEFAULT_MINIMUMFEEDRATE;
float acceleration;         // Normal acceleration mm/s^2  THIS IS THE DEFAULT ACCELERATION for all moves. M204 SXXXX
float retract_acceleration; //  mm/s^2   filament pull-pack and push-forward  while standing still in the other axis M204 TXXXX
float max_xy_jerk; //speed than can be stopped at once, if i understand correctly.
float max_z_jerk;
float max_e_jerk;
float mintravelfeedrate=DEFAULT_MINTRAVELFEEDRATE;



// The current position of the tool in absolute steps
long position[4];   //rescaled from extern when axis_steps_per_unit are changed by gcode
static float previous_speed[4]; // Speed of previous path line segment
static float previous_nominal_speed; // Nominal speed of previous path line segment




//===========================================================================
//=================semi-private variables, used in inline  functions    =====
//===========================================================================
block_t block_buffer[BLOCK_BUFFER_SIZE];            // A ring buffer for motion instfructions
volatile unsigned char block_buffer_head;           // Index of the next block to be pushed
volatile unsigned char block_buffer_tail;           // Index of the block to process now



//===========================================================================
//=============================private variables ============================
//===========================================================================

#ifdef XY_FREQUENCY_LIMIT
#define MAX_FREQ_TIME (1000000.0/XY_FREQUENCY_LIMIT)
// Used for the frequency limit
static unsigned char old_direction_bits = 0;               // Old direction bits. Used for speed calculations
static long x_segment_time[3]={MAX_FREQ_TIME + 1,0,0};     // Segment times (in us). Used for speed calculations
static long y_segment_time[3]={MAX_FREQ_TIME + 1,0,0};
#endif


//������һ��block�����
int8_t next_block_index(int8_t block_index) 
{
  block_index++;
  if (block_index == BLOCK_BUFFER_SIZE) 
	{ 
    block_index = 0; 
  }
  return(block_index);
}


//������һ��block�����
int8_t prev_block_index(int8_t block_index) 
{
  if (block_index == 0) 
	{ 
    block_index = BLOCK_BUFFER_SIZE; 
  }
  block_index--;
  return(block_index);
}

//===========================================================================
//=============================functions         ============================
//===========================================================================

// Calculates the distance (not time) it takes to accelerate from initial_rate to target_rate using the 
// given acceleration:	
// ���ݳ��ٶȣ�Ŀ���ٶȺ͸����ļ��ٶȼ������
float estimate_acceleration_distance(float initial_rate, float target_rate, float acceleration)
{
  if (acceleration!=0) 
	{
    return((target_rate*target_rate-initial_rate*initial_rate)/
      (2.0f*acceleration));
  }
  else 
	{
    return 0.0;  // acceleration was 0, set acceleration distance to 0
  }
}

// This function gives you the point at which you must start braking (at the rate of -acceleration) if 
// you started at speed initial_rate and accelerated until this point and want to end at the final_rate after
// a total travel of distance. This can be used to compute the intersection point between acceleration and
// deceleration in the cases where the trapezoid has no plateau (i.e. never reaches maximum speed)
//��û�����ٶε�����£����ݳ��ٶȣ�ĩ�ٶȣ����ٶȺ;��������ٵ��λ��
float intersection_distance(float initial_rate, float final_rate, float acceleration, float distance) 
{
  if (acceleration!=0) 
	{
    return((2.0f*acceleration*distance-initial_rate*initial_rate+final_rate*final_rate)/
      (4.0f*acceleration) );
  }
  else 
	{
    return 0.0;  // acceleration was 0, set intersection distance to 0
  }
}

// Calculates trapezoid parameters so that the entry- and exit-speed is compensated by the provided factors.
//�������β���
void calculate_trapezoid_for_block(block_t *block, float entry_factor, float exit_factor) 
{
	//�����ٶȻ������ӻ�����ٺ�ĩ��
  unsigned long initial_rate = ceil(block->nominal_rate*entry_factor); // (step/min)
  unsigned long final_rate = ceil(block->nominal_rate*exit_factor); // (step/min)
  unsigned long acceleration = block->acceleration_st;
  #ifdef ADVANCE
  volatile long initial_advance; 
  volatile long final_advance;
  #endif // ADVANCE

	//������٣����ٺ����ٶ�Ӧ�Ĳ���
  int32_t accelerate_steps =   ceil(estimate_acceleration_distance(block->initial_rate, block->nominal_rate,(float)acceleration));
  int32_t decelerate_steps =   floor(estimate_acceleration_distance(block->nominal_rate, block->final_rate, -(float)acceleration));
  // Calculate the size of Plateau of Nominal Rate.
  int32_t plateau_steps = block->step_event_count-accelerate_steps-decelerate_steps;

  if(initial_rate <120) 
  {
    initial_rate=120; 
  }
  if(final_rate < 120) 
  {
    final_rate=120;  
  }

  // Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
  // have to use intersection_distance() to calculate when to abort acceleration and start braking
  // in order to reach the final_rate exactly at the end of this block.
	//�������û�����ٶΣ����¼���
  if (plateau_steps < 0) 
	{
    accelerate_steps = ceil(intersection_distance(block->initial_rate, block->final_rate, acceleration, block->step_event_count));
    accelerate_steps = max(accelerate_steps,0); // Check limits due to numerical round-off
    accelerate_steps = min((uint32_t)accelerate_steps,block->step_event_count);//(We can cast here to unsigned, because the above line ensures that we are above zero)
    plateau_steps = 0;
  }

#ifdef ADVANCE
  initial_advance = block->advance*entry_factor*entry_factor; 
  final_advance = block->advance*exit_factor*exit_factor;
#endif // ADVANCE

  // block->accelerate_until = accelerate_steps;
  // block->decelerate_after = accelerate_steps+plateau_steps;
  CRITICAL_SECTION_START;  // Fill variables used by the stepper in a critical section
  if(block->busy == false) 
	{ // Don't update variables if block is busy.
    block->accelerate_until = accelerate_steps;
    block->decelerate_after = accelerate_steps+plateau_steps;
    block->initial_rate = initial_rate;
    block->final_rate = final_rate;
#ifdef ADVANCE
    block->initial_advance = initial_advance;
    block->final_advance = final_advance;
#endif //ADVANCE
  }
  CRITICAL_SECTION_END;
}

// Calculates the maximum allowable speed at this point when you must be able to reach target_velocity using the 
// acceleration within the allotted distance.
//���ݾ��룬���ٶȣ�ĩ�٣��������
float max_allowable_speed(float acceleration, float target_velocity, float distance) 
{
  return  sqrt(target_velocity*target_velocity-2*acceleration*distance);
}

// The kernel called by planner_recalculate() when scanning the plan from last to first entry.
//������һ��block��entry_speed������ǰblock��entry_speed
void planner_reverse_pass_kernel(block_t *previous, block_t *current, block_t *next) 
{
  if(!current) 
	{ 
    return; 
  }
  if (next) 
	{
    // If entry speed is already at the maximum entry speed, no need to recheck. Block is cruising.
    // If not, block in state of acceleration or deceleration. Reset entry speed to maximum and
    // check for maximum allowable speed reductions to ensure maximum possible planned speed.
    if (current->entry_speed != current->max_entry_speed) 
		{

      // If nominal length true, max junction speed is guaranteed to be reached. Only compute
      // for max allowable speed if block is decelerating and nominal length is false.
      if ((!current->nominal_length_flag) && (current->max_entry_speed > next->entry_speed)) 
			{
        current->entry_speed = min( current->max_entry_speed,
        max_allowable_speed(-current->acceleration,next->entry_speed,current->millimeters));
      } 
      else 
			{
        current->entry_speed = current->max_entry_speed;
      }
      current->recalculate_flag = true;

    }
  } // Skip last block. Already initialized and set for recalculation.
}

// planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This 
// implements the reverse pass.
void planner_reverse_pass() 
{
  uint8_t block_index = block_buffer_head;
  unsigned char tail = block_buffer_tail;
  //Make a local copy of block_buffer_tail, because the interrupt can alter it
  CRITICAL_SECTION_START;
 
  CRITICAL_SECTION_END
  
  if(((block_buffer_head-tail + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1)) > 3) 
  {
		block_t *block[3] = {NULL, NULL, NULL};
    block_index = (block_buffer_head - 3) & (BLOCK_BUFFER_SIZE - 1);
    while(block_index != tail) 
		{ 
      block_index = prev_block_index(block_index); 
      block[2]= block[1];
      block[1]= block[0];
      block[0] = &block_buffer[block_index];
      planner_reverse_pass_kernel(block[0], block[1], block[2]);
    }
  }
}

// The kernel called by planner_recalculate() when scanning the plan from first to last entry.
void planner_forward_pass_kernel(block_t *previous, block_t *current, block_t *next) 
{
  if(!previous) 
	{ 
    return; 
  }

  // If the previous block is an acceleration block, but it is not long enough to complete the
  // full speed change within the block, we need to adjust the entry speed accordingly. Entry
  // speeds have already been reset, maximized, and reverse planned by reverse planner.
  // If nominal length is true, max junction speed is guaranteed to be reached. No need to recheck.
  if (!previous->nominal_length_flag) 
	{
    if (previous->entry_speed < current->entry_speed) 
		{
      double entry_speed = min( current->entry_speed,
      max_allowable_speed(-previous->acceleration,previous->entry_speed,previous->millimeters) );

      // Check for junction speed change
      if (current->entry_speed != entry_speed) 
			{
        current->entry_speed = entry_speed;
        current->recalculate_flag = true;
      }
    }
  }
}

// planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This 
// implements the forward pass.
void planner_forward_pass() 
{
  uint8_t block_index = block_buffer_tail;
  block_t *block[3] = {NULL, NULL, NULL};

  while(block_index != block_buffer_head) 
	{
    block[0] = block[1];
    block[1] = block[2];
    block[2] = &block_buffer[block_index];
    planner_forward_pass_kernel(block[0],block[1],block[2]);
    block_index = next_block_index(block_index);
  }
  planner_forward_pass_kernel(block[1], block[2], NULL);
}

// Recalculates the trapezoid speed profiles for all blocks in the plan according to the 
// entry_factor for each junction. Must be called by planner_recalculate() after 
// updating the blocks.
void planner_recalculate_trapezoids() 
{
  int8_t block_index = block_buffer_tail;
  block_t *current;
  block_t *next = NULL;

  while(block_index != block_buffer_head) 
	{
    current = next;
    next = &block_buffer[block_index];
    if (current) 
		{
      // Recalculate if current block entry or exit junction speed has changed.
      if (current->recalculate_flag || next->recalculate_flag) 
			{
        // NOTE: Entry and exit factors always > 0 by all previous logic operations.
        calculate_trapezoid_for_block(current, current->entry_speed/current->nominal_speed,
        next->entry_speed/current->nominal_speed);
        current->recalculate_flag = false; // Reset current only to ensure next trapezoid is computed
      }
    }
    block_index = next_block_index( block_index );
  }
  // Last/newest block in buffer. Exit speed is set with MINIMUM_PLANNER_SPEED. Always recalculated.
  if(next != NULL) 
	{
    calculate_trapezoid_for_block(next, next->entry_speed/next->nominal_speed,
    MINIMUM_PLANNER_SPEED/next->nominal_speed);
    next->recalculate_flag = false;
  }
}

// Recalculates the motion plan according to the following algorithm:
//
//   1. Go over every block in reverse order and calculate a junction speed reduction (i.e. block_t.entry_factor) 
//      so that:
//     a. The junction jerk is within the set limit
//     b. No speed reduction within one block requires faster deceleration than the one, true constant 
//        acceleration.
//   2. Go over every block in chronological order and dial down junction speed reduction values if 
//     a. The speed increase within one block would require faster accelleration than the one, true 
//        constant acceleration.
//
// When these stages are complete all blocks have an entry_factor that will allow all speed changes to 
// be performed using only the one, true constant acceleration, and where no junction jerk is jerkier than 
// the set limit. Finally it will:
//
//   3. Recalculate trapezoids for all blocks.

void planner_recalculate(void) 
{   
  planner_reverse_pass();
  planner_forward_pass();
  planner_recalculate_trapezoids();
}


void planner_variable_init(void)
{
	max_feedrate[0]=(float)jmp_ex_config_struct.x_max_feedrate;
	max_feedrate[1]=(float)jmp_ex_config_struct.y_max_feedrate;
	max_feedrate[2]=(float)jmp_ex_config_struct.z_max_feedrate;
	max_feedrate[3]=(float)jmp_ex_config_struct.e_max_feedrate;
	
	axis_steps_per_unit[0]=(float)jmp_ex_config_struct.x_step_per_mm;
	axis_steps_per_unit[1]=(float)jmp_ex_config_struct.y_step_per_mm;
	axis_steps_per_unit[2]=(float)jmp_ex_config_struct.z_step_per_mm;
	axis_steps_per_unit[3]=(float)jmp_ex_config_struct.e_step_per_mm;
	
	max_acceleration_units_per_sq_second[0]=(unsigned long)jmp_ex_config_struct.x_max_acc;
	max_acceleration_units_per_sq_second[1]=(unsigned long)jmp_ex_config_struct.y_max_acc;
	max_acceleration_units_per_sq_second[2]=(unsigned long)jmp_ex_config_struct.z_max_acc;
	max_acceleration_units_per_sq_second[3]=(unsigned long)jmp_ex_config_struct.e_max_acc;
	
	acceleration=(float)jmp_ex_config_struct.default_acc;
	retract_acceleration=(float)jmp_ex_config_struct.default_retract_acc;
	max_xy_jerk=(float)jmp_ex_config_struct.default_xy_jerk;
	max_z_jerk=(float)jmp_ex_config_struct.default_z_jerk;
	max_e_jerk=(float)jmp_ex_config_struct.default_e_jerk;
}

void plan_init(void) 
{
  block_buffer_head = 0;
  block_buffer_tail = 0;
  memset(position, 0, sizeof(position)); // clear position
  previous_speed[0] = 0.0;
  previous_speed[1] = 0.0;
  previous_speed[2] = 0.0;
  previous_speed[3] = 0.0;
  previous_nominal_speed = 0.0;
	planner_variable_init();
}


void check_axes_activity(void)
{
  unsigned char x_active = 0;
  unsigned char y_active = 0;  
  unsigned char z_active = 0;
  unsigned char e_active = 0;

  block_t *block;

  if(block_buffer_tail != block_buffer_head)
  {
    uint8_t block_index = block_buffer_tail;
    while(block_index != block_buffer_head)
    {
      block = &block_buffer[block_index];
      if(block->steps_x != 0) x_active++;
      if(block->steps_y != 0) y_active++;
      if(block->steps_z != 0) z_active++;
      if(block->steps_e != 0) e_active++;
      block_index = (block_index+1) & (BLOCK_BUFFER_SIZE - 1);
    }
  }
  if((DISABLE_X) && (x_active == 0)) disable_x(); 
  if((DISABLE_Y) && (y_active == 0)) disable_y();
  if((DISABLE_Z) && (z_active == 0)) disable_z();
  if((DISABLE_E) && (e_active == 0))
  {
    disable_e0();
    disable_e1();
  }
}

#ifdef COREXY
float plan_convert_corexy_to_x_axis_steps(float x,float y)
{
	return((x+y)/2);
}

float plan_convert_corexy_to_y_axis_steps(float x,float y)
{
	return((x-y)/2);
}
#endif


float junction_deviation = 0.1;
// Add a new linear movement to the buffer. steps_x, _y and _z is the absolute position in 
// mm. Microseconds specify how many microseconds the move should take to perform. To aid acceleration
// calculation the caller must also provide the physical length of the line in millimeters.
void plan_buffer_line(const float x, const float y, const float z, const float e, float feed_rate, const uint8_t extruder)
{
  // Calculate the buffer head after we push this byte
  int next_buffer_head = next_block_index(block_buffer_head);
  long target[4];
  block_t *block ;
  float delta_mm[4];
  float inverse_millimeters;
  float inverse_second;
  int moves_queued;
  float current_speed[4];
  float speed_factor;
  int i;
  float steps_per_mm; 
  float vmax_junction ; 
  float vmax_junction_factor ;
  double v_allowable ;
   unsigned long segment_time;
	float safe_speed;
  // If the buffer is full: good! That means we are well ahead of the robot. 
  // Rest here until there is room in the buffer.
  while(block_buffer_tail == next_buffer_head)
  {
		if(jmp_config_state_struct.printing_abort==1)
		{
			return;
		}
    vTaskDelay(10);
  }

  // The target position of the tool in absolute steps
  // Calculate target position in absolute steps
  //this should be done after the wait, because otherwise a M92 code within the gcode disrupts this calculation somehow
	//������������µ�Ŀ��λ�õĲ���
	#ifdef COREXY
	target[X_AXIS]=round(plan_convert_corexy_to_x_axis_steps(x,y)*axis_steps_per_unit[X_AXIS]);
	target[Y_AXIS]=round(plan_convert_corexy_to_y_axis_steps(x,y)*axis_steps_per_unit[Y_AXIS]);
	#else
  target[X_AXIS] = round(x*axis_steps_per_unit[X_AXIS]);
  target[Y_AXIS] = round(y*axis_steps_per_unit[Y_AXIS]);
	#endif
  target[Z_AXIS] = round(z*axis_steps_per_unit[Z_AXIS]);     
  target[E_AXIS] = round(e*axis_steps_per_unit[E_AXIS]);

  // Prepare to set up new block
  block = &block_buffer[block_buffer_head];

  // Mark block as not busy (Not executed by the stepper interrupt)
  block->busy = false;

  // Number of steps for each axis
	//���������Ҫ�ƶ��Ĳ���
  block->steps_x = labs(target[X_AXIS]-position[X_AXIS]);
  block->steps_y = labs(target[Y_AXIS]-position[Y_AXIS]);
  block->steps_z = labs(target[Z_AXIS]-position[Z_AXIS]);
  block->steps_e = labs(target[E_AXIS]-position[E_AXIS]);
  block->steps_e *= extrudemultiply;
  block->steps_e /= 100;
  block->step_event_count = max(block->steps_x, max(block->steps_y, max(block->steps_z, block->steps_e)));
  #ifdef DEBUG_PRINTF 
	printf("\r\nX:%ld\r\n",block->steps_x);
	printf("Y:%ld\r\n",block->steps_y);
	printf("Z:%ld\r\n",block->steps_z);
	printf("E:%ld\r\n",block->steps_e);
	printf("block->step_event_count:%ld\r\n",block->step_event_count);
   #endif
  // Bail if this is a zero-length block

	//�����Ĳ�����С����С�������򷵻أ�����
  if (block->step_event_count <= dropsegments)
  { 
    return; 
  }
  #ifdef DEBUG_PRINTF 
	printf("continue!\r\n");
   #endif

  block->fan_speed = fanSpeed;

  // Compute direction bits for this block 
	//ȷ����������˶�����
  block->direction_bits = 0;
  if (target[X_AXIS] < position[X_AXIS])
  {
    block->direction_bits |= (1<<X_AXIS); 
  }
  if (target[Y_AXIS] < position[Y_AXIS])
  {
    block->direction_bits |= (1<<Y_AXIS); 
  }
  if (target[Z_AXIS] < position[Z_AXIS])
  {
    block->direction_bits |= (1<<Z_AXIS); 
  }
  if (target[E_AXIS] < position[E_AXIS])
  {
    block->direction_bits |= (1<<E_AXIS); 
  }

  block->active_extruder = extruder;

  //enable active axes
	//�����˶���ʹ�ܸ���
  #ifdef COREXY
  if((block->steps_x != 0) || (block->steps_y != 0))
  {	
    enable_x();
    enable_y();
  }
  #else
  if(block->steps_x != 0) 
  {
		enable_x();
   #ifdef DEBUG_PRINTF 
		printf("enable_x\n");
   #endif
  }
  if(block->steps_y != 0) 
  {
		enable_y();
    #ifdef DEBUG_PRINTF 		
		#endif
  }
  #endif
#ifndef Z_LATE_ENABLE
  if(block->steps_z != 0) enable_z();
#endif

  // Enable all
  if(block->steps_e != 0)
  {
    enable_e0();
    enable_e1();
  }

  if (block->steps_e == 0)
  {
    if(feed_rate<mintravelfeedrate) feed_rate=mintravelfeedrate;
  }
  else
  {
    if(feed_rate<minimumfeedrate) feed_rate=minimumfeedrate;
  } 
	//����������ƶ��ĳ��Ⱥ��ƶ����ܳ���
  delta_mm[X_AXIS] = (target[X_AXIS]-position[X_AXIS])/axis_steps_per_unit[X_AXIS];
  delta_mm[Y_AXIS] = (target[Y_AXIS]-position[Y_AXIS])/axis_steps_per_unit[Y_AXIS];
  delta_mm[Z_AXIS] = (target[Z_AXIS]-position[Z_AXIS])/axis_steps_per_unit[Z_AXIS];
  delta_mm[E_AXIS] = ((target[E_AXIS]-position[E_AXIS])/axis_steps_per_unit[E_AXIS])*extrudemultiply/100.0f;
  if ( block->steps_x <=dropsegments && block->steps_y <=dropsegments && block->steps_z <=dropsegments )
  {
    block->millimeters = fabs(delta_mm[E_AXIS]);
  } 
  else
  {
    block->millimeters = sqrt(square(delta_mm[X_AXIS]) + square(delta_mm[Y_AXIS]) + square(delta_mm[Z_AXIS]));
  }
  inverse_millimeters = 1.0f/block->millimeters;  // Inverse millimeters to remove multiple divides 

    // Calculate speed in mm/second for each axis. No divide by zero due to previous checks.
  inverse_second = feed_rate * inverse_millimeters;

	//��buff���ٵ�һ���̶�ʱ���ʵ������ٶ�
  moves_queued=(block_buffer_head-block_buffer_tail + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1);

  // slow down when de buffer starts to empty, rather than wait at the corner for a buffer refill
#ifdef OLD_SLOWDOWN
  if(moves_queued < (BLOCK_BUFFER_SIZE * 0.5) && moves_queued > 1)
    feed_rate = feed_rate*moves_queued / (BLOCK_BUFFER_SIZE * 0.5); 
#endif

#ifdef SLOWDOWN
  //  segment time im micro seconds
  segment_time = round(1000000.0f/inverse_second);
  if ((moves_queued > 1) && (moves_queued < (BLOCK_BUFFER_SIZE * 0.5)))
  {
    if (segment_time < minsegmenttime)
    { // buffer is draining, add extra time.  The amount of time added increases if the buffer is still emptied more.
      inverse_second=1000000.0/(segment_time+round(2*(float)(minsegmenttime-segment_time)/moves_queued));
      #ifdef XY_FREQUENCY_LIMIT
         segment_time = round(1000000.0/inverse_second);
      #endif
    }
  }
#endif
  //  END OF SLOW DOWN SECTION    


  block->nominal_speed = block->millimeters * inverse_second; // (mm/sec) Always > 0
  block->nominal_rate = ceil(block->step_event_count * inverse_second); // (step/sec) Always > 0

  // Calculate and limit speed in mm/sec for each axis
	//����һ�ᳬ���޶�ֵ����������Ҳһͬ�ܵ����ƣ�ͨ��һ��speed_factor���ӿ���
  speed_factor = 1.0; //factor <=1 do decrease speed
  for(i=0; i < 4; i++)
  {
    current_speed[i] = delta_mm[i] * inverse_second;
    if(fabs(current_speed[i]) > max_feedrate[i])
		{
      speed_factor = min(speed_factor, max_feedrate[i] / fabs(current_speed[i]));
		}
  }

  // Max segement time in us.
#ifdef XY_FREQUENCY_LIMIT
#define MAX_FREQ_TIME (1000000.0/XY_FREQUENCY_LIMIT)
  // Check and limit the xy direction change frequency
  unsigned char direction_change = block->direction_bits ^ old_direction_bits;
  old_direction_bits = block->direction_bits;
  segment_time = lround((float)segment_time / speed_factor);
  
  if((direction_change & (1<<X_AXIS)) == 0)
  {
    x_segment_time[0] += segment_time;
  }
  else
  {
    x_segment_time[2] = x_segment_time[1];
    x_segment_time[1] = x_segment_time[0];
    x_segment_time[0] = segment_time;
  }
  if((direction_change & (1<<Y_AXIS)) == 0)
  {
    y_segment_time[0] += segment_time;
  }
  else
  {
    y_segment_time[2] = y_segment_time[1];
    y_segment_time[1] = y_segment_time[0];
    y_segment_time[0] = segment_time;
  }
  long max_x_segment_time = max(x_segment_time[0], max(x_segment_time[1], x_segment_time[2]));
  long max_y_segment_time = max(y_segment_time[0], max(y_segment_time[1], y_segment_time[2]));
  long min_xy_segment_time =min(max_x_segment_time, max_y_segment_time);
  if(min_xy_segment_time < MAX_FREQ_TIME)
    speed_factor = min(speed_factor, speed_factor * (float)min_xy_segment_time / (float)MAX_FREQ_TIME);
#endif

  // Correct the speed 
	//ͨ��speed_factor���¼�������ٶ�
  if( speed_factor < 1.0f)
  {
    for(i=0; i < 4; i++)
    {
      current_speed[i] *= speed_factor;
    }
    block->nominal_speed *= speed_factor;
    block->nominal_rate *= speed_factor;
  }

  // Compute and limit the acceleration rate for the trapezoid generator.  
  steps_per_mm = block->step_event_count/block->millimeters;
  if(block->steps_x == 0 && block->steps_y == 0 && block->steps_z == 0)
  {
    block->acceleration_st = ceil(retract_acceleration * steps_per_mm); // convert to: acceleration steps/sec^2
  }
  else
  {
    block->acceleration_st = ceil(acceleration * steps_per_mm); // convert to: acceleration steps/sec^2
    // Limit acceleration per axis
    if(((float)block->acceleration_st * (float)block->steps_x / (float)block->step_event_count) > axis_steps_per_sqr_second[X_AXIS])
      block->acceleration_st = axis_steps_per_sqr_second[X_AXIS];
    if(((float)block->acceleration_st * (float)block->steps_y / (float)block->step_event_count) > axis_steps_per_sqr_second[Y_AXIS])
      block->acceleration_st = axis_steps_per_sqr_second[Y_AXIS];
    if(((float)block->acceleration_st * (float)block->steps_e / (float)block->step_event_count) > axis_steps_per_sqr_second[E_AXIS])
      block->acceleration_st = axis_steps_per_sqr_second[E_AXIS];
    if(((float)block->acceleration_st * (float)block->steps_z / (float)block->step_event_count ) > axis_steps_per_sqr_second[Z_AXIS])
      block->acceleration_st = axis_steps_per_sqr_second[Z_AXIS];
  }
  block->acceleration = block->acceleration_st / steps_per_mm;
  block->acceleration_rate = (long)((float)block->acceleration_st * 8.388608f);

#if 0  // Use old jerk for now
  // Compute path unit vector
  double unit_vec[3];

  unit_vec[X_AXIS] = delta_mm[X_AXIS]*inverse_millimeters;
  unit_vec[Y_AXIS] = delta_mm[Y_AXIS]*inverse_millimeters;
  unit_vec[Z_AXIS] = delta_mm[Z_AXIS]*inverse_millimeters;

  // Compute maximum allowable entry speed at junction by centripetal acceleration approximation.
  // Let a circle be tangent to both previous and current path line segments, where the junction
  // deviation is defined as the distance from the junction to the closest edge of the circle,
  // colinear with the circle center. The circular segment joining the two paths represents the
  // path of centripetal acceleration. Solve for max velocity based on max acceleration about the
  // radius of the circle, defined indirectly by junction deviation. This may be also viewed as
  // path width or max_jerk in the previous grbl version. This approach does not actually deviate
  // from path, but used as a robust way to compute cornering speeds, as it takes into account the
  // nonlinearities of both the junction angle and junction velocity.
  double vmax_junction = MINIMUM_PLANNER_SPEED; // Set default max junction speed

  // Skip first block or when previous_nominal_speed is used as a flag for homing and offset cycles.
  if ((block_buffer_head != block_buffer_tail) && (previous_nominal_speed > 0.0)) {
    // Compute cosine of angle between previous and current path. (prev_unit_vec is negative)
    // NOTE: Max junction velocity is computed without sin() or acos() by trig half angle identity.
    double cos_theta = - previous_unit_vec[X_AXIS] * unit_vec[X_AXIS]
      - previous_unit_vec[Y_AXIS] * unit_vec[Y_AXIS]
      - previous_unit_vec[Z_AXIS] * unit_vec[Z_AXIS] ;

    // Skip and use default max junction speed for 0 degree acute junction.
    if (cos_theta < 0.95) {
      vmax_junction = min(previous_nominal_speed,block->nominal_speed);
      // Skip and avoid divide by zero for straight junctions at 180 degrees. Limit to min() of nominal speeds.
      if (cos_theta > -0.95) {
        // Compute maximum junction velocity based on maximum acceleration and junction deviation
        double sin_theta_d2 = sqrt(0.5*(1.0-cos_theta)); // Trig half angle identity. Always positive.
        vmax_junction = min(vmax_junction,
        sqrt(block->acceleration * junction_deviation * sin_theta_d2/(1.0-sin_theta_d2)) );
      }
    }
  }
#endif
  // Start with a safe speed
	//ȷ����ʼ�ٶ�
  vmax_junction = max_xy_jerk/2; 
  vmax_junction_factor = 1.0; 
  if(fabs(current_speed[Z_AXIS]) > max_z_jerk/2) 
    vmax_junction = min(vmax_junction, max_z_jerk/2);
  if(fabs(current_speed[E_AXIS]) > max_e_jerk/2) 
    vmax_junction = min(vmax_junction, max_e_jerk/2);
  vmax_junction = min(vmax_junction, block->nominal_speed);
  safe_speed = vmax_junction;

  if ((moves_queued > 1) && (previous_nominal_speed > 0.0001f)) {
    float jerk = sqrt(pow((current_speed[X_AXIS]-previous_speed[X_AXIS]), 2)+pow((current_speed[Y_AXIS]-previous_speed[Y_AXIS]), 2));
    //    if((fabs(previous_speed[X_AXIS]) > 0.0001) || (fabs(previous_speed[Y_AXIS]) > 0.0001)) {
    vmax_junction = block->nominal_speed;
    //    }
    if (jerk > max_xy_jerk) {
      vmax_junction_factor = (max_xy_jerk/jerk);
    } 
    if(fabs(current_speed[Z_AXIS] - previous_speed[Z_AXIS]) > max_z_jerk) {
      vmax_junction_factor= min(vmax_junction_factor, (max_z_jerk/fabs(current_speed[Z_AXIS] - previous_speed[Z_AXIS])));
    } 
    if(fabs(current_speed[E_AXIS] - previous_speed[E_AXIS]) > max_e_jerk) {
      vmax_junction_factor = min(vmax_junction_factor, (max_e_jerk/fabs(current_speed[E_AXIS] - previous_speed[E_AXIS])));
    } 
    vmax_junction = min(previous_nominal_speed, vmax_junction * vmax_junction_factor); // Limit speed to max previous speed
  }
  block->max_entry_speed = vmax_junction;

  // Initialize block entry speed. Compute based on deceleration to user-defined MINIMUM_PLANNER_SPEED.
  v_allowable = max_allowable_speed(-block->acceleration,MINIMUM_PLANNER_SPEED,block->millimeters);
  block->entry_speed = min(vmax_junction, v_allowable);

  // Initialize planner efficiency flags
  // Set flag if block will always reach maximum junction speed regardless of entry/exit speeds.
  // If a block can de/ac-celerate from nominal speed to zero within the length of the block, then
  // the current block and next block junction speeds are guaranteed to always be at their maximum
  // junction speeds in deceleration and acceleration, respectively. This is due to how the current
  // block nominal speed limits both the current and next maximum junction speeds. Hence, in both
  // the reverse and forward planners, the corresponding block junction speed will always be at the
  // the maximum junction speed and may always be ignored for any speed reduction checks.
  if (block->nominal_speed <= v_allowable) { 
    block->nominal_length_flag = true; 
  }
  else { 
    block->nominal_length_flag = false; 
  }
  block->recalculate_flag = true; // Always calculate trapezoid for new block

  // Update previous path unit_vector and nominal speed
  memcpy(previous_speed, current_speed, sizeof(previous_speed)); // previous_speed[] = current_speed[]
  previous_nominal_speed = block->nominal_speed;


#ifdef ADVANCE
  // Calculate advance rate
  if((block->steps_e == 0) || (block->steps_x == 0 && block->steps_y == 0 && block->steps_z == 0)) {
    block->advance_rate = 0;
    block->advance = 0;
  }
  else {
    long acc_dist = estimate_acceleration_distance(0, block->nominal_rate, block->acceleration_st);
    float advance = (STEPS_PER_CUBIC_MM_E * EXTRUDER_ADVANCE_K) * 
      (current_speed[E_AXIS] * current_speed[E_AXIS] * EXTRUTION_AREA * EXTRUTION_AREA)*256;
    block->advance = advance;
    if(acc_dist == 0) {
      block->advance_rate = 0;
    } 
    else {
      block->advance_rate = advance / (float)acc_dist;
    }
  }

#endif // ADVANCE

  calculate_trapezoid_for_block(block, block->entry_speed/block->nominal_speed,
  safe_speed/block->nominal_speed);
  //	SERIAL_ECHO_START
  //    printf("After:\r\n") ;
//	  printf("step_event_count :%d\n",block->step_event_count);
//	  printf("accelerate_until :%d\n",block->accelerate_until);
	  
  // Move buffer head
  block_buffer_head = next_buffer_head;
//	  
  // Update position
  memcpy(position, target, sizeof(target)); // position[] = target[]

  
  
  planner_recalculate();

 // st_wake_up();
}

void plan_set_position(const float x, const float y, const float z, const float e)
{
	#ifdef COREXY
	position[X_AXIS]=round(plan_convert_corexy_to_x_axis_steps(x,y)*axis_steps_per_unit[X_AXIS]);
	position[Y_AXIS]=round(plan_convert_corexy_to_y_axis_steps(x,y)*axis_steps_per_unit[Y_AXIS]);
	#else
  position[X_AXIS] = round(x*axis_steps_per_unit[X_AXIS]);
  position[Y_AXIS] = round(y*axis_steps_per_unit[Y_AXIS]);
	#endif
  position[Z_AXIS] = round(z*axis_steps_per_unit[Z_AXIS]);     
  position[E_AXIS] = round(e*axis_steps_per_unit[E_AXIS]);  
  st_set_position(position[X_AXIS], position[Y_AXIS], position[Z_AXIS], position[E_AXIS]);
  previous_nominal_speed = 0.0; // Resets planner junction speeds. Assumes start from rest.
  previous_speed[0] = 0.0;
  previous_speed[1] = 0.0;
  previous_speed[2] = 0.0;
  previous_speed[3] = 0.0;
}

void plan_set_e_position(const float e)
{
  position[E_AXIS] = round(e*axis_steps_per_unit[E_AXIS]);  
  st_set_e_position(position[E_AXIS]);
}

uint8_t movesplanned()
{
  return (block_buffer_head-block_buffer_tail + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1);
}


// Calculate the steps/s^2 acceleration rates, based on the mm/s^s
void reset_acceleration_rates(void)
{	
	int8_t i;
	for(i=0; i < NUM_AXIS; i++)
  {
		axis_steps_per_sqr_second[i] = max_acceleration_units_per_sq_second[i] * axis_steps_per_unit[i];
  }
}

//������ǰ��block
void plan_discard_current_block(void)  
{  
  if (block_buffer_head != block_buffer_tail) 
	{
    block_buffer_tail = (block_buffer_tail + 1) & (BLOCK_BUFFER_SIZE - 1);  
  }
}

//��ȡ��ǰ��block
block_t *plan_get_current_block(void) 
{
  if (block_buffer_head == block_buffer_tail) 
	{ 
    return(NULL); 
  }
  else
  {
		block_t *block = &block_buffer[block_buffer_tail];
		block->busy = true;
		return(block);
  }
}

//�ȴ����е�block����ȡ
bool blocks_queued(void) 
{
  if (block_buffer_head == block_buffer_tail) 
	{ 
    return false; 
  }
  else
    return true;
}












