/*
 * uart_debug.h
 *
 *  Created on: Apr 24, 2025
 *      Author: elktest.pc
 */

#ifndef INC_UART_DEBUG_H_
#define INC_UART_DEBUG_H_

void init_uart_debug();
void send_UART_CSV_Data(uint32_t* buf, uint16_t buf_len);


#endif /* INC_UART_DEBUG_H_ */
