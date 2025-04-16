/*
 * app.c
 *
 *  Created on: Apr 7, 2025
 *      Author: mumin
 */
#include "main.h"
#include "app.h"


extern UART_HandleTypeDef hlpuart1;
extern TIM_HandleTypeDef htim1;
uint8_t rxData_UART;

uint8_t first_value = 0;

uint16_t pwm_duty = 25;
uint16_t encoder_position = 0;


extern TIM_HandleTypeDef htim2;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == LPUART1)
	{
		/*
		HAL_UART_Transmit(&hlpuart1, &rxData_UART, 1, HAL_MAX_DELAY);
		HAL_UART_Receive_IT(&hlpuart1, &rxData_UART, 1);

	    if((first_value == 1) & (rxData_UART >= 0) & (rxData_UART <= 100))
	    {
	    	if(pwm_duty == 0)
	    	{
	    		//pwm_duty = rxData_UART;

	    		//hall_state = Read_Hall_Sensors();
	    		//motor_commutation(hall_state);
	    	}
	    	else
	    	{
	    		//pwm_duty = rxData_UART;
	    	}


	    }
		first_value = 1;
		*/
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
		set_mosfets(pwm_duty, 0, 0, 1,  1,  0,  0);

		break;

	case 2:
				     //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 0, 1,  0,  1,  0);
		break;

	case 3:
			         //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 1, 0, 0,  0,  1,  0);

		break;

	case 4:
			  	     //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 1, 0, 0,  0,  0,  1);

		break;

	case 5:
				     //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 1, 0,  0,  0,  1);
		break;

	case 6:
				     //du,    uh,vh,wh, ul, vl, wl
		set_mosfets(pwm_duty, 0, 1, 0,  1,  0,  0);

		break;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint8_t commutation_step = 2;
    if (htim->Instance == TIM2)
    {
        // Her 34 adımda bir burası çalışacak!
    	motor_commutation(commutation_step++);
    	if(commutation_step > 6){
    		commutation_step = 1;
    	}
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_10) // PB10'a bağlı Z sinyali
    {
        // Z darbesi geldiğinde yapılacak iş:
        __HAL_TIM_SET_COUNTER(&htim2, 0); // Sayaç sıfırlama
        // veya başka bir şey: encoder_position = __HAL_TIM_GET_COUNTER(&htim2);
    }
}

void encoder_init(void)
{
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL); // Encoder mode başlat

}

void encoder_read(void)
{
    encoder_position = __HAL_TIM_GET_COUNTER(&htim2);
}

void init_app()
{

	HAL_UART_Receive_IT(&hlpuart1, &rxData_UART, 1);
	encoder_init();

    //HAL_TIMEx_HallSensor_Start_IT(&htim2);

	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL); // Interrupt ile encoder başlat

	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);


	motor_commutation(1);
	encoder_read();

	motor_commutation(2);
	encoder_read();

	/*motor_commutation(3);
	encoder_read();
	motor_commutation(4);
	encoder_read();
	motor_commutation(5);
	encoder_read();
	motor_commutation(6);*/
	encoder_read();
	//hall_state = Read_Hall_Sensors();
	//motor_commutation(hall_state);

	/*uint16_t init_sequence = 100;
	while(init_sequence > 0){
		hall_state = Read_Hall_Sensors();
		motor_commutation(hall_state);
		HAL_Delay(10);
		init_sequence -= 10;
	}*/

}
