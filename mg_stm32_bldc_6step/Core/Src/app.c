/*
 * app.c
 *
 *  Created on: Apr 7, 2025
 *      Author: mumin
 */
#include "main.h"
#include "cmsis_os.h"
#include "app.h"
#include "app_sensorless.h"
#include "uart_debug.h"

#define HALLSENSOR
//#define SENSORLESS


#define CHU 0
#define CHV 1
#define CHW 2
#define ENABLE 1
#define DISABLE 0
#define RISING 0
#define FALLING  1


volatile uint8_t hall_state, hall_state_old;
volatile uint8_t hall_state_buf[100];
volatile uint16_t hall_state_buf_cntr = 0;

volatile uint8_t state_buf[100];
volatile uint16_t state_buf_cntr = 0;

extern TIM_HandleTypeDef htim1;
extern uint32_t rpm_counter_x10;

extern TIM_HandleTypeDef htim3;

uint8_t first_value = 0;


uint16_t bufferCounter = 0;
uint16_t pwm_duty = 16;
uint32_t rpm_value = 0;

uint8_t motor_running = 0;
uint8_t motor_commutation_step = 1;


volatile uint8_t hall_state, hall_state_old;

extern TIM_HandleTypeDef htim2;

int hall_position = 0;

uint8_t com_State = 1;


uint8_t readH1(){
	if(HAL_GPIO_ReadPin(HALL_H1_GPIO_Port, HALL_H1_Pin) == GPIO_PIN_SET){
		return 1;
	}
	else return 0;
}

uint8_t readH2(){
	if(HAL_GPIO_ReadPin(HALL_H2_GPIO_Port, HALL_H2_Pin) == GPIO_PIN_SET){
		return 1;
	}
	else return 0;
}

uint8_t readH3(){
	if(HAL_GPIO_ReadPin(HALL_H3_GPIO_Port, HALL_H3_Pin) == GPIO_PIN_SET){
		return 1;
	}
	else return 0;
}

uint8_t Read_Hall_Sensors(void) {
	if ((readH1() == 1 )&( readH2() == 0 )&( readH3() == 0)) 		return 1;
	else if((readH1() == 1 )&( readH2() == 1 )&( readH3() == 0)) 	return 2;
	else if((readH1() == 0 )&( readH2() == 1 )&( readH3() == 0)) 	return 3;
	else if((readH1() == 0 )&( readH2() == 1 )&( readH3() == 1)) 	return 4;
	else if((readH1() == 0 )&( readH2() == 0 )&( readH3() == 1)) 	return 5;
	else if((readH1() == 1 )&( readH2() == 0 )&( readH3() == 1)) 	return 6;
	else{
		//fault
		return 0;
	}
}

void set_mosfets(uint16_t duty, uint8_t uh, uint8_t vh, uint8_t wh, uint8_t ul, uint8_t vl, uint8_t wl)
{
	TIM1->CCR1 = duty*uh;
	TIM1->CCR2 = duty*vh;
	TIM1->CCR3 = duty*wh;

	HAL_GPIO_WritePin(GPIO_UL_GPIO_Port, GPIO_UL_Pin, 1*ul);
	HAL_GPIO_WritePin(GPIO_VL_GPIO_Port, GPIO_VL_Pin, 1*vl);
	HAL_GPIO_WritePin(GPIO_WL_GPIO_Port, GPIO_WL_Pin, 1*wl);
}

void motor_commutation(uint8_t step_val){
	switch(step_val){
	case 1:
				     //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 0, 1,  1,  0,  0); //boşta -> v

		break;

	case 2:
				     //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 0, 1,  0,  1,  0);//boşta -> u

		break;

	case 3:
			         //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 1, 0, 0,  0,  1,  0);//boşta -> w

		break;

	case 4:
			  	     //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 1, 0, 0,  0,  0,  1);//boşta -> v

		break;

	case 5:
				     //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 1, 0,  0,  0,  1);//boşta -> u

		break;

	case 6:
				     //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 1, 0,  1,  0,  0);//boşta -> w

		break;
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
    	//hall_state = Read_Hall_Sensors();
    	//motor_commutation(hall_state);
    }
}

uint8_t get_motor_commutation_step_minus_1(){
	if(motor_commutation_step == 1) return 6;
	else {
		return (motor_commutation_step-1) ;
	}
}

void HAL_TIM_PeriodElapsedCallback_App(TIM_HandleTypeDef *htim)
{
	static uint32_t counter_10us = 1;
	static uint32_t counter_10us_1 = 1;
	static uint32_t pid_loop_period = 100000;


	if (htim->Instance == TIM1){
		if(motor_running == 0) return;
		start_ADC_Sensorless(get_motor_commutation_step_minus_1());
		//start_ADC_Sensorless(hall_state);
	}
    if ((htim->Instance == TIM3)) { //10 us timer
    	counter_10us++;
    	if(motor_running == 1) counter_10us_1++;
    	/*if((counter_10us % 100000  == 0) & (motor_running == 1)){
    		counter_10us = 0;
    		if(pwm_duty < 50){
    			//pwm_duty = pwm_duty + 5;
    		}
    	}*/

    	if((counter_10us % 5000) == 0){
    		rpm_value = (rpm_counter_x10*20); //10'a böl, 60'la çarp
    		rpm_counter_x10 = 0;
    		counter_10us = 1;
    		PID_Loop();

    	}
    	/*
    	if((counter_10us_1 % pid_loop_period) == 0){
    		//rpm_value = (rpm_counter_x10 * 0.06); //10'a böl, 60'la çarp
    		//rpm_counter_x10 = 0;
    		pid_loop_period = 100000;
    		counter_10us_1 = 1;
    		PID_Loop();
    	}
    	*/
    }
}

void motor_step(){
	motor_commutation(motor_commutation_step);
	motor_commutation_step++;
	if(motor_commutation_step > 6){
		motor_commutation_step = 1;
	}
	//HAL_Delay(1);
}

void forcedInitialization(){
	//for(uint8_t i = 0;i<10;i++){ //1 tam tur
		motor_step();
		HAL_Delay(10);

		motor_step();
		HAL_Delay(10);

		motor_step();
		HAL_Delay(10);

		motor_step();
		HAL_Delay(10);

		motor_step();
		HAL_Delay(10);

		motor_step();
		//HAL_Delay(10);
	//}

	motor_running = 1;
}

void init_app()
{


	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;

#ifdef HALLSENSOR
		HAL_TIMEx_HallSensor_Start_IT(&htim2);
		hall_state = Read_Hall_Sensors();
		//motor_commutation(hall_state);
#endif

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);

	init_uart_debug();
	init_app_sensorless();

	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);

	//motor_commutation(com_State);
	HAL_TIM_Base_Start_IT(&htim3);


	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);



	forcedInitialization();

	app_sensorless_loop();

}
