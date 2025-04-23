/*
 * app_sensorless.c
 *
 *  Created on: Apr 20, 2025
 *      Author: zafer
 */

#include "main.h"
#include "app_sensorless.h"
#include "uart_debug.h"

#define ADC_BUFFER_SIZE 100
uint16_t currentAdcBuffer1[ADC_BUFFER_SIZE];
uint16_t currentAdcBuffer2[ADC_BUFFER_SIZE];
uint16_t* sample_current_buffer;
uint16_t* process_current_buffer;
uint32_t rpm_counter_x10 = 0;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

uint32_t adc_value_u = 0, adc_value_v = 0, adc_value_w = 0;
static uint8_t bemf_sampling_channel = 0;

extern uint8_t motor_commutation_step;
extern uint8_t motor_running;

float line_current;
uint8_t f_current_dma_finish = 0;


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
	uint32_t max_comm_adc_value = 2200; //1700
	uint32_t min_comm_adc_value = 1300;

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
				if(adc_value_w > max_comm_adc_value) {
					motor_step();
					rpm_counter_x10++;
				}
				break;
			case 2:
				// w low
				if(adc_value_v < min_comm_adc_value) {
					motor_step();
					rpm_counter_x10++;
				}
				break;
			case 3:
				// w low
				if(adc_value_u > max_comm_adc_value) {
					motor_step();
					rpm_counter_x10++;
				}
				break;
			case 4:
				//u low
				if(adc_value_w < min_comm_adc_value) {
					motor_step();
					rpm_counter_x10++;
				}
				break;
			case 5:
				//u low
				if(adc_value_v > max_comm_adc_value) {
					motor_step();
					rpm_counter_x10++;
				}
				break;
			case 6:
				//v low
				if(adc_value_u < min_comm_adc_value) {
					motor_step();
					rpm_counter_x10++;
				}
				break;

			}
		}

		if((debug_cntr++ == 10) & (motor_running == 1)){
			debug_cntr = 0;
			debug_buf[0] = adc_value_u;
			debug_buf[1] = adc_value_v;
			debug_buf[2] = adc_value_w;
			debug_buf[3] = 0;


			//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
			//send_UART_CSV_Data(debug_buf,4);
			//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
		}

	}

	else if (hadc->Instance == ADC2){
		f_current_dma_finish = 1;
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


void start_ADC_Sensorless(uint8_t commutation_state){
	switch (commutation_state){
	case 1:
		bemf_sampling_channel = CH_V;
		conv_adc_bemf();
		break;
	case 2:
		bemf_sampling_channel = CH_U;
		conv_adc_bemf();
		break;
	case 3:
		bemf_sampling_channel = CH_W;
		conv_adc_bemf();
		break;
	case 4:
		bemf_sampling_channel = CH_V;
		conv_adc_bemf();
		break;
	case 5:
		bemf_sampling_channel = CH_U;
		conv_adc_bemf();
		break;
	case 6:
		bemf_sampling_channel = CH_W;
		conv_adc_bemf();
		break;
	}
}

void init_app_sensorless(){
	process_current_buffer = currentAdcBuffer2;
	sample_current_buffer = currentAdcBuffer1;

	HAL_ADC_Start_DMA(&hadc2, (uint32_t*)currentAdcBuffer1, ADC_BUFFER_SIZE);
}

void app_sensorless_loop()
{
	while(1){
		if(f_current_dma_finish == 1){
			f_current_dma_finish = 0;

			__NOP();


			//double buffer
			/*uint16_t* temp = sample_current_buffer;
			sample_current_buffer = process_current_buffer;
			process_current_buffer = temp;*/

			uint32_t sum = 0, mean;

			for(uint8_t i = 0;i<ADC_BUFFER_SIZE;i++){
				sum += currentAdcBuffer1[i];
			}

			mean = sum / ADC_BUFFER_SIZE;
			line_current = calculate_line_current(mean);

		}
	}
}
