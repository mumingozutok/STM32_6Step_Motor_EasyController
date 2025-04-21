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
extern ADC_HandleTypeDef hadc2;
uint32_t adc_value_u = 0, adc_value_v = 0, adc_value_w = 0;
uint32_t adc_value_cu = 0, adc_value_cv = 0, adc_value_cw = 0;
static uint8_t bemf_sampling_channel = 0;
static uint8_t current_sampling_channel = 0;

extern uint8_t motor_commutation_step;
extern uint8_t motor_running;


#define CH_U 1
#define CH_V 2
#define CH_W 3

static float calculate_line_current(uint16_t adc_val)
{
    float voltage = ((float)adc_val / 4095.0f) * 3.3f;
    float current = (voltage - 1.65f) / 0.126f;
    return current;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	uint32_t debug_buf[6];
	static uint8_t debug_cntr = 0;
	float line_current = 0.0;

	if (hadc->Instance == ADC1)
	{
		switch(bemf_sampling_channel){
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

		if(motor_running){
			switch(motor_commutation_step){
			case 1:
				//v low
				if(adc_value_w > 1700) {
					motor_step();
				}
				break;
			case 2:
				// w low
				if(adc_value_v < 1300) {
					motor_step();
				}
				break;
			case 3:
				// w low
				if(adc_value_u > 1700) {
					motor_step();
				}
				break;
			case 4:
				//u low
				if(adc_value_w < 1300) {
					motor_step();
				}
				break;
			case 5:
				//u low
				if(adc_value_v > 1700) {
					motor_step();
				}
				break;
			case 6:
				//v low
				if(adc_value_u < 1300) {
					motor_step();
				}
				break;

			}
		}


	}
	//

	else if (hadc->Instance == ADC2)
	{
		switch(current_sampling_channel){
		case CH_U:
			adc_value_cu = HAL_ADC_GetValue(hadc);  // değeri al
			line_current = calculate_line_current(adc_value_cu);
			break;
		case CH_V:
			adc_value_cv = HAL_ADC_GetValue(hadc);  // değeri al
			line_current = calculate_line_current(adc_value_cv);
			break;
		case CH_W:
			adc_value_cw = HAL_ADC_GetValue(hadc);  // değeri al
			line_current = calculate_line_current(adc_value_cw);
			break;
		}
	}

    //

	if ((hadc->Instance == ADC1) || (hadc->Instance == ADC2))
	{
		if((debug_cntr++ == 10) & (motor_running == 1)){
			debug_cntr = 0;
			debug_buf[0] = adc_value_u;
			debug_buf[1] = adc_value_v;
			debug_buf[2] = adc_value_w;
			debug_buf[3] = (uint32_t)((float)line_current*1000.0f);


			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
			send_UART_CSV_Data(debug_buf,4);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
		}
	}
}

void conv_adc_bemf(){

	uint32_t channel;

	switch(bemf_sampling_channel){
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

void conv_adc_current(){

	uint32_t channel;

	switch(current_sampling_channel){
	case CH_U:
		channel = 1;//ADC_CHANNEL_1;
		break;
	case CH_V:
		channel = 7;//ADC_CHANNEL_7;
		break;
	case CH_W:
		channel = 6;//ADC_CHANNEL_6;
		break;
	}

	ADC2->SQR1 = (channel << 6);

	HAL_ADC_Start_IT(&hadc2);
}


void start_ADC_Sensorless(uint8_t commutation_state){
	switch (commutation_state){
	case 1:
		bemf_sampling_channel = CH_V;
		conv_adc_bemf();

		current_sampling_channel = CH_U;
		conv_adc_current();
		break;
	case 2:
		bemf_sampling_channel = CH_U;
		conv_adc_bemf();

		current_sampling_channel = CH_V;
		conv_adc_current();
		break;
	case 3:
		bemf_sampling_channel = CH_W;
		conv_adc_bemf();

		current_sampling_channel = CH_V;
		conv_adc_current();
		break;
	case 4:
		bemf_sampling_channel = CH_V;
		conv_adc_bemf();

		current_sampling_channel = CH_W;
		conv_adc_current();
		break;
	case 5:
		bemf_sampling_channel = CH_U;
		conv_adc_bemf();

		current_sampling_channel = CH_W;
		conv_adc_current();
		break;
	case 6:
		bemf_sampling_channel = CH_W;
		conv_adc_bemf();

		current_sampling_channel = CH_U;
		conv_adc_current();
		break;
	}
}

