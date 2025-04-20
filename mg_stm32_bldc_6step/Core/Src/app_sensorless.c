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



#define CH_U 1
#define CH_V 2
#define CH_W 3


/*
#define BUFFER_SIZE 256


uint16_t windowValue[BUFFER_SIZE];
int adc_value = 0;

void next_commutation(uint8_t state)
{
#ifdef SENSORLESS
	motor_commutation(state);
#endif

}

uint8_t is_inWindow(uint16_t value, uint16_t GPIO_Pin, uint8_t slope)
{
	uint8_t retVal = 0;
	uint16_t lowerLimit = 70;
	uint16_t higherLimit = 170;
	if(slope == FALLING)
	{
		lowerLimit = 70;
		higherLimit = 170;
	}
	else if (slope == RISING)
	{
		lowerLimit = 430;
		higherLimit = 470;
	}
	if((value > lowerLimit) & (value < higherLimit))
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_Pin, GPIO_PIN_SET);
		retVal=1;
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_Pin, GPIO_PIN_RESET);
		retVal=0;
	}
	return retVal;
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	uint16_t value;
	uint8_t state;
	value = adcBuffer[2]; // Green
	if(bufferCounter >= 256) bufferCounter = 0;
	windowValue[bufferCounter++] =  value;
#ifdef HALLSENSOR
	state = hall_state;
#else
	state =  com_State;
#endif
	switch(state){
	case 1:
		//v boşta
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
		value = adcBuffer[2]; // Green
		if(is_inWindow(value, GPIO_PIN_4, FALLING)==1)
	    {
			next_commutation(2);
		}

		break;
	case 2:
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
		//u boşta
		value = adcBuffer[0]; // Yellow
		if(is_inWindow(value, GPIO_PIN_4, RISING) == 1)
		{
			next_commutation(3);
		}
		break;
	case 3:
		//w boşta
		value = adcBuffer[1]; // Blue
		if(is_inWindow(value, GPIO_PIN_4, FALLING) == 1)
		{
			next_commutation(4);
		}
		break;
	case 4:
		//v boşta
		value = adcBuffer[2];
		if(is_inWindow(value, GPIO_PIN_4, RISING) == 1)
		{
			next_commutation(5);
		}
		break;
	case 5:
		//u boşta
		value = adcBuffer[0];
		if(is_inWindow(value, GPIO_PIN_4, FALLING) == 1)
		{
			next_commutation(6);
		}
		break;
	case 6:
		//w boşta
		value = adcBuffer[1];
		if(is_inWindow(value, GPIO_PIN_4, RISING) == 1)
		{
			next_commutation(0);
		}
		break;
	}
	HAL_ADC_Stop_DMA(&hadc1);

}
 */


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

		if(debug_cntr++ == 10){
			debug_cntr = 0;
			debug_buf[0] = adc_value_u;
			debug_buf[1] = adc_value_v;
			debug_buf[2] = adc_value_w;

			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
			send_UART_CSV_Data(debug_buf,3);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
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

	/*ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = channel;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;

	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}*/

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

