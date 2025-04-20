/*
 * app.h
 *
 *  Created on: Apr 7, 2025
 *      Author: mumin
 */

#ifndef INC_APP_H_
#define INC_APP_H_

uint8_t is_inWindow(uint16_t value, uint16_t GPIO_Pin, uint8_t slope);
void init_app();
void HAL_TIM_PeriodElapsedCallback_App(TIM_HandleTypeDef *htim);

#endif /* INC_APP_H_ */
