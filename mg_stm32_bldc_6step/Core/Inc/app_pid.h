/*
 * app_pid.h
 *
 *  Created on: May 3, 2025
 *      Author: mumin
 */

#ifndef INC_APP_PID_H_
#define INC_APP_PID_H_

struct LowPassFilter{
	float Tf;
	float y_prev;
};

float LowPassFilter_operator(float x, struct LowPassFilter* filter);

struct PIDController{
	float P;
	float I;
	float D;
	float output_ramp;
	float limit;
    float error_prev;
    float output_prev;
    float integral_prev;
};
float PID_operator(float error, struct PIDController* pid);


#endif /* INC_APP_PID_H_ */
