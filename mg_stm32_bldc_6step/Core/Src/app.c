/*
 * app.c
 *
 *  Created on: Apr 7, 2025
 *      Author: mumin
 */
#include "main.h"
#include "app.h"

extern TIM_HandleTypeDef htim3;


extern UART_HandleTypeDef hlpuart1;
extern TIM_HandleTypeDef htim1;
uint8_t rxData_UART;

uint8_t first_value = 0;
uint16_t pwm_duty = 20;
uint16_t target_pwm_duty = 20;
volatile uint8_t motor_running = 0;

uint16_t encoder_position = 0;

extern TIM_HandleTypeDef htim2;

volatile uint32_t counter_100us = 0;
volatile uint8_t encoder_timer_overflow_flag = 0;

typedef struct {
	uint16_t encoder_position;
	uint8_t comm_step;
	uint8_t loop_state;
} sEncCommDebug;

sEncCommDebug encCommDebugData;

#define ENCODER_PPR 2048
#define COMMUTATION_OFFSET 0

uint16_t encoder_commutation_table[ENCODER_PPR];
uint8_t commutation_sequence[6] = {2,3,4,5,6,1};

//#define TEST_ENCODER

void buildCommutationTable()
{
	uint16_t i;

	for(i=0;i<ENCODER_PPR;i++)
	{
		encoder_commutation_table[i] = commutation_sequence[(i*10*6/ENCODER_PPR) % 6];
	}
}

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

#ifdef TEST_ENCODER
	return; //no commutation, just return
#endif

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

volatile uint32_t speed_counter_now= 0;
volatile uint32_t speed_counter_old= 0;

float rpm_measured = 0;
float time_delta; //x*100us
float period;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	//float time_delta; //x*100us
	if (GPIO_Pin == GPIO_PIN_10) // PB10'a bağlı Z sinyali
	{
		// Z darbesi geldiğinde yapılacak iş:
		//__HAL_TIM_SET_COUNTER(&htim2, 0); // Sayaç sıfırlama

		time_delta = (counter_100us - speed_counter_old) ;
		time_delta = time_delta/1e+4;

		rpm_measured = 60 / time_delta ;

		speed_counter_old = counter_100us;


		__NOP();
		// veya başka bir şey: encoder_position = __HAL_TIM_GET_COUNTER(&htim2);
	}
}

void encoder_init(void)
{
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL); // Encoder mode başlat
}


uint16_t get_test_encoder_val(uint8_t reset){

	static uint16_t test_encoder_position = 0;

	if(reset == 1){
		test_encoder_position = 0;
	}

	else{
		test_encoder_position++;

		if(test_encoder_position > 2048) test_encoder_position = 0;
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

volatile int32_t encoder_count;
uint16_t encoder_shaft_pos; //this is the shaft position as encoder counts
uint16_t encoder_commutation_pos; //this is shaft position from the beginning of current commuatiton sequence.
volatile uint16_t encoder_lastCount;

void getEncoderCount()
{
	int16_t delta;
	uint16_t now = get_encoder_value();

	if(now<encoder_lastCount){
		//overflow --> now: 3, encoder_lastCount = 65532
		delta = (2048-encoder_lastCount) + now+1;
	}

	else{
		delta = (int16_t)(now - encoder_lastCount);
	}

	encoder_lastCount = now;

	encoder_count = (encoder_count + delta) % ENCODER_PPR;
	encoder_shaft_pos= encoder_count;
}

void forcedInitialization(){
	motor_commutation(1);
	HAL_Delay(10);
	motor_commutation(2);
	HAL_Delay(10);
	motor_commutation(3);
	HAL_Delay(10);
	motor_commutation(4);
	HAL_Delay(10);
	motor_commutation(5);
	HAL_Delay(10);
	motor_commutation(6);
	HAL_Delay(10);
	motor_running = 1;
	reset_encoder_value();
}

void init_Encoder(){
	encoder_count=0;
	encoder_lastCount=0;
	encoder_shaft_pos=0;

	buildCommutationTable();
	forcedInitialization();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {
    // 100 µs'de çalışan kod

	  counter_100us++;

	  if((counter_100us % 1000) == 0){ //100ms
		  if(target_pwm_duty <99){
			  if((pwm_duty<target_pwm_duty) & motor_running) {
				  pwm_duty++;
			  }
			  else if((pwm_duty>target_pwm_duty) & motor_running) {
				  pwm_duty--;
			  }
		  }
	  }
  }

  else   if (htim->Instance == TIM2)
  {
    // Overflow/Underflow oldu
    encoder_timer_overflow_flag = 1;
  }
}

void init_app()
{
	HAL_UART_Receive_IT(&hlpuart1, &rxData_UART, 1);
	encoder_init();

	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL); // Interrupt ile encoder başlat

	HAL_TIM_Base_Start_IT(&htim3);  // Timer'ı interrupt ile başlat

	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	init_Encoder();

	while(1){
		getEncoderCount();

		//check commutation position for trapezoid commutation if done by encoder
		if(encoder_commutation_pos != encoder_commutation_table[encoder_shaft_pos])
		{
			encoder_commutation_pos = encoder_commutation_table[encoder_shaft_pos];
			motor_commutation(encoder_commutation_pos);

			encCommDebugData.encoder_position = encoder_shaft_pos;
			encCommDebugData.comm_step = encoder_commutation_pos;
			//encCommDebugData.loop_state = 0;

			//send_debug_struct_over_uart(&encCommDebugData);
		}

	}
}
