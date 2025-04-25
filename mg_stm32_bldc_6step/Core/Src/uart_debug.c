/*
 * uart_debug.c
 *
 *  Created on: Apr 20, 2025
 *      Author: zafer
 */

#include "main.h"
#include "uart_debug.h"

uint8_t rxData_UART;
extern UART_HandleTypeDef hlpuart1;
extern uint8_t motor_commutation_step;
extern uint8_t hall_state;
extern uint16_t pwm_duty;

char uart_buffer[64];
uint8_t uart_tx_ready = 1;

void init_uart_debug(){
	HAL_UART_Receive_IT(&hlpuart1, &rxData_UART, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == LPUART1)
	{
		HAL_UART_Receive_IT(&hlpuart1, &rxData_UART, 1);
	}
}

void send_UART_CSV_Data(uint32_t* buf, uint16_t buf_len){
	if (uart_tx_ready)
	{
		int len = snprintf(uart_buffer, sizeof(uart_buffer), "%lu,%lu,%lu,%lu\r\n",
				buf[0], buf[1], buf[2], buf[3]);

		if (len > 0)
		{
			uart_tx_ready = 0;
			HAL_UART_Transmit_IT(&hlpuart1, (uint8_t*)uart_buffer, len);
		}
	}

}



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == LPUART1)
	{
		uart_tx_ready = 1;
	}
}
