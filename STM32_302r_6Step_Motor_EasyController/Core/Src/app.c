/*
 * app.c
 *
 *  Created on: Apr 7, 2025
 *      Author: mumin
 */
#include "main.h"
#include "app.h"

#define CHU 0
#define CHV 1
#define CHW 2
#define ENABLE 1
#define DISABLE 0

volatile uint8_t hall_state, hall_state_old;
volatile uint8_t hall_state_buf[100];
volatile uint16_t hall_state_buf_cntr = 0;

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim1;
uint8_t rxData_UART;

uint8_t first_value = 0;

uint16_t pwm_duty = 25;


volatile uint8_t hall_state, hall_state_old;

extern TIM_HandleTypeDef htim2;

int hall_position = 0;

uint8_t osdelay = 10;


/*

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
}*/

uint32_t pulse_counter = 0;
uint32_t pulse_buffer[100];
uint32_t pulse_buffer_counter = 0;

#if 0
uint8_t Read_Hall_Sensors(void) {
	/*uint8_t hall_state = 0;

	if((readH1() == 1) & (readH2() == 0) & (readH3() == 0)) 		return 1;
	else if(readH1() == 1 & readH2() == 1 & readH3() == 0) 	return 2;
	else if(readH1() == 0 & readH2() == 1 & readH3() == 0) 	return 3;
	else if(readH1() == 0 & readH2() == 1 & readH3() == 1) 	return 4;
	else if(readH1() == 0 & readH2() == 0 & readH3() == 1) 	return 5;
	else if(readH1() == 1 & readH2() == 0 & readH3() == 1) 	return 6;
	else{
		//fault
		return 0;
	}*/

	if(readH3() == 1){
		pulse_buffer[pulse_buffer_counter++] = pulse_counter;
		pulse_counter = 0;
	}

	else if(readH1() == 1 && readH2() == 0){
		pulse_counter++;
	}
}
#endif

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		HAL_UART_Transmit(&huart2, &rxData_UART, 1, HAL_MAX_DELAY);
		HAL_UART_Receive_IT(&huart2, &rxData_UART, 1);

	    if((first_value == 1) & (rxData_UART >= 0) & (rxData_UART <= 100))
	    {
	    	if(pwm_duty == 0)
	    	{
	    		pwm_duty = rxData_UART;

	    		//hall_state = Read_Hall_Sensors();
	    		motor_commutation(hall_state);
	    	}
	    	else
	    	{
	    		pwm_duty = rxData_UART;
	    	}


	    }
		first_value = 1;
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
#if 0
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
    	motor_commutation(hall_state);
    	//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_4);
    }
}
#endif


// HAL'da callback fonksiyonunu override edeceğiz:
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

void init_app()
{

	HAL_UART_Receive_IT(&huart2, &rxData_UART, 1);
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


///Encoder Code///
volatile uint32_t encoder_position = 0;

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
