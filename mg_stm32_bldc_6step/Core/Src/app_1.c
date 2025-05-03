/*
 * app_1.c
 *
 *  Created on: May 2, 2025
 *      Author: mumin
 */
#include "main.h"
#include "app_six_step_L6230.h"
#include "app_encoder.h"
#include "app_pid_control.h"

#define PID_MAX_PWM 990
#define PID_MIN_PWM 300

extern TIM_HandleTypeDef htim3;

uint16_t pwm_duty = 800;
volatile uint32_t rpm_value = 0;
volatile int32_t encoder_count;
uint16_t encoder_shaft_pos; //this is the shaft position as encoder counts
uint16_t encoder_commutation_pos; //this is shaft position from the beginning of current commuatiton sequence.
volatile uint16_t encoder_lastCount;

#define ENCODER_PPR 2400

uint16_t encoder_commutation_table[ENCODER_PPR];
uint8_t commutation_sequence[6] = {2,3,4,5,0,1};

extern ADC_HandleTypeDef hadc1;

//ADC Örneklemesini TIM1-PWM tetikliyor.
volatile uint16_t adc_current_val;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	adc_current_val = HAL_ADC_GetValue(hadc);
	HAL_GPIO_TogglePin(DEBUG_PIN_GPIO_Port, DEBUG_PIN_Pin);
    // Akım hesapla
}

void buildCommutationTable()
{
	uint16_t i;

	for(i=0;i<ENCODER_PPR;i++)
	{
		encoder_commutation_table[i] = commutation_sequence[(i*7*6/ENCODER_PPR) % 6];
	}
}

void getEncoderCount()
{
	int16_t delta;
	uint16_t now = encoder_get_count();

	if(now<encoder_lastCount){
		//overflow --> now: 3, encoder_lastCount = 65532
		delta = (ENCODER_PPR-encoder_lastCount) + now+1;
	}

	else{
		delta = (int16_t)(now - encoder_lastCount);
	}

	encoder_lastCount = now;

	encoder_count = (encoder_count + delta) % ENCODER_PPR;
	encoder_shaft_pos= encoder_count;
}

void forcedInitialization(){
	set_duty_cycle(pwm_duty);

    motor_set_step(0);
    motor_commutation_step(0);
	HAL_Delay(10);

	motor_commutation_step(1);
	HAL_Delay(10);

	motor_commutation_step(1);
	HAL_Delay(10);

	motor_commutation_step(1);
	HAL_Delay(10);

	motor_commutation_step(1);
	HAL_Delay(10);

	motor_commutation_step(1);
	HAL_Delay(10);

	motor_commutation_step(1);
	HAL_Delay(10);

	encoder_reset();
}

void init_app1(){
	static uint32_t  rpm_shaft_pos_old = 0;
	static uint8_t app_state;
	encoder_count=0;
	encoder_lastCount=0;
	encoder_shaft_pos=0;
	uint16_t rpm_error_counter = 0;

	encoder_init();

	buildCommutationTable();

	HAL_ADC_Start_IT(&hadc1);

	while(1){
		switch(app_state){
		case 0: //ilk kalkış
			reset_pid_params();
			forcedInitialization();

		    app_state = 1;

			break;

		case 1: // start commutation
			HAL_TIM_Base_Start_IT(&htim3);  // Timer'ı interrupt ile başlat

			//Clear error counters
			rpm_error_counter = 0;
			app_state = 2;
			break;

		case 2: {
		    static uint32_t last_check_time = 0;
		    static uint32_t rpm_shaft_pos_old = 0;

		    // Her 100ms'de bir kontrol et (örnek)
		    if (HAL_GetTick() - last_check_time >= 10) {
		        last_check_time = HAL_GetTick();

		        uint16_t now = encoder_get_count();

		        if (now == rpm_shaft_pos_old) {
		            rpm_error_counter++;
		        } else {
		            rpm_shaft_pos_old = now;
		            rpm_error_counter = 0;  // hareket varsa sayaç sıfırla
		        }

		        if (rpm_error_counter > 10) { // 10 x 100ms = 1 saniye boyunca hareketsizlik
		            HAL_TIM_Base_Stop_IT(&htim3);  // Timer durdur
		            app_state = 0;                 // Yeniden başlat
		        }
		    }

		    break;
		}
		}

	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static uint32_t  rpm_shaft_pos_old = 0;
  static uint32_t  counter_10us_rpm_counter = 1, counter_10us_loop_counter= 1;
  static float speed_pid_loop_output;


  if (htim->Instance == TIM3)
  {
    // 10 µs'de çalışan kod

	counter_10us_rpm_counter++;
	counter_10us_loop_counter++;

	if(counter_10us_loop_counter % 10 == 0){ // 100us
		getEncoderCount();

		//check commutation position for trapezoid commutation if done by encoder
		if(encoder_commutation_pos != encoder_commutation_table[encoder_shaft_pos])
		{
			encoder_commutation_pos = encoder_commutation_table[encoder_shaft_pos];
			motor_set_step(encoder_commutation_pos);
			motor_commutation_step(0);
			set_duty_cycle(pwm_duty);
		}
	}

	if(counter_10us_rpm_counter % 100 == 0) // 1msec
	{
		counter_10us_rpm_counter = 1;

		uint16_t now = encoder_get_count();

		if(now > rpm_shaft_pos_old){
			rpm_value = (now - rpm_shaft_pos_old)*25;
			rpm_shaft_pos_old = now;
		}

		else{
			rpm_shaft_pos_old = now;
		}

		speed_pid_loop_output += PID_Loop();


	    if (pwm_duty > PID_MAX_PWM) pwm_duty = PID_MAX_PWM;
	    if (pwm_duty < PID_MIN_PWM) pwm_duty = PID_MIN_PWM;

	}
  }



}

