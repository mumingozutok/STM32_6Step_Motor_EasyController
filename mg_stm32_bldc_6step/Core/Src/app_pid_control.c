/*
 * app_pid.c
 *
 *  Created on: May 3, 2025
 *      Author: mumin
 */
#include "main.h"

extern uint16_t pwm_duty;
extern uint32_t rpm_value;


//output = Kp * error + Ki * integral + Kd * derivative;

static float target_rpm = 500.0f;
static float target_ramp_rpm = 500.0f;
static float current_rpm = 0.0f;

static float Kp = 0.02f;
static float Ki = 0.01f;//0.02f;
static float Kd = 0.0f;//0.0002f;

static float error = 0.0f;
static float previous_error = 0.0f;
static float integral = 0.0f;
static float derivative = 0.0f;

static float pid_output = 0.0f;
static float dt = 0.001f; // PID döngüsü her 1ms'de bir çağrılıyor
static uint32_t pid_loop_cntr = 0;

void reset_pid_params(){
	integral = 0;
	//target_rpm = 500;
}

float pid_control(float target, float measured)
{
    error = target - measured;
    integral += error * dt;
    derivative = (error - previous_error) / dt;

    float output = Kp * error + Ki * integral + Kd * derivative;
    previous_error = error;

    return output;
}

float PID_Loop()
{
	//uint16_t temp_pwm_duty = pwm_duty;
    current_rpm = (float)rpm_value;

    if(target_rpm > target_ramp_rpm){
    	target_ramp_rpm += 1;
    }

    else{
    	target_ramp_rpm -= 1;
    }

    pid_output = pid_control(target_ramp_rpm, current_rpm);

    // PID çıkışını PWM duty'e çevir
    //temp_pwm_duty += pid_output;

    //if (temp_pwm_duty > PID_MAX_PWM) temp_pwm_duty = PID_MAX_PWM;
    //if (temp_pwm_duty < PID_MIN_PWM) temp_pwm_duty = PID_MIN_PWM;

    //pwm_duty = temp_pwm_duty;

    //pid_loop_cntr++;

    return pid_output;
}

