/*
 * app_sensorless.c
 *
 *  Created on: Apr 20, 2025
 *      Author: zafer
 */

#include "main.h"
#include "app_sensorless.h"
#include "uart_debug.h"

#define ADC_BUFFER_SIZE 3*1
uint16_t adcBuffer[ADC_BUFFER_SIZE];

extern ADC_HandleTypeDef hadc1;
uint32_t adc_value_u = 0, adc_value_v = 0, adc_value_w = 0;
static uint8_t current_sampling_channel = 0;

extern uint8_t motor_commutation_step;
extern uint8_t motor_running;


#define CH_U 1
#define CH_V 2
#define CH_W 3


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	uint32_t debug_buf[3];
	static uint8_t debug_cntr = 0;

	if (hadc->Instance == ADC1)
	{
		switch(current_sampling_channel){
		case CH_U:
			adc_value_u = HAL_ADC_GetValue(hadc);  // değeri al
			break;
		case CH_V:
			adc_value_v = HAL_ADC_GetValue(hadc);  // değeri al
			break;
		case CH_W:
			adc_value_w = HAL_ADC_GetValue(hadc);  // değeri al
			break;
		}

		if((debug_cntr++ == 10) & (motor_running == 1)){
			debug_cntr = 0;
			debug_buf[0] = adc_value_u;
			debug_buf[1] = adc_value_v;
			debug_buf[2] = adc_value_w;

			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
			send_UART_CSV_Data(debug_buf,3);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
		}

		if(motor_running){
			switch(motor_commutation_step){
			case 1:
				if(adc_value_w > 1700) {
					motor_step();
				}
				break;
			case 2:
				if(adc_value_v < 1300) {
					motor_step();
				}
				break;
			case 3:
				if(adc_value_u > 1700) {
					motor_step();
				}
				break;
			case 4:
				if(adc_value_w < 1300) {
					motor_step();
				}
				break;
			case 5:
				if(adc_value_v > 1700) {
					motor_step();
				}
				break;
			case 6:
				if(adc_value_u < 1300) {
					motor_step();
				}
				break;

			}
		}


	}
}

void conv_adc(){

	uint32_t channel;

	switch(current_sampling_channel){
	case CH_U:
		channel = 5;//ADC_CHANNEL_5;
		break;
	case CH_V:
		channel = 14;//ADC_CHANNEL_14;
		break;
	case CH_W:
		channel = 11;//ADC_CHANNEL_11;
		break;
	}

	ADC1->SQR1 = (channel << 6);

	HAL_ADC_Start_IT(&hadc1);
}


void start_ADC_Sensorless(uint8_t commutation_state){
	switch (commutation_state){
	case 1:
		current_sampling_channel = CH_V;
		conv_adc();
		break;
	case 2:
		current_sampling_channel = CH_U;
		conv_adc();
		break;
	case 3:
		current_sampling_channel = CH_W;
		conv_adc();
		break;
	case 4:
		current_sampling_channel = CH_V;
		conv_adc();
		break;
	case 5:
		current_sampling_channel = CH_U;
		conv_adc();
		break;
	case 6:
		current_sampling_channel = CH_W;
		conv_adc();
		break;
	}
}

