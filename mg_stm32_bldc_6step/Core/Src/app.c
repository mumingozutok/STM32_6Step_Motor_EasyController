/*
 * app.c
 *
 *  Created on: Apr 7, 2025
 *      Author: mumin
 */
#include "main.h"
#include "cmsis_os.h"
#include "app.h"

#define CHU 0
#define CHV 1
#define CHW 2
#define ENABLE 1
#define DISABLE 0

volatile uint8_t hall_state, hall_state_old;
volatile uint8_t hall_state_buf[100];
volatile uint16_t hall_state_buf_cntr = 0;

extern UART_HandleTypeDef hlpuart1;
extern TIM_HandleTypeDef htim1;
uint8_t rxData_UART;

uint16_t pwm_duty = 25;

osThreadId_t appTaskHandle;
const osThreadAttr_t appTask_attributes = {
		.name = "appTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 128 * 4
};

volatile uint8_t hall_state, hall_state_old;

extern TIM_HandleTypeDef htim2;

int hall_position = 0;

uint8_t osdelay = 10;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == LPUART1)
	{
		HAL_UART_Transmit(&hlpuart1, &rxData_UART, 1, HAL_MAX_DELAY);
		HAL_UART_Receive_IT(&hlpuart1, &rxData_UART, 1);

		//osdelay = rxData_UART;
	}
}

uint8_t Read_Hall_Sensors(void) {
	uint8_t hall_state = 0;

	if (HAL_GPIO_ReadPin(HALL_H1_GPIO_Port, HALL_H1_Pin) == GPIO_PIN_SET) hall_state |= 0x01;
	if (HAL_GPIO_ReadPin(HALL_H2_GPIO_Port, HALL_H2_Pin) == GPIO_PIN_SET) hall_state |= 0x02;
	if (HAL_GPIO_ReadPin(HALL_H3_GPIO_Port, HALL_H3_Pin) == GPIO_PIN_SET) hall_state |= 0x04;

	return hall_state;
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

void motor_step(uint8_t step_val){
	switch(step_val){
	case 6:
				     //du,    	uh,vh,wh, ul, vl, wl
		//set_mosfets(pwm_duty, 1, 0, 0,  0,  1,  0);

		//HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		//HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);

		TIM1->CCR1 = pwm_duty; //enable TIM_CHANNEL1 pwm
		TIM1->CCR2 = 0; //enable TIM_CHANNEL1 pwm
		TIM1->CCR3 = 0; //enable TIM_CHANNEL1 pwm

		//HAL_GPIO_WritePin(PWM_VL_GPIO_Port, PWM_VL_Pin, 1);
		//HAL_GPIO_WritePin(PWM_WL_GPIO_Port, PWM_WL_Pin, 0);

		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

		break;

	case 4:
				     //du,      uh,vh,wh, ul, vl, wl
		//set_mosfets(pwm_duty, 1, 0, 0,  0,  0,  1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);


		TIM1->CCR1 = pwm_duty; //enable TIM_CHANNEL1 pwm
		TIM1->CCR2 = 0; //enable TIM_CHANNEL1 pwm
		TIM1->CCR3 = 0; //enable TIM_CHANNEL1 pwm

		//HAL_GPIO_WritePin(PWM_VL_GPIO_Port, PWM_VL_Pin, 0);
		//HAL_GPIO_WritePin(PWM_WL_GPIO_Port, PWM_WL_Pin, 1);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
		break;

	case 5:
			         //du,      uh,vh,wh, ul, vl, wl
		//set_mosfets(pwm_duty, 0, 1, 0,  0,  0,  1);

		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);



		TIM1->CCR1 = 0; //enable TIM_CHANNEL1 pwm
		TIM1->CCR2 = pwm_duty; //enable TIM_CHANNEL1 pwm
		TIM1->CCR3 = 0; //enable TIM_CHANNEL1 pwm

		//HAL_GPIO_WritePin(PWM_VL_GPIO_Port, PWM_UL_Pin, 0);
		//HAL_GPIO_WritePin(PWM_WL_GPIO_Port, PWM_WL_Pin, 1);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

		break;

	case 1:
			  	     //du,     uh,vh,wh, ul, vl, wl
		//set_mosfets(pwm_duty, 0, 1, 0,  1,  0,  0);

		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);

		TIM1->CCR1 = 0; //enable TIM_CHANNEL1 pwm
		TIM1->CCR2 = pwm_duty; //enable TIM_CHANNEL1 pwm
		TIM1->CCR3 = 0; //enable TIM_CHANNEL1 pwm

		//HAL_GPIO_WritePin(PWM_VL_GPIO_Port, PWM_UL_Pin, 1);
		//HAL_GPIO_WritePin(PWM_WL_GPIO_Port, PWM_WL_Pin, 0);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
		break;

	case 3:
				     //du,      uh,vh,wh, ul, vl, wl
		//set_mosfets(pwm_duty, 0, 0, 1,  1,  0,  0);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

		TIM1->CCR1 = 0; //enable TIM_CHANNEL1 pwm
		TIM1->CCR2 = 0; //enable TIM_CHANNEL1 pwm
		TIM1->CCR3 = pwm_duty; //enable TIM_CHANNEL1 pwm

		//HAL_GPIO_WritePin(PWM_VL_GPIO_Port, PWM_UL_Pin, 1);
		//HAL_GPIO_WritePin(PWM_WL_GPIO_Port, PWM_VL_Pin, 0);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
		break;

	case 2:
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

				     //du,      uh,vh,wh, ul, vl, wl
		//set_mosfets(pwm_duty, 0, 0, 1,  0,  1,  0);
		TIM1->CCR1 = 0; //enable TIM_CHANNEL1 pwm
		TIM1->CCR2 = 0; //enable TIM_CHANNEL1 pwm
		TIM1->CCR3 = pwm_duty; //enable TIM_CHANNEL1 pwm

		//HAL_GPIO_WritePin(PWM_VL_GPIO_Port, PWM_UL_Pin, 0);
		//HAL_GPIO_WritePin(PWM_WL_GPIO_Port, PWM_VL_Pin, 1);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
		break;
	}
}


