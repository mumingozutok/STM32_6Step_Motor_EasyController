/*
 * app_pid.c
 *
 *  Created on: May 3, 2025
 *      Author: mumin
 */

#include "app_pid.h"

#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

float LowPassFilter_operator(float x, struct LowPassFilter* filter){
	float dt=5E-3f;
	float alpha = filter->Tf/(filter->Tf + dt);
	float y = alpha*filter->y_prev + (1.0f - alpha)*x;
	filter->y_prev = y;
	return y;
}

float PID_operator(float error, struct PIDController* pid){
	float Ts = 2.5E-3f;
	float proportional = pid->P * error;
	float integral = pid->integral_prev + pid->I*Ts*0.5f*(error + pid->error_prev);
	integral = _constrain(integral, -pid->limit, pid->limit);

	float derivative = pid->D*(error - pid->error_prev)/Ts;
	float output = proportional + integral + derivative;
	output = _constrain(output, -pid->limit, pid->limit);

	if(pid->output_ramp > 0){
		float output_rate = (output - pid->output_prev)/Ts;
		if (output_rate > pid->output_ramp)
			output = pid->output_prev + pid->output_ramp*Ts;
		else if (output_rate < -pid->output_ramp)
			output = pid->output_prev - pid->output_ramp*Ts;
	}

	pid->integral_prev = integral;
	pid->output_prev = output;
	pid->error_prev = error;

	return output;
}

