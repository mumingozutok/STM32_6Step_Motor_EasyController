/*
 * app_six_step_control.c
 *
 *  Created on: May 2, 2025
 *      Author: mumin
 */

#include "app_six_step_control.h"
#include "app_encoder.h"
#include "app_six_step_L6230.h"

#define POLE_PAIR 7

void initialize_motor_movement(){
	//send motor to a known position and reset encoder
	motor_set_step(0);
	motor_commutation_step();
	encoder_reset();
}

void motor_six_step_encoder_control_loop(){
	static uint8_t control_state = 0;
	float encoder_angle;
	float step_degree = 8.57; //360 / POLE_PAIR / 6;
	uint8_t commutation_pos;

	switch(control_state){
	case 0:
		initialize_motor_movement();
		control_state = 1;
		break;
	case 1:
		encoder_angle= encoder_get_angle_deg();

		commutation_pos = (((uint16_t)(encoder_angle / step_degree) % 6) + 1) % 6;

		motor_set_step(commutation_pos);
		motor_commutation_step();

		/*if(encoder_commutation_pos != encoder_commutation_table[encoder_count])
		{
			encoder_commutation_pos = encoder_commutation_table[encoder_count];

		}*/

		break;
	}
}

