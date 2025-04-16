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

typedef struct {
	uint16_t encoder_position;
	uint8_t comm_step;
	uint8_t loop_state;
} sEncCommDebug;

sEncCommDebug encCommDebugData;

uint16_t encoder_debug_buf_cntr = 0;

#define LOOP_STATE_FIRST_IGNITION 0
#define LOOP_STATE_1 1
#define LOOP_STATE_2 2
#define LOOP_STATE_3 3
#define LOOP_STATE_RUN 4

//#define TEST_ENCODER

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
	step_val = 7-step_val; //terse dönmesi için

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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_10) // PB10'a bağlı Z sinyali
	{
		// Z darbesi geldiğinde yapılacak iş:
		//__HAL_TIM_SET_COUNTER(&htim2, 0); // Sayaç sıfırlama

		__NOP();
		// veya başka bir şey: encoder_position = __HAL_TIM_GET_COUNTER(&htim2);
	}
}

void encoder_init(void)
{
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL); // Encoder mode başlat
}

static uint8_t commutation_step = 1;
void commutate_motor()
{
#ifdef TEST_ENCODER
	commutation_step++;
	if(commutation_step > 6) commutation_step = 1;
#else
	motor_commutation(commutation_step++);
	if(commutation_step > 6) commutation_step = 1;
#endif
}

uint16_t get_test_encoder_val(uint8_t reset){
	static uint16_t test_encoder_position = 0;

	if(reset == 1){
		test_encoder_position = 0;
	}

	else{
		test_encoder_position++;
	}

	return test_encoder_position;
}

uint16_t get_encoder_value(){
	uint16_t ret;
#ifdef TEST_ENCODER
		ret = get_test_encoder_val(0);
#else
		ret = __HAL_TIM_GET_COUNTER(&htim2);
#endif
		return ret;
}

void reset_encoder_value(){
#ifdef TEST_ENCODER
		get_test_encoder_val(1);
#else
		__HAL_TIM_SET_COUNTER(&htim2, 0);
#endif
}

void send_debug_struct_over_uart(sEncCommDebug *data)
{
    char uart_buf[64];
    int len = snprintf(uart_buf, sizeof(uart_buf), "%u,%u,%u\r\n",
                       data->encoder_position,
                       data->comm_step,
                       data->loop_state);

    HAL_UART_Transmit(&hlpuart1, (uint8_t *)uart_buf, len, HAL_MAX_DELAY);
}

void init_app()
{
	uint16_t old_encoder_position = 0;
	uint32_t loop_state = LOOP_STATE_FIRST_IGNITION;
	uint8_t encoder_limit_buf[7] = {32,32,32,32,32,32,35};
	uint8_t encoder_limit;
	uint8_t encoder_limit_buf_cntr = 0;

	HAL_UART_Receive_IT(&hlpuart1, &rxData_UART, 1);
	encoder_init();

	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL); // Interrupt ile encoder başlat

	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	while(1){

		encoder_position = get_encoder_value();

		switch(loop_state){

		case LOOP_STATE_FIRST_IGNITION:
			commutate_motor();
			HAL_Delay(10);
			reset_encoder_value();
			commutate_motor();
			HAL_Delay(10);

			reset_encoder_value();
			encoder_position = get_encoder_value();
			commutate_motor();

			loop_state = LOOP_STATE_1;
			break;

		case LOOP_STATE_1:
			if(encoder_position > 22){
				reset_encoder_value();
				commutate_motor();
				loop_state = LOOP_STATE_2;
			}
			else{
				loop_state = LOOP_STATE_1;
			}
			break;

		case LOOP_STATE_2:
			if(encoder_position > 26){
				reset_encoder_value();
				commutate_motor();
				loop_state = LOOP_STATE_RUN;
			}
			else{
				loop_state = LOOP_STATE_2;
			}
			break;

		case LOOP_STATE_RUN:
			encoder_limit = encoder_limit_buf[encoder_limit_buf_cntr];
			if(encoder_position > encoder_limit){
				encoder_limit_buf_cntr++;
				if(encoder_limit_buf_cntr == sizeof(encoder_limit_buf)){
					encoder_limit_buf_cntr = 0;
				}

				reset_encoder_value();
				commutate_motor();

				loop_state = LOOP_STATE_RUN; //motorun durduğunu anlayapı ona göre tekrar başlatmamız gerekiyor.
			}
			break;
		}


		if(old_encoder_position != encoder_position){
			old_encoder_position = encoder_position;

			encCommDebugData.encoder_position = encoder_position;
			encCommDebugData.comm_step = commutation_step;
			encCommDebugData.loop_state = loop_state;

			send_debug_struct_over_uart(&encCommDebugData);
		}
	}
}
