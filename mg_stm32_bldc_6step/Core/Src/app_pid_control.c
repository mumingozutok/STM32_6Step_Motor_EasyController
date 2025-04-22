/*
 * app_pid_control.c
 *
 *  Created on: Apr 21, 2025
 *      Author: zafer
 */


#include "main.h"
#include "app_pid_control.h"

#define PID_MAX_PWM 95
#define PID_MIN_PWM 15

extern uint16_t pwm_duty;
extern uint32_t rpm_value;


//output = Kp * error + Ki * integral + Kd * derivative;

static float target_rpm = 120.0f;
static float current_rpm = 0.0f;

static float Kp = 0.02f;
static float Ki = 0.02f;
static float Kd = 0.0002f;

static float error = 0.0f;
static float previous_error = 0.0f;
static float integral = 0.0f;
static float derivative = 0.0f;

static float pid_output = 0.0f;
static float dt = 1.0f; // PID döngüsü her 10ms'de bir çağrılıyor
static uint32_t pid_loop_cntr = 0;

float pid_control(float target, float measured)
{
    error = target - measured;
    integral += error * dt;
    derivative = (error - previous_error) / dt;

    float output = Kp * error + Ki * integral + Kd * derivative;
    previous_error = error;

    return output;
}

void PID_Loop()
{
	uint32_t debug_buf[6];

	uint16_t temp_pwm_duty = pwm_duty;
    current_rpm = (float)rpm_value;

    pid_output = pid_control(target_rpm, current_rpm);

    // PID çıkışını PWM duty'e çevir
    temp_pwm_duty += pid_output;

    if (temp_pwm_duty > PID_MAX_PWM) temp_pwm_duty = PID_MAX_PWM;
    if (temp_pwm_duty < PID_MIN_PWM) temp_pwm_duty = PID_MIN_PWM;

    pwm_duty = temp_pwm_duty;
    /*
	debug_buf[0] = rpm_value;
	debug_buf[1] = error;
	debug_buf[2] = pid_output;
	debug_buf[3] =  target_rpm;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
	send_UART_CSV_Data(debug_buf,4);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
	*/
	//
    pid_loop_cntr++;
}