void motor_commutation(uint8_t step_val){
	switch(step_val){
	case 1:
				     //du,    	uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 1, 0, 0,  0,  1,  0);

		break;

	case 2:
				     //du,      uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 1, 0, 0,  0,  0,  1);
		break;

	case 3:
			         //du,      uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 1, 0,  0,  0,  1);

		break;

	case 4:
			  	     //du,     uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 1, 0,  1,  0,  0);

		break;

	case 5:
				     //du,      uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 0, 1,  1,  0,  0);
		break;

	case 6:
				     //du,      uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 0, 1,  0,  1,  0);

		break;
	}
}
// EXTI (Harici Kesme) Callback Fonksiyonu
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	hall_state = Read_Hall_Sensors();

	if(hall_state != hall_state_old){
		hall_state_old = hall_state;
		hall_state_buf[hall_state_buf_cntr++] = hall_state;
		if(hall_state_buf_cntr == 100) hall_state_buf_cntr = 0;
	}

	motor_step(hall_state);
}


void StartAppTask(void *argument)
{
	uint8_t test_step[6] = {1,2,3,4,5,6};

	pwm_duty = 25;

	static uint8_t test_step_i = 0;
	/* Infinite loop */
	for(;;)
	{
		//motor_step(test_step[test_step_i++]);
		if(test_step_i == 6) test_step_i = 0;

		motor_commutation(test_step[test_step_i++]);

		/*if(pwm_duty < 60) {
			pwm_duty++;
			motor_step(test_step[test_step_i++]);
		}*/

		osDelay(osdelay); //wait for 1 sec
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        // Burada capture olduğunda yapılacak işlemleri yazarsınız
        // Gerekirse GPIO pinlerini doğrudan okuyarak hall kodu çıkarabilirsiniz
    	hall_state = Read_Hall_Sensors();

    	if(hall_state != hall_state_old){
    		hall_state_old = hall_state;
    		hall_state_buf[hall_state_buf_cntr++] = hall_state;
    		if(hall_state_buf_cntr == 100) hall_state_buf_cntr = 0;
    	}
    	motor_step(hall_state);
    }
}

void init_app()
{

	HAL_UART_Receive_IT(&hlpuart1, &rxData_UART, 1);


    //HAL_TIMEx_HallSensor_Start_IT(&htim2);

	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	osThreadNew(StartAppTask, NULL, &appTask_attributes);
}
